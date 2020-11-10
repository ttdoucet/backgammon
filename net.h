/* Written by Todd Doucet.  See file copyright.txt.
 */
#pragma once

#include "mathfuncs.h"
#include "matrix.h"
#include "random.h"

template<int Features, int Hidden>
struct TwoLayerParameters
{
    using W1 = matrix<Hidden, Features>;
    using W2 = matrix<1, Hidden>;

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

template<int Features,
         int Hidden,
         class Activ1,
         class Activ2,
         class Activ3 = affine<1, 0>
         >
class FcTwoLayerNet
{
protected:

    float feedForward()
    {
        act.hidden = params.M * act.input;

        for (int i = 0; i < Hidden; i++)
            act.hidden(i, 0) = Activ1::fwd(act.hidden(i, 0));

        float x =  params.V * act.hidden;
        act.out = Activ2::fwd(x);
        return Activ3::fwd(act.out);
    }

public:
    using Parameters = TwoLayerParameters<Features, Hidden>; 
    Parameters params;

    struct Activations
    {
        using InputVector = matrix<Features, 1>;
        using HiddenVector = matrix<Hidden, 1>;
        
        InputVector input;
        HiddenVector hidden;
        float out;
    } act;


    using InputVector = typename Activations::InputVector;

    InputVector& input()
    {
        return act.input;
    }

    /* Computes dy/dx, where y is the scalar output
     * of the net during the last forward calculation,
     * and x is any learning parameter.
     */
    void backprop(Parameters& grad)
    {
        auto const f = Activ3::bwd(1) * Activ2::bwd(act.out);

        grad.V = f * act.hidden.Transpose();

        matrix<Hidden, 1> lhs = f * params.V.Transpose();

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
        for (int r = 0; r < Hidden; r++)
            for (int c = 0; c < Features; c++)
                params.M(r, c) = rand1.random();

        RNG_normal rand2(0, 1.0 / Hidden);
        for (int c = 0; c < Hidden; c++)
            params.V(0, c) = rand2.random();
    }
};

template<int Features, int Hidden>
class MiscNet : public FcTwoLayerNet<Features, Hidden,
                                     bipolar_sigmoid,
                                     bipolar_sigmoid,
                                     affine<3, 0> > { };

template<int Features, int Hidden>
class SigmoidNet : public FcTwoLayerNet<Features, Hidden,
                                        logistic,
                                        bipolar_sigmoid,
                                        affine<3, 0> > { };
