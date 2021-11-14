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

    matrix_t bwd(matrix_t const& upstream_d)
    {
        matrix_t ret;

        auto d = dst.cbegin();
        auto u = upstream_d.cbegin();
        auto r = ret.begin();

        while (r < ret.end())
            *r++ = *u++ * Activ::bwd(*d++);

        return ret;
    }

    void bwd_param(matrix_t const& upstream_d) {  /* no parameters */  }
};

template<class src_t, class dst_t>
class Linear
{
public:
    using param_t = matrix<dst_t::length(), src_t::length()>;

    Linear(src_t const& x,
           dst_t& y,
           param_t& M,
           param_t& grad_M
    )
        : x{x}, y{y}, M{M}, grad_M{grad_M}
    {
        RNG_normal rand(0, 1.0 / x.length());
        for (auto& m : M)
            m = rand.next();
    }

    void fwd()
    {
        y = M * x;
    }

    src_t bwd(dst_t const& upstream_d)
    {
        bwd_param(upstream_d);

        src_t ret;

        for (auto r = 0; r < y.length(); r++)
            for (auto c = 0; c < ret.length(); c++)
                ret(c) += upstream_d(r) * M(r, c);

        return ret;
    }

    void bwd_param(dst_t const& upstream_d)
    {
        for (auto r = 0; r < M.Rows(); r++)
            for (auto c = 0; c < M.Cols(); c++)
                grad_M(r, c) += upstream_d(r) * x(c);
    }

private:
    param_t &M;
    param_t &grad_M;
    src_t const& x;
    dst_t &y;
};

template<int xdim, int ydim>
class Bias
{
    using param_t = matrix<ydim, xdim>;
    using src_t = param_t;
    using dst_t = param_t;

    param_t &B;
    param_t &grad_B;

    src_t const& x;
    dst_t& y;

public:
    Bias(
        src_t const& x,
        dst_t &y,
        param_t &B,
        param_t &grad_B
    )
        : x{x},
          y{y},
          B{B},
          grad_B{grad_B}
    {
        for (auto& b : B)
            b = 0;
    }

    void fwd()
    {
        y = x + B;
    }

    src_t bwd(dst_t const& upstream_d)
    {
        bwd_param(upstream_d);
        return upstream_d;
    }

    void bwd_param(dst_t const& upstream_d)
    {
        grad_B += upstream_d;
    }
};
