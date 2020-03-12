/* Written by Todd Doucet.  See file copyright.txt.
 */
#pragma once

#include "board.h"
#include "mathfuncs.h"

template<int N_INPUTS, int N_HIDDEN>
class net
{
public:
    constexpr static int n_inputs = N_INPUTS;
    constexpr static int n_hidden = N_HIDDEN;

    typedef float input_vector[N_INPUTS];
    typedef float hidden_vector[N_HIDDEN];

    /* Access to model parameters.
     */
    float& M(int r, int c)
    {
        return weights_1[r][c];
    }

    float& V(int i)
    {
        return weights_2[i];
    }

protected:
    constexpr static float MAX_EQUITY = 3.0f;

    constexpr static float net_to_equity(float p)
    {
        return  (2 * p - 1) * MAX_EQUITY;
    }

    /* Evaluate the net on its input.
     */
    virtual float feedForward()
    {
        // invites vectorization
        for (int i = 0; i < N_HIDDEN; i++)
            pre_hidden[i] = dotprod<N_INPUTS>(input, weights_1[i]);

        for (int i = 0; i < N_HIDDEN; i++)
            hidden[i] = squash(pre_hidden[i]);

        return squash(dotprod<N_HIDDEN>(hidden, weights_2));
    }

    /* Activations.
     */
    alignas(16) input_vector input;
    alignas(16) hidden_vector pre_hidden;
    alignas(16) hidden_vector hidden;

    /* Model parameters.
     */
    alignas(16) float weights_1[N_HIDDEN][N_INPUTS];
    alignas(16) float weights_2[N_HIDDEN];
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
        feature_calc{b}.calc(this->input);
        return this->feedForward();
    }

    uint64_t seed = 0;  // legacy
    int64_t games_trained = 0;  // legacy
};

using netv3 = BackgammonNet<features_v3<float*>, 30>;

using BgNet = netv3;

BgNet *readFile(const char *fn);
void writeFile(BgNet& n, const char *fn);
