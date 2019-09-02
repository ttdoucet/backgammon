#pragma once

#include <Eigen/Dense>

#include "board.h"
#include "mathfuncs.h"

#include "console.h"

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
        return weights_1(r,c);
    }

    float& V(int i)
    {
        return weights_2(i, 0);
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
        pre_hidden = weights_1 * input;
        for (int i = 0; i < N_HIDDEN; i++)
            hidden(i, 0) = squash_sse(pre_hidden(i, 0));

        float output = squash_sse( hidden.dot(weights_2) );
        return net_to_equity(output);
    }

protected:
    /* Activations.
     */
    Eigen::Matrix<float, N_INPUTS, 1> input;
    Eigen::Matrix<float, N_HIDDEN, 1> pre_hidden;
    Eigen::Matrix<float, N_HIDDEN, 1> hidden;

    /* Model parameters.
     */
    Eigen::Matrix<float, N_HIDDEN, N_INPUTS, Eigen::RowMajor> weights_1;
    Eigen::Matrix<float, N_HIDDEN, 1> weights_2;
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
        feature_calc{b}.calc(this->input.data());
        return this->feedForward();
    }

    unsigned long seed = 0;  // legacy
    long games_trained = 0;  // legacy
};

using netv3 = BackgammonNet<features_v3<float*>, 30>;

#if 0
class netv3_marginal : public netv3
{
    alignas(16) input_vector prev_input;

public:
    float feedForward() override
    {
        static int count = 0;
        if (count-- == 0)
        {
            for (int i = 0; i < n_inputs; i++)
                prev_input[i] = 0;
            for (int i = 0; i < n_hidden; i++)
                pre_hidden[i] = 0;
            count = 500;
        }

        for (int i = 0; i < n_inputs ; i++)
        {
            if (prev_input[i] == input[i])
                continue;

            float d = input[i] - prev_input[i] ;
            prev_input[i] = input[i];

            for (int j = 0; j < n_hidden; j++)
                pre_hidden[j] += d * M(j, i);

        }
        for (int j = 0; j < n_hidden; j++)
            hidden[j] = squash_sse(pre_hidden[j]);

        float f = dotprod<n_hidden>(hidden, weights_2);
        return net_to_equity( squash_sse(f) );
    }
};
#endif

using BgNet = netv3;
//using BgNet = netv3_marginal;

BgNet *readFile(const char *fn);
void writeFile(BgNet& n, const char *fn);
