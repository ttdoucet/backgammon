/* Written by Todd Doucet.  See file copyright.txt.
 */
#pragma once

#include "mathfuncs.h"
#include "matrix.h"
#include "random.h"
#include "netop.h"

template<int Features, int Hidden>
struct TwoLayerParameters
{
    matrix<Hidden, Features> M;
    rowvec<Hidden> V;

    void clear()
    {
        M.clear();
        V.clear();
    }

    TwoLayerParameters& operator+=(const TwoLayerParameters &rhs)
    {
        M += rhs.M;
        V += rhs.V;
        return *this;
    }

    TwoLayerParameters& operator*=(float scale)
    {
        M *= scale;
        V *= scale;
        return *this;
    }

    TwoLayerParameters operator*(float scale) const
    {
        TwoLayerParameters r(*this);
        return r *= scale;
    }
};

template<int F, int H>
TwoLayerParameters<F,H> operator+(const TwoLayerParameters<F,H> &lhs,
				  const TwoLayerParameters<F,H> &rhs)
{
    TwoLayerParameters<F,H> v(lhs);
    return v += rhs;
}

template<int Features, int Hidden>
class SigmoidNet
{
protected:

    Linear<Features, Hidden>      Op_1{act.input, act.hidden, params.M, grad.M};
    Termwise<logistic, Hidden>    Op_2{act.hidden, act.hidden};
    Linear<Hidden, 1>             Op_3{act.hidden, act.pre_out, params.V, grad.V};
    Termwise<bipolar_sigmoid, 1>  Op_4{act.pre_out, act.pre_out};
    Termwise<affine<3,0>, 1>      Op_5{act.pre_out, act.out};

    float feedForward()
    {
        Op_1.fwd();
        Op_2.fwd();
        Op_3.fwd();
        Op_4.fwd();
        Op_5.fwd();
        return act.out;
    }

public:
    using Parameters = TwoLayerParameters<Features, Hidden>; 
    Parameters params, grad;

    struct Activations
    {
        using InputVector = vec<Features>;
        using HiddenVector = vec<Hidden>;
        using OutputVector = vec<1>;

        InputVector input;
        HiddenVector hidden;
        OutputVector pre_out;
        OutputVector out;
    } act;

    using InputVector = typename Activations::InputVector;

    InputVector& input()
    {
        return act.input;
    }

    /* Computes dy/dw, where y is the scalar output of the net
     * from the last forward calculation, and w is any learnable
     * parameter.
     */
    void gradient(Parameters& g)
    {
        grad.clear();

        matrix<1,1> one = { 1 };
        auto r5 = Op_5.bwd(one);
        auto r4 = Op_4.bwd(r5);
        auto r3 = Op_3.bwd(r4);
        auto r2 = Op_2.bwd(r3);
        Op_1.bwd_param(r2);
        g = grad;
    }

    void update_model(const Parameters& adj)
    {
        params += adj;
    }

    SigmoidNet()
    {
        RNG_normal rand1(0, 1.0 / Features);
        for (auto& m : params.M)
            m = rand1.random();

        RNG_normal rand2(0, 1.0 / Hidden);
        for (auto& v : params.V)
            v = rand2.random();
    }
};
