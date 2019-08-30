#pragma once

#include "board.h"
#include "features.h"
#include "mathfuncs.h"

#include "stopwatch.h"
extern stopwatch mtimer, ftimer, stimer;

template<int SIZE, typename T>
class Vector
{
public:
    T v[SIZE];

    Vector()
    {
    }

    Vector(const Vector&) = delete;

public:
    T& operator [](int i)
    {
        return v[i];
    }

    int size() const
    {
        return SIZE;
    }

    using iterator=T*;

    T* begin()
    {
        return v;
    }

    T* end()
    {
        return v + size();
    }
};

class net
{
public:
    static net *readFile(const char *fn);
    void writeFile(const char *fn);

    /* Neural net estimate of the equity for the side on roll. */
    float equity(const board &b) noexcept
    {
        if constexpr (full_calc)
        {
            features_v3(b, input);

            mtimer.start();
            auto v = feedForward();
            mtimer.stop();

            return v;
        }
        else
        {
            static int count = 0;

            if (count++ == 100)
            {
                init_play();
                count = 0;
            }
            features_v3(b, inbuf);

            mtimer.start();
            auto v = feedForward_marginal();
            mtimer.stop();
            return v;
        }
    }

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
    constexpr static int N_HIDDEN = 30;
    constexpr static int N_INPUTS = 156;
    constexpr static int stride = N_INPUTS;
    constexpr static bool full_calc = false;
    constexpr static float MAX_EQUITY = 3.0f;

    constexpr static float net_to_equity(float p)
    {
        return  (2 * p - 1) * MAX_EQUITY;
    }

    void init_play()
    {
        for (int i = 0; i < N_INPUTS; i++)
            input[i] = 0.0f;
        feedForward();
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
    float feedForward()
    {
        for (int i = 0; i < N_HIDDEN; i++)
        {
            pre_hidden[i] = dotprod<N_INPUTS>(input.begin(), weights_1[i]);

            // I think this one makes a copy of input and is a little slower.
            // So input would come back empty but dotprod would return
            // the correct value.
            // pre_hidden[i] = dotprod<N_INPUTS>(input, weights_1[i]);
        }

        stimer.start();
        for (int i = 0; i < N_HIDDEN; i++)
            hidden[i] = squash_sse(pre_hidden[i]);
        stimer.stop();

        output = squash_sse(dotprod<N_HIDDEN>(hidden, weights_2));
        return net_to_equity(output);
    }

    float feedForward_marginal()
    {
        for (int i = 0; i < N_INPUTS ; i++)
        {
            if (input[i] == inbuf[i])
                continue;

            float d = inbuf[i] - input[i] ;
            input[i] = inbuf[i];

            for (int j = 0; j < N_HIDDEN; j++)
                pre_hidden[j] += d * M(j, i);

        }
        stimer.start();
        for (int j = 0; j < N_HIDDEN; j++)
            hidden[j] = squash_sse(pre_hidden[j]);
        stimer.stop();

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
//  alignas(16) float inbuf[N_INPUTS];
//  alignas(16) float input[N_INPUTS];
    alignas(16) Vector<N_INPUTS, float> inbuf;
    alignas(16) Vector<N_INPUTS, float> input;

    alignas(16) float hidden[N_HIDDEN];
    alignas(16) float pre_hidden[N_HIDDEN];

    float output;

    unsigned long seed = 0;  // legacy
    long games_trained = 0;  // legacy
};
