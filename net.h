/* Written by Todd Doucet.  See file copyright.txt.
 */
#pragma once

#include "mathfuncs.h"
#include "matrix.h"
#include "random.h"

template<int FEATURES, int HIDDEN>
struct TwoLayerParameters
{
    using W1 = matrix<HIDDEN, FEATURES>;
    using W2 = matrix<1, HIDDEN>;

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

template<int FEATURES, int HIDDEN>
class SigmoidNet
{
protected:
    constexpr static int MAX_EQUITY = 3;

    constexpr static float net_to_equity(float p)
    {
        return  (2*p - 1) * MAX_EQUITY;
    }

    float feedForward()
    {
        act.hidden = params.M * act.input;

        for (int i = 0; i < HIDDEN; i++)
            act.hidden(i, 0) = logistic::fwd(act.hidden(i, 0));

        float x =  params.V * act.hidden;
        act.out = logistic::fwd(x);
        return net_to_equity(act.out);
    }

public:

    /* Model parameters.
     */
    using Parameters = TwoLayerParameters<FEATURES, HIDDEN>; 
    using InputVector =  matrix<FEATURES, 1>;

    Parameters params;

    struct Activations
    {
        using HiddenVector = matrix<HIDDEN, 1>;

        InputVector input;
        HiddenVector hidden;
        float out;
    } act;

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
        auto const f = 2 * MAX_EQUITY * logistic::bwd(act.out);

        grad.V = f * act.hidden.Transpose();

        matrix<HIDDEN, 1> lhs = f * params.V.Transpose();

        for (int i = 0; i < lhs.Rows(); i++)
            lhs(i, 0) *= logistic::bwd(act.hidden(i, 0));

        grad.M = lhs * act.input.Transpose();
    }

    void update_model(const Parameters& adj)
    {
        params += adj;
    }

    SigmoidNet()
    {
        RNG_normal rand1(0, 1.0 / FEATURES);
        for (int r = 0; r < HIDDEN; r++)
            for (int c = 0; c < FEATURES; c++)
                params.M(r, c) = rand1.random();

        RNG_normal rand2(0, 1.0 / HIDDEN);
        for (int c = 0; c < HIDDEN; c++)
            params.V(0, c) = rand2.random();
    }
};

// This net in intended for experiments and investigations,
// and its specific implementation could change often.  When
// something is found which is desired to be kept, it can
// be renamed to something else.  So any network data files
// saved in this format are subject to being orphaned.
//
template<int FEATURES, int HIDDEN>
class MiscNet
{
protected:
    constexpr static int MAX_EQUITY = 3;

    constexpr static float net_to_equity(float p)
    {
        return  p * MAX_EQUITY;
    }

    float feedForward()
    {
        act.hidden = params.M * act.input;

        for (int i = 0; i < HIDDEN; i++)
            act.hidden(i, 0) = bipolar_sigmoid::fwd(act.hidden(i, 0));

        float x =  params.V * act.hidden;
        act.out = bipolar_sigmoid::fwd(x); 
        return net_to_equity(act.out);
    }

public:
    using Parameters = TwoLayerParameters<FEATURES, HIDDEN>; 
    using InputVector =  matrix<FEATURES, 1>;

    Parameters params;

    struct Activations
    {
        using HiddenVector = matrix<HIDDEN, 1>;

        InputVector input;
        HiddenVector hidden;
        float out;
    } act;

    InputVector& input()
    {
        return act.input;
    }

    void backprop(Parameters& grad)
    {
        auto const f = MAX_EQUITY * bipolar_sigmoid::bwd(act.out);

        grad.V = f * act.hidden.Transpose();

        matrix<HIDDEN, 1> lhs = f * params.V.Transpose();

        for (int i = 0; i < lhs.Rows(); i++)
            lhs(i, 0) *= bipolar_sigmoid::bwd(act.hidden(i, 0));

        grad.M = lhs * act.input.Transpose();
    }

    void update_model(const Parameters& adj)
    {
        params += adj;
    }

    MiscNet()
    {
        RNG_normal rand1(0, 1.0 / FEATURES);
        for (int r = 0; r < HIDDEN; r++)
            for (int c = 0; c < FEATURES; c++)
                params.M(r, c) = rand1.random();

        RNG_normal rand2(0, 1.0 / HIDDEN);
        for (int c = 0; c < HIDDEN; c++)
            params.V(0, c) = rand2.random();
    }
};

