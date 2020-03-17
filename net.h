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
    constexpr static float MAX_EQUITY = 3.0f;

    constexpr static float net_to_equity(float p)
    {
        return  (2 * p - 1) * MAX_EQUITY;
    }

    virtual float feedForward()
    {
        pre_hidden = M * input;

        for (int i = 0; i < N_HIDDEN; i++)
            hidden(i) = squash(pre_hidden(i));

        return squash( hidden.Transpose() * V );
    }

    /* Activations.
     */
    input_vector input;
    hidden_vector pre_hidden;
    hidden_vector hidden;

public:
    /* Model parameters.
     */
    matrix<N_HIDDEN, N_INPUTS> M;
    matrix<N_HIDDEN, 1> V;
};


/* Backgammon-specific class derivations.
 */

#include "features.h"

template<class feature_calc, int N_HIDDEN>
class BackgammonNet : public net<feature_calc::count, N_HIDDEN>
{
public:
    /* Neural net estimate of the equity for the side on roll.
     */
    float equity(const board &b) noexcept
    {
        feature_calc{b}.calc(this->input.Data());
        return this->feedForward();
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
