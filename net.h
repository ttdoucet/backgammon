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
    using W1 = matrix<Hidden, Features>;
    using W2 = rowvec<Hidden>;

    W1 M;
    W2 V;

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

template<int Features,
         int Hidden,
         class Activ1,
         class Activ2,
         class Activ3 = affine<1, 0>
         >
class FcTwoLayerNet
{
protected:

    Linear<Features, Hidden>      Op_1{act.input, act.hidden, params.M, grad.M};
    Termwise<logistic, Hidden>    Op_2{act.hidden};
    Linear<Hidden, 1>             Op_3{act.hidden, act.output, params.V, grad.V};
    Termwise<bipolar_sigmoid, 1>  Op_4{act.output};
    Termwise<affine<3,0>, 1>      Op_5{act.output};

    float feedForward()
    {
        Op_1.fwd();
        Op_2.fwd();
        Op_3.fwd();
        Op_4.fwd();
        Op_5.fwd();
        return act.output;
    }

public:
    using Parameters = TwoLayerParameters<Features, Hidden>; 
    Parameters params;
    Parameters grad;

    struct Activations
    {
        using InputVector = vec<Features>;
        using HiddenVector = vec<Hidden>;
        using OutputVector = vec<1>;

        InputVector input;
        HiddenVector hidden;
        OutputVector output;
        float out;  // not used after new backprop is done.
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
    void gradient(Parameters& grad)
    {
        auto const f = Activ3::bwd(1) * Activ2::bwd(act.out);
        grad.V = f * act.hidden.Transpose();
        auto lhs = f * params.V.Transpose();

        for (int i = 0; i < lhs.Rows(); i++)
            lhs(i, 0) *= Activ1::bwd(act.hidden(i, 0));

        grad.M = lhs * act.input.Transpose();
    }

    void update_model(const Parameters& adj)
    {
        params += adj;
    }

    FcTwoLayerNet()
    {
        RNG_normal rand1(0, 1.0 / Features);
        for (auto& m : params.M)
            m = rand1.random();

        RNG_normal rand2(0, 1.0 / Hidden);
        for (auto& v : params.V)
            v = rand2.random();
    }
};
