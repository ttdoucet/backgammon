
#pragma once
#include <cmath>
#include <cassert>
#include <initializer_list>
#include <iostream>
#include <cstdlib>

/* Written by Todd Doucet.
 *
 * Intended for relatively small matrices whose sizes are known
 * at compile-time.   Intended to be fast and efficient.
 */ 
template<int R, int C>
class matrix
{
    typedef float Array[R][C];

  public:
    constexpr int Rows() const { return R; }
    constexpr int Cols() const { return C; }

    // initialize to zeros.
    matrix<R,C>()
    {
        for (auto &p : *this)
            p = 0;
    }

    matrix<R,C>(std::initializer_list<float> li)
    {
        assert(li.size() == R * C);
        auto it = li.begin();
        for (auto &p : *this)
            p = *it++;
    }

    float& operator() (int r, int c)
    {
        return data[r][c];
    }

    float operator() (int r, int c) const
    {
        return data[r][c];
    }

    template<typename T>
    struct dependent_false { static constexpr bool value = false; };

    float operator() (int idx) const
    {
        if constexpr(C == 1)
            return (*this)(idx,0);
        else if constexpr(R == 1)
            return (*this)(0, idx);
        else
            static_assert(dependent_false<matrix<R,C>>::value,
                          "Single index requires row or column vector.");
    }

    float& operator() (int idx)
    {
        if constexpr(C == 1)
            return (*this)(idx,0);
        else if constexpr(R == 1)
            return (*this)(0, idx);
        else
            static_assert(dependent_false<matrix<R,C>>::value,
                          "Single index requires row or column vector.");
    }

    constexpr int length() const
    {
        if constexpr(C == 1 || R == 1)
            return std::max(R, C);
        else
            static_assert(dependent_false<matrix<R,C>>::value,
                          "length() requires row or column vector.");

    }

    matrix<R,C>& operator+=(const matrix<R,C> &rhs)
    {
        for (int r = 0; r < Rows(); r++)
            for (int c = 0; c < Cols(); c++)
                data[r][c] += rhs(r,c);
        return *this;
    }

    matrix<R,C>& operator-=(const matrix<R,C> &rhs)
    {
        for (int r = 0; r < Rows(); r++)
            for (int c = 0; c < Cols(); c++)
                data[r][c] -= rhs(r,c);
        return *this;
    }

    matrix<R,C>& operator*=(float scale)
    {
        for (auto &p : *this)
            p *= scale;
        return *this;
    }

    matrix<R,C>& operator/=(float scale)
    {
        for (auto &p : *this)
            p /= scale;
        return *this;
    }

    matrix<R,C> operator*(float scale) const
    {
        matrix<R,C> v(*this);
        return (v *= scale);
    }

    matrix<R,C> operator/(float scale) const
    {
        matrix<R,C> v(*this);
        return (v /= scale);
    }

    // unary minus
    matrix<R,C> operator-() const
    {
        return -1 * (*this);
    }

    // unary plus
    matrix<R,C> operator+() const
    {
        return (*this);
    }

    bool operator==(const matrix<R,C> &rhs) const
    {
        for (int r = 0; r < Rows(); r++)
            for (int c = 0; c < Cols(); c++)
                if (data[r][c] != rhs(r,c))
                    return false;
        return true;
    } 

    bool operator!=(const matrix<R,C> &rhs) const
    {
        return !(*this == rhs);
    } 

    void clear()
    {
        for (auto &p : *this)
            p = 0;
    }

    bool isfinite() const
    {
        for (int r = 0; r < Rows(); r++)
            for (int c = 0; c < Cols(); c++)
                if (std::isfinite((*this)(r,c)) == false)
                    return false;
        return true;
    }

    double magnitude()
    {
        double sum = 0;
        for (auto p : *this)
                sum += p * p;
        return sqrt(sum);
    }

    float *Data() { return &data[0][0]; }

    const float *RowData(int r) const
    {
        return &data[r][0];
    }

    const float *ColData() const
    {
        static_assert(C == 1);
        return &data[0][0];
    }

    float *begin() { return Data(); }
    float *end()   { return &data[R-1][C-1] + 1; }

    explicit operator float() const;

  private:
    alignas(16) Array data;
};

template<int n>
using vec = matrix<n,1>;

template<int n>
using rowvec = matrix<1,n>;

/* Conversion to float only for 1-by-1 matrix.
 */
template<> inline matrix<1, 1>::operator float() const
{
    return (*this)(0, 0);
}

/* Multiplication of two matrices.
 */

#if 0

