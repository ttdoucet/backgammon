/* Written by Todd Doucet.  See file copyright.txt.
 */
#pragma once

#include "board.h"
#include "mathfuncs.h"
#include "matrix.h"
#include "random.h"

#include <iostream>

template<int N_INPUTS, int N_HIDDEN>
class net
{
public:
    constexpr static int n_inputs = N_INPUTS;
    constexpr static int n_hidden = N_HIDDEN;

    typedef matrix<N_INPUTS, 1> input_vector;
    typedef matrix<N_HIDDEN, 1> hidden_vector;

    typedef matrix<N_HIDDEN, N_INPUTS> W1;
    typedef matrix<1, N_HIDDEN> W2;

protected:
    float feedForward()
    {
        hidden = parms.M * input;

        for (int i = 0; i < N_HIDDEN; i++)
            hidden(i, 0) = squash(hidden(i, 0));

        return  out = squash( parms.V * hidden );
    }

    input_vector input;

public:
    /* Model parameters.
     */
    struct Parameters
    {
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
    };
    Parameters parms;

    void backprop(float err)
    {
        Parameters grad;

        auto const f = out * (1 - out);
        grad.V = f * hidden.Transpose();

        matrix<N_HIDDEN, 1> lhs = f * parms.V.Transpose();

        for (int i = 0; i < lhs.Rows(); i++)
            lhs(i, 0) *= ( hidden(i, 0) * (1 - hidden(i, 0)) );

        grad.M = lhs * input.Transpose();

        grad_adj += grad_sum * err;

        grad_sum *= lambda;
        grad_sum += grad;
    }

    float lambda = 0.85f;    // Temporal discount.
    float alpha = 0.001f;    // Learning rate.

    void clear_gradients()
    {
        grad_adj.clear();
        grad_sum.clear();
    }

    void update_model()
    {
        parms += grad_adj * (-alpha);
    }

    net()
    {
        RNG_normal rand1(0, 1.0 / N_INPUTS);
        for (int r = 0; r < N_HIDDEN; r++)
            for (int c = 0; c < N_INPUTS; c++)
                parms.M(r, c) = rand1.random();

        RNG_normal rand2(0, 1.0 / N_HIDDEN);
        for (int c = 0; c < N_HIDDEN; c++)
            parms.V(0, c) = rand2.random();
    }

private:
    /* State activations maintained after 
     * feedForward() for backpropagation.
     */
    hidden_vector hidden;
    float out;

    /* Sum of gradients with lambda temporal discount.
     */
    Parameters grad_sum;

    /* Accumulated adjustments to weights.
     */
    Parameters grad_adj;
};


/* Backgammon-specific class derivations.
 */

#include "features.h"

template<class feature_calc, int N_HIDDEN>
class BackgammonNet : public net<feature_calc::count, N_HIDDEN>
{
protected:
    constexpr static int MAX_EQUITY = 3;

    constexpr static float net_to_equity(float p)
    {
        return  (2*p - 1) * MAX_EQUITY;
    }

    constexpr static float delta_equity_to_delta_net(float ediff)
    {
        return  ediff / (2 * MAX_EQUITY);
    }

public:
    /* Neural net estimate of the equity for the side on roll.
     */
    float equity(const board &b) noexcept
    {
        feature_calc{b}.calc(this->input.Data());
        return net_to_equity( this->feedForward() );
    }

    void reconsider(float err)
    {
        this->backprop( delta_equity_to_delta_net(err) );
    }

    BackgammonNet()
    {
        assert( feature_calc::count == this->input.Rows() * this->input.Cols() );
    }

    uint64_t seed = 0;  // legacy
    int64_t games_trained = 0;  // legacy
};

using netv3 = BackgammonNet<features_v3<float*>, 30>;

using BgNet = netv3;

void readFile( BgNet& n, std::string fn);
void writeFile(BgNet& n, std::string fn);
