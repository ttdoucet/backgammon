/* Written by Todd Doucet.  See file copyright.txt.
 */
#pragma once

#include "mathfuncs.h"
#include "matrix.h"
#include "random.h"

template<int N_INPUTS, int N_HIDDEN>
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
        A.hidden = params.M * A.input;

        for (int i = 0; i < N_HIDDEN; i++)
            A.hidden(i, 0) = squash(A.hidden(i, 0));

        float x =  params.V * A.hidden;
        A.out = squash(x);
        return net_to_equity(A.out);
    }

public:
    /* Model parameters.
     */
    struct Parameters
    {
        using W1 = matrix<N_HIDDEN, N_INPUTS>;
        using W2 = matrix<1, N_HIDDEN>;

        W1 M;
        W2 V;

        void clear()
        {
            M.clear();
            V.clear();
        }

        Parameters& operator+=(const Parameters &rhs)
        {
            M += rhs.M;
            V += rhs.V;
            return *this;
        }

        Parameters& operator*=(float scale)
        {
            M *= scale;
            V *= scale;
            return *this;
        }

        Parameters operator*(float scale) const
        {
            Parameters r(*this);
            return r *= scale;
        }
    } params;  // maybe P ?

    using InputVector =  matrix<N_INPUTS, 1>;

    struct Activations
    {
        using HiddenVector = matrix<N_HIDDEN, 1>;

        InputVector input;
        HiddenVector hidden;
        float out;
    } A;

    InputVector& input()
    {
        return A.input;
    }

    /* Computes dy/dx, where y is the scalar output
     * of the net during the last forward calculation,
     * and x is any learning parameter.
     */
    void backprop(Parameters& grad)
    {
        auto const f = 2 * MAX_EQUITY * A.out * (1 - A.out);

        grad.V = f * A.hidden.Transpose();

        matrix<N_HIDDEN, 1> lhs = f * params.V.Transpose();

        for (int i = 0; i < lhs.Rows(); i++)
            lhs(i, 0) *= ( A.hidden(i, 0) * (1 - A.hidden(i, 0)) );

        grad.M = lhs * A.input.Transpose();
    }

    void update_model(const Parameters& adj)
    {
        params += adj;
    }

    SigmoidNet()
    {
        RNG_normal rand1(0, 1.0 / N_INPUTS);
        for (int r = 0; r < N_HIDDEN; r++)
            for (int c = 0; c < N_INPUTS; c++)
                params.M(r, c) = rand1.random();

        RNG_normal rand2(0, 1.0 / N_HIDDEN);
        for (int c = 0; c < N_HIDDEN; c++)
            params.V(0, c) = rand2.random();
    }
};

