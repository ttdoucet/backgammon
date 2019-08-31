#pragma once

template<int R, int C, typename T>
class Matrix
{
    T v[R][C];

public:
    T& operator()(int r, int c)
    {
        return v[r][c];
    }

    //  Matrix(const Matrix&) = delete;

    int rows() const
    {
        return R;
    }

    int cols() const
    {
        return C;
    }

    class row_iterator
    {
        Matrix& m;
        int r;

    public:
        row_iterator(Matrix& mr, int row) : m(mr), r(row)
        {
        }

        int size() const
        {
            return m.size();
        }

        T& operator [](int i)
        {
            return m(r, i);
        }
    };


    row_iterator row(int r)
    {
        return row_iterator(*this, r);
    }



};
