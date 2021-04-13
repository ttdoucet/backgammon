/* Written by Todd Doucet.  See file copyright.txt.
 */
#pragma once

#include "mathfuncs.h"
#include "matrix.h"

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

    vec_t bwd(const vec_t& upstream)
    {
        vec_t r;

        for (int i = 0; i < len; i++)
            r(i) = upstream(i) * Activ::bwd( dst(i) );

        return r;
    }

    void bwd_param(const vec_t& upstream) {  /* no parameters */  }

    Termwise(const vec_t& src, vec_t& dest) : src{src}, dst{dest} { }
};

template<int xdim, int ydim>
class Linear
{
    using src_t = vec<xdim>;
    using dst_t = vec<ydim>;

    matrix<ydim, xdim>& M;
    matrix<ydim, xdim>& dl_dM;
    const src_t& x;
    dst_t& y;

public:

    Linear(const src_t& x,
           dst_t& y,
           matrix<ydim,xdim>& M,
           matrix<ydim,xdim>& dl_dM
    )
        : x{x}, y{y}, M{M}, dl_dM{dl_dM}
    {
        RNG_normal rand1(0, 1.0 / M.Cols());
        for (auto& m : M)
            m = rand1.random();
    }

    void fwd()
    {
        y = M * x;
    }

    src_t bwd(const dst_t& upstream)
    {
        bwd_param(upstream);

        src_t ret;

        for (auto r = 0; r < y.length(); r++)
            for (auto c = 0; c < ret.length(); c++)
                ret(c) += upstream(r) * M(r, c);

        return ret;
    }

    void bwd_param(const dst_t& upstream)
    {
        for (auto r = 0; r < M.Rows(); r++)
            for (auto c = 0; c < M.Cols(); c++)
                dl_dM(r, c) += upstream(r) * x(c);
    }
};
