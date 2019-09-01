#pragma once

#include "board.h"
#include "features.h"
#include "mathfuncs.h"

#include "stopwatch.h"

class net
{
    constexpr static int N_HIDDEN = 30;
    constexpr static int N_INPUTS = 156;

public:
    typedef float input_vector[N_INPUTS];
    typedef float hidden_vector[N_HIDDEN];

    static net *readFile(const char *fn);
    void writeFile(const char *fn);

    net()
    {
        init_play();
    }

    /*
     * Access to model parameters.
     */

    float& M(int r, int c)
    {
        return weights_1[r][c];
    }

    float& V(int i)
    {
        return weights_2[i];
    }

private:
    constexpr static int stride = N_INPUTS;
    constexpr static bool full_calc = true;
    constexpr static float MAX_EQUITY = 3.0f;

    constexpr static float net_to_equity(float p)
    {
        return  (2 * p - 1) * MAX_EQUITY;
    }

    void init_play()
    {
        if constexpr (full_calc == 0)
        {
            for (int i = 0; i < N_INPUTS; i++)
                prev_input[i] = 0;
            feedForward(prev_input);
        }
    }

    // Applies a functor to each weight parameter in the network.
    template<class Ftn> void applyFunction(Ftn f)
    {
        for (int i = 0; i < N_HIDDEN; i++)
        {
            for (int j = 0; j < N_INPUTS; j++)
                f( M(i, j) );
        }
        for (int i = 0; i < N_HIDDEN; i++)
            f( V(i) );
    }

    /*
     * Have the network evaluate its input.
     */
    float feedForward(float* input)
    {
        for (int i = 0; i < N_HIDDEN; i++)
            pre_hidden[i] = dotprod<N_INPUTS>(input, weights_1[i]);

        for (int i = 0; i < N_HIDDEN; i++)
            hidden[i] = squash_sse(pre_hidden[i]);

        output = squash_sse(dotprod<N_HIDDEN>(hidden, weights_2));
        return net_to_equity(output);
    }

    float feedForward_marginal(float* input)
    {
        static int count = 0;

        if (count++ == 100)
        {
            init_play();
            count = 0;
        }

        for (int i = 0; i < N_INPUTS ; i++)
        {
            if (prev_input[i] == input[i])
                continue;

            float d = input[i] - prev_input[i] ;
            prev_input[i] = input[i];

            for (int j = 0; j < N_HIDDEN; j++)
                pre_hidden[j] += d * M(j, i);

        }
        for (int j = 0; j < N_HIDDEN; j++)
            hidden[j] = squash_sse(pre_hidden[j]);

        float f = dotprod<N_HIDDEN>(hidden, weights_2);
        output = squash_sse(f);
        return net_to_equity(output);
    }


    /* Model parameters.
     */
    alignas(16) float weights_1[N_HIDDEN][stride];
    alignas(16) float weights_2[N_HIDDEN];

    /* Activations.
     */
    alignas(16) hidden_vector hidden;
    alignas(16) hidden_vector pre_hidden;

    float output;

    alignas(16) input_vector prev_input;

    unsigned long seed = 0;  // legacy
    long games_trained = 0;  // legacy


public:
    alignas(16) input_vector features;

    /* Neural net estimate of the equity for the side on roll. */
    float equity(const board &b) noexcept
    {
        features_v3(b, features);

        if constexpr (full_calc)
            return feedForward(features);
        else
            return feedForward_marginal(features);
    }
};
