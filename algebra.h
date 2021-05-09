/* Written by Todd Doucet. */
#pragma once

#include <iostream>
#include <tuple>

template<class T>
struct algebra : public T
{
    using T::T;

    template<typename Lambda>
    static void memberwise(Lambda lambda)
    {
        std::apply(lambda, T::members);
    }

    void clear()
    {
        memberwise
        (
            [&](auto ...args) { ( (this->*args).clear(), ... ); }
        );
    }

    algebra<T>& operator+=(T const& rhs)
    {
        memberwise
        (
            [&](auto ...args) { ( (this->*args += rhs.*args), ... ); }
        );
        return *this;
    }

    algebra<T>& operator*=(float scale)
    {
        memberwise
        (
            [&](auto ...args) { ( (this->*args *= scale), ... ); }
        );
        return *this;
    }

    algebra<T> operator*(float scale)
    {
        return algebra<T>(*this) *= scale;
    }
};

template<class T>
algebra<T> operator+(algebra<T> const& lhs,
                     algebra<T> const& rhs)
{
    return algebra<T>{lhs} += rhs;
}

template<class T>
std::ostream& operator<<(std::ostream& os, algebra<T> const& val)
{
    algebra<T>::memberwise
    (
        [&os, &val](auto ...args) { ( (os << val.*args << "\n"), ... ); }
    );
    return os;
}

template<class T>
std::istream& operator>>(std::istream& s, algebra<T> & val)
{
    algebra<T>::memberwise
    (
        [&s, &val](auto ...args) { ( (s >> val.*args), ... ); }
    );
    return s;
}

