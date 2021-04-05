/* Written by Todd Doucet.  See file copyright.txt.
 */
#pragma once

#include "mathfuncs.h"
#include "matrix.h"

#include <cassert>

class netop
{
public:
    virtual void fwd() = 0;
    virtual void bwd() = 0;
    virtual void bwd_param() = 0;
    virtual ~netop() = default;
};


template<class Activ, int len>
  class Termwise : public netop
{
public:
    vec<len>& v;

    void fwd()
    {
        for (int r = 0; r < v.Rows(); r++)
            v(r, 0) = Activ::fwd(v(r, 0));
    }

    void bwd()
    {
        for (int r = 0; r < v.Rows(); r++)
            v(r, 0) = Activ::bwd(v(r, 0));
    }

    void bwd_param()
    {
        // no parameters
    }

    Termwise(vec<len>& v)
        : v{v}
     {
     }
};

template<int xdim, int ydim>
  class Linear : public netop
{
public:
    matrix<ydim, xdim>& M;
    matrix<ydim, xdim>& dl_dM;
    vec<xdim>& x;
    vec<ydim>& y;

    Linear(vec<xdim>& x,
           vec<ydim>& y,
           matrix<ydim,xdim>& M,
           matrix<ydim,xdim>& dl_dM
    )
        : x{x}, y{y}, M{M}, dl_dM{dl_dM}
    {
    }

    void fwd()
    {
        y = M * x;
    }

    void bwd()
    {
        bwd_param();
        assert(false);   // nyi

    }

    void bwd_param()
    {
        assert(false); // nyi
    }

};
