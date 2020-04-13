#pragma once
#include <cassert>
#include <initializer_list>

/* Written by Todd Doucet.
 *
 * Intended for relatively small matrices whose sizes are known
 * at compile-time.   Intended to be fast and efficient.
 */ 
template<int R, int C=1>
class matrix
{
    typedef float Array[R][C];

public:
    float *begin() { return Data(); }
    float *end()   { return &data[R-1][C-1] + 1; }

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

    matrix<R,C>& operator +=(const matrix<R,C> &rhs)
    {
        for (int r = 0; r < Rows(); r++)
            for (int c = 0; c < Cols(); c++)
                data[r][c] += rhs(r,c);
        return *this;
    }

    matrix<R,C>& operator -=(const matrix<R,C> &rhs)
    {
        for (int r = 0; r < Rows(); r++)
            for (int c = 0; c < Cols(); c++)
                data[r][c] -= rhs(r,c);
        return *this;
    }

    matrix<R,C>& operator *=(float scale)
    {
        for (auto &p : *this)
            p *= scale;
        return *this;
    }

    matrix<R,C>& operator /=(float scale)
    {
        for (auto &p : *this)
            p /= scale;
        return *this;
    }

    matrix<R,C> operator *(float scale) const
    {
        matrix<R,C> v(*this);
        return (v *= scale);
    }

    matrix<R,C> operator /(float scale) const
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

    /*
     * These construct and return an appropriate matrix,
     * copying the data as necessary.  But in practice
     * the copy is often optimized away.
     */

    matrix<1,C> RowVector(int r) const
    {
        matrix<1,C> dest;
        for (int c = 0; c < Cols(); c++)
            dest(0, c) = data[r][c];
        return dest;
    }

    matrix<R,1> ColumnVector(int c) const
    {
        matrix<R,1> dest;
        for (int r = 0; r < Rows(); r++)
            dest(r, 0) = data[r][c];
        return dest;
    }

    matrix<C,R> Transpose() const
    {
        matrix<C,R> dest;
        for (int r = 0; r < Rows(); r++)
            for (int c = 0; c < Cols(); c++)
                dest(c,r) = data[r][c];
        return dest;
    }

    void clear()
    {
        for (auto &p : *this)
            p = 0;
    }

    double magnitude()
    {
        double sum = 0;
        for (auto p : *this)
                sum += p * p;
        return sqrt(sum);
    }

    float *Data() { return &data[0][0]; }
    operator float() const;

 private:
    alignas(16) Array data;
};

// Conversion to float only for 1-by-1 matrix.
template<>  inline matrix<1, 1>::operator float() const
{
  return data[0][0];
}

/* Multiplication of two matrices.
 */
template<int S1, int S2, int S3>
   matrix<S1,S3> operator *(const matrix<S1, S2> &lhs, const matrix<S2, S3> &rhs)
{
    matrix<S1,S3> result;
    for (int r = 0; r < result.Rows(); r++)
    {
        for (int c = 0; c < result.Cols(); c++)
        {
            result(r, c) = 0;
            for (int k = 0; k < lhs.Cols(); k++)
                result(r, c) += ( lhs(r, k) * rhs(k, c) );
        }
    }
    return result;
}

/* Multiplication of a matrix by a scalar.
 */
template<int R, int C> matrix<R,C> operator *(float scale, const matrix<R,C> rhs)
{
    matrix<R,C> v(rhs);
    return v *= scale;
}

/* Addition of two matrices.
 */
template<int R, int C>  matrix<R,C> operator+(const matrix<R,C> &lhs, const matrix<R,C> &rhs)
{
    matrix<R,C> v(lhs);
    return v += rhs;
}

/* Subtraction of matrices.
 */
template<int R, int C>  matrix<R,C> operator-(const matrix<R,C> &lhs, const matrix<R,C> &rhs)
{
    matrix<R,C> v(lhs);
    return v -= rhs;
}

#include <iostream>

template<int R, int C>
std::ostream& operator<<(std::ostream& s, const matrix<R,C>& m)
{
    for (int r = 0; r < m.Rows(); r++)
    {
        for (int c = 0; c < m.Cols(); c++)
            s << m(r, c) << " ";
        s << "\n";
    }
    return s;
}
