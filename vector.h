#pragma once

template<int SIZE, typename T>
class Vector
{
    T v[SIZE];

public:

    Vector()
    {
    }

    Vector(const Vector&) = delete;

public:
    T& operator [](int i)
    {
        return v[i];
    }

    int size() const
    {
        return SIZE;
    }

    using iterator=T*;

    T* begin()
    {
        return v;
    }

    T* end()
    {
        return v + size();
    }
};
