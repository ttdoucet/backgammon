/* Written by Todd Doucet.  See file copyright.txt.
 */
#pragma once

#include "mathfuncs.h"
#include "matrix.h"

class netop
{
public:
    virtual void fwd() = 0;
    virtual void bwd() = 0;
    virtual void bwd_param() = 0;
    virtual ~netop() = default;
};



template<class Activ, int xydim>
  class Termwise_t : public netop
{
public:
    vec<xydim>& x;
    vec<xydim>& y;

// can be simplified now
    void fwd()
    {
        auto src = x.begin();
        auto dst = y.begin();
        while ( src < x.end() )
            *dst++ = Activ::fwd(*src++);
    }

// can be simplified now
    void bwd()
    {
        auto src = y.begin();
        auto dst = x.begin();
        while ( src < y.end() )
            *dst++ = Activ::bwd(*src++);
    }

    void bwd_param()
    {
        // no parameters
    }

    Termwise_t(vec<xydim>& x, vec<xydim>& y)
        : x{x}, y{y}
     {
     }
};

template<class Activ, int xydim>
auto Termwise(vec<xydim>& x, vec<xydim>& y)
{
    return Termwise_t<Activ, xydim>(x, y);
}




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
        // nyi
    }

    void bwd_param()
    {
        // nyi
    }

};
