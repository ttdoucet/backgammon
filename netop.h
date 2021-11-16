/* Written by Todd Doucet. */
#pragma once
#include <cmath>
#include "matrix.h"
#include "random.h"

/*
 * An activation function f computes y = f(x) with f.fwd(x), and computes
 * the local derivative at x, dy/dx, with f.bwd(y).
 */

struct logistic
{
    static inline float fwd(float x) { return  1 / (1 + exp(-x)); }
    static inline float bwd(float y) { return y * (1 - y); }
};

struct bipolar_sigmoid
{
    static inline float fwd(float x) { return 2 * logistic::fwd(x) - 1; }
    static inline float bwd(float y) { return 2 * logistic::bwd( (y+1) / 2 ); }
};

struct tanh_sigmoid
{
//  static inline float fwd(float x) { return 2 * logistic::fwd(2 * x) - 1; }
    static inline float fwd(float x) { return tanh(x); }
    static inline float bwd(float y) { return 1 - y * y; }
};

struct ReLU
{
    static inline float fwd(float x) { return std::max(0.0f, x); }
    static inline float bwd(float y) { return (y < 0) ? 0 : 1; }
};

struct leaky_ReLU
{
    static inline float fwd(float x) { return (x >= 0) ? x : (0.01 * x); }
    static inline float bwd(float y) { return (y < 0) ? 0.01 : 1; }
};

template<int scale, int offset>
struct affine
{
    static inline float fwd(float x) { return scale * x + offset; }
    static inline float bwd(float y) { return scale; }
};

/*
 * The netops are nodes in a computational graph which support
 * forward and back propagation.
 */

template<class matrix_t, class Activ>
class Termwise
{
public:
    Termwise(matrix_t const& src, matrix_t & dest)
        : src{src}, dst{dest} { }

    matrix_t const& src;
    matrix_t &dst;

    void fwd()
    {
        auto s = src.cbegin();
        auto d = dst.begin();

        while (s < src.cend())
            *d++ = Activ::fwd(*s++);
    }

    matrix_t bwd(matrix_t const& up_d)
    {
        matrix_t ret;

        auto d = dst.cbegin();
        auto u = up_d.cbegin();
        auto r = ret.begin();

        while (r < ret.end())
            *r++ = *u++ * Activ::bwd(*d++);

        return ret;
    }

    void bwd_param(matrix_t const& up_d) {  /* no parameters */  }
};

template<class src_t, class dst_t>
class Linear
{
public:
    using param_t = matrix<dst_t::length(), src_t::length()>;

    Linear(src_t const& x,
           dst_t& y,
           param_t& M,
           param_t& M_grad
    )
        : x{x}, y{y}, M{M}, M_grad{M_grad}
    {
        static_assert(src_t::Cols() == 1);
        static_assert(dst_t::Cols() == 1);

        RNG_normal rand(0, 1.0 / x.length());
        for (auto& m : M)
            m = rand.next();
    }

    void fwd()
    {
        y = M * x;
    }

    src_t bwd(dst_t const& up_d)
    {
        bwd_param(up_d);

        src_t ret;

        for (auto r = 0; r < y.length(); r++)
            for (auto c = 0; c < ret.length(); c++)
                ret(c) += up_d(r) * M(r, c);

        return ret;
    }

    void bwd_param(dst_t const& up_d)
    {
        for (auto r = 0; r < M.Rows(); r++)
            for (auto c = 0; c < M.Cols(); c++)
                M_grad(r, c) += up_d(r) * x(c);
    }

private:
    param_t &M;
    param_t &M_grad;
    src_t const& x;
    dst_t &y;
};

// in progress
template<class src_t, class dst_t>
class Linear_
{
public:
    using param_t = matrix<src_t::length(), dst_t::length()>;

    Linear_(src_t const& x,
           dst_t& y,
           param_t& M,
           param_t& M_grad
    )
        : x{x}, y{y}, M{M}, M_grad{M_grad}
    {
        static_assert(src_t::Rows() == 1);
        static_assert(dst_t::Rows() == 1);

        RNG_normal rand(0, 1.0 / x.length());
        for (auto& m : M)
            m = rand.next();
    }

    void fwd()
    {
        y = x * M;
    }

    src_t bwd(dst_t const& up_d)
    {
        assert(false); // nyi
    }

    void bwd_param(dst_t const& up_d)
    {
        assert(false); // nyi
    }

private:
    param_t &M;
    param_t &M_grad;
    src_t const& x;
    dst_t &y;
};

template<class matrix_t>
class Bias
{
    using param_t = matrix_t;

public:
    Bias(
        matrix_t const& x,
        matrix_t &y,
        param_t &B,
        param_t &B_grad
    )
        : x{x}, y{y}, B{B}, B_grad{B_grad}
    {
        for (auto& b : B)
            b = 0;
    }

    void fwd()
    {
        y = x + B;
    }

    matrix_t bwd(matrix_t const& up_d)
    {
        bwd_param(up_d);
        return up_d;
    }

    void bwd_param(matrix_t const& up_d)
    {
        B_grad += up_d;
    }

private:
    param_t& B;
    param_t& B_grad;

    matrix_t const& x;
    matrix_t& y;
};
