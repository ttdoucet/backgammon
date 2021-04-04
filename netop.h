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

    void fwd()
    {
        for (int r = 0; r < x.Rows(); r++)
            y(r, 0) = x(r,0);
    }

    void bwd()
    {
        for (int r = 0; r < x.Rows(); r++)
            x(r, 0) = y(r,0);
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