template<int S1, int S2, int S3> inline
matrix<S1,S3> operator *(const matrix<S1, S2> &lhs, const matrix<S2, S3> &rhs)
{
    // Ensure that a specialization, below, is used instead.
    // static_assert(rhs.Cols() != 1);

    matrix<S1,S3> result;
    for (int r = 0; r < result.Rows(); r++)
    {
        for (int c = 0; c < result.Cols(); c++)
        {
            float res = 0;
            for (int k = 0; k < lhs.Cols(); k++)
                res += ( lhs(r, k) * rhs(k, c) );
            result(r, c) = res;
        }
    }
    return result;
}

#endif

#include <numeric>

#if 1

template<typename Tp, typename R>
R dot_product(Tp a, Tp b, int n, R init)
{
    R r = 0;
    R sums[n], *s = sums;;

    for (int i = 0; i < n; i++)
        s[i] = a[i] * b[i];

    s = sums;
    for (int i = 0; i < n; i++)
        r += sums[i];

    return r;
}

template<int S1, int S2> inline
vec<S1> operator *(const matrix<S1, S2> &lhs, const vec<S2> &rhs)
{
    const float *rp = rhs.ColData();
    const float *lp = lhs.RowData(0);

    const auto n = lhs.Cols();

    vec<S1> result;
    for (int r = 0; r < result.Rows(); r++, lp+=n)
        result(r) = dot_product(lp, rp, n, 0.0f);
    return result;
}

#endif


#if 0

template<int S1, int S2> inline
vec<S1> operator *(const matrix<S1, S2> &lhs, const vec<S2> &rhs)
{
    const float *rp = rhs.ColData();
    const float *lp = lhs.RowData(0);

    const auto n = lhs.Cols();

    vec<S1> result;
    for (int r = 0; r < result.Rows(); r++, lp+=n)
        result(r) = std::inner_product(lp, lp + n, rp, 0.0f);
    return result;
}

#endif

#if 0

template<int S1, int S2> inline
vec<S1> operator *(const matrix<S1, S2> &lhs, const vec<S2> &rhs)
{
    vec<S1> result;
    float *dst = result.Data();
    float const *rpstart = rhs.ColData();
    float const *lp = lhs.RowData(0);

    for(int r = 0; r < S1 ; r++ ){
        auto rp = rpstart;

        for (int c = 0; c < S2; c++)
            *dst += (*lp++ * *rp++);
        dst++;
    }
    return result;
}

#endif

/* Multiplication of a matrix by a scalar.
 */
template<int R, int C> inline
matrix<R,C> operator*(float scale, const matrix<R,C> rhs)
{
    matrix<R,C> v(rhs);
    return v *= scale;
}

/* Addition of two matrices.
 */
template<int R, int C> inline
matrix<R,C> operator+(const matrix<R,C> &lhs, const matrix<R,C> &rhs)
{
    matrix<R,C> v(lhs);
    return v += rhs;
}

/* Subtraction of matrices.
 */
template<int R, int C> inline
matrix<R,C> operator-(const matrix<R,C> &lhs, const matrix<R,C> &rhs)
{
    matrix<R,C> v(lhs);
    return v -= rhs;
}

template<int R, int C>
std::ostream& operator<<(std::ostream& s, const matrix<R,C>& m)
{
    using namespace std;
    ios_base::fmtflags f(s.flags());

    s << dec << "{ " << m.Rows() << " " << m.Cols() << "\n" << hexfloat;
    for (int r = 0; r < m.Rows(); r++)
    {
        s << " ";
        for (int c = 0; c < m.Cols(); c++)
            s << " " << m(r, c);
        s << "\n";
    }
    s << "}\n";

    s.flags(f);
    return s;
}

template<int R, int C>
std::istream& operator>>(std::istream& s, matrix<R,C>& m)
{
    using namespace std;
    ios_base::fmtflags f(s.flags());

        int rows, cols;
        string curly, fstr;

        s >> dec >> curly >> rows >> cols >> hexfloat;

        if (!s || curly != "{"s )
            throw runtime_error("matrix: could not determine geometry");

        if ( rows != m.Rows() || cols != m.Cols() )
            throw runtime_error("matrix: geometry mismatch");

        for (int r = 0; r < rows; r++)
            for (int c = 0; c < cols; c++)
            {
                s >> fstr;
                if (!s)
                    throw runtime_error("matrix: could not read float");
                char *end;
                float f = strtof(fstr.c_str(), &end);
                if ( (end - fstr.c_str()) != fstr.length())
                    throw runtime_error("matrix: float conversion error");
                m(r, c) = f;

            }

        s >> curly;
        if (!s || curly != "}"s)
            throw runtime_error("matrix: expected }");

    s.flags(f);
    return s;
}
