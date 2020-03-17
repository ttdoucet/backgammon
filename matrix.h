#pragma once
#include <cassert>
#include <initializer_list>

/* Written by Todd Doucet.
 *
 * Intended for relatively small matrices whose sizes are known
 * at compile-time.   Intended to be fast and efficient.
 */ 
template<int R, int C=1> class matrix
{
    typedef float Array[R][C];

 public:
    int Rows() const { return R; }
    int Cols() const { return C; }

    // initialize to zeros.
    matrix<R,C>()
    {
        for (int r = 0; r < Rows(); r++)
            for (int c = 0; c < Cols(); c++)
                data[r][c] = 0;
    }

    matrix<R,C>(std::initializer_list<float> li)
    {
        assert(li.size() == R * C);
        auto it = li.begin();
        for (int r = 0; r < Rows(); r++)
            for (int c = 0; c < Cols(); c++)
                data[r][c] = *it++;
    }

    float& operator() (int r, int c=0)
    {
         return data[r][c];
    }

    float operator() (int r, int c=0) const
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
        for (int r = 0; r < Rows(); r++)
            for (int c = 0; c < Cols(); c++)
                data[r][c] *= scale;
        return *this;
    }

    matrix<R,C>& operator /=(float scale)
    {
        for (int r = 0; r < Rows(); r++)
            for (int c = 0; c < Cols(); c++)
                data[r][c] /= scale;
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
