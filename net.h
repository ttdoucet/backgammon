/* Written by Todd Doucet.  See file copyright.txt.
 */
#pragma once

#include "board.h"
#include "mathfuncs.h"
#include "matrix.h"

template<int N_INPUTS, int N_HIDDEN>
class net
{
public:
    constexpr static int n_inputs = N_INPUTS;
    constexpr static int n_hidden = N_HIDDEN;

    typedef matrix<N_INPUTS, 1> input_vector;
    typedef matrix<N_HIDDEN, 1> hidden_vector;

protected:
    float feedForward(bool learn=false)
    {
        auto hidden = M * input;

        for (int i = 0; i < N_HIDDEN; i++)
            hidden(i) = squash(hidden(i));

        auto out = squash( V * hidden );

        if (learn)
        {
            auto const f = out * (1 - out);
            V_grad = f * hidden.Transpose();

            auto lhs = f * V.Transpose();
            for (int i = 0; i < lhs.Cols(); i++)
                lhs *= ( hidden(i) * (1 - hidden(i)) );
            M_grad = lhs * input.Transpose();
        }

        return out;
    }

    input_vector input;

public:
    /* Model parameters & gradients of parameters.
     */
    matrix<N_HIDDEN, N_INPUTS> M, M_grad;
    matrix<1, N_HIDDEN> V, V_grad;
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

    constexpr static float equity_to_net(float e)
    {
        return  (e/MAX_EQUITY + 1) / 2;
    }

public:
    /* Neural net estimate of the equity for the side on roll.
     */
    float equity(const board &b) noexcept
    {
        feature_calc{b}.calc(this->input.Data());
        return net_to_equity( this->feedForward() );
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

BgNet *readFile(const char *fn);
void writeFile(BgNet& n, const char *fn);
