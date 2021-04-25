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

template<class Activ, int len>
class Termwise
{
    using vec_t = vec<len>;

public:
    const vec_t& src;
    vec_t& dst;

    void fwd()
    {
        for (int i = 0; i < len; i++)
            dst(i) = Activ::fwd( src(i) );
    }

    vec_t bwd(const vec_t& upstream_d)
    {
        vec_t r;

        for (int i = 0; i < len; i++)
            r(i) = upstream_d(i) * Activ::bwd( dst(i) );

        return r;
    }

    void bwd_param(const vec_t& upstream_d) {  /* no parameters */  }

    Termwise(const vec_t& src, vec_t& dest) : src{src}, dst{dest} { }
};

template<int xdim, int ydim>
class Linear
{
    using src_t = vec<xdim>;
    using dst_t = vec<ydim>;

    matrix<ydim, xdim>& M;
    matrix<ydim, xdim>& grad_M;
    const src_t& x;
    dst_t& y;

public:
    Linear(const src_t& x,
           dst_t& y,
           matrix<ydim,xdim>& M,
           matrix<ydim,xdim>& grad_M
    )
        : x{x}, y{y}, M{M}, grad_M{grad_M}
    {
        RNG_normal rand(0, 1.0 / M.Cols());
        for (auto& m : M)
            m = rand.random();
    }

    void fwd()
    {
        y = M * x;
    }

    src_t bwd(const dst_t& upstream_d)
    {
        bwd_param(upstream_d);

        src_t ret;

        for (auto r = 0; r < y.length(); r++)
            for (auto c = 0; c < ret.length(); c++)
                ret(c) += upstream_d(r) * M(r, c);

        return ret;
    }

    void bwd_param(const dst_t& upstream_d)
    {
        for (auto r = 0; r < M.Rows(); r++)
            for (auto c = 0; c < M.Cols(); c++)
                grad_M(r, c) += upstream_d(r) * x(c);
    }
};
