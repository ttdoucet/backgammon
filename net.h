#pragma once

#include <cmath>
#include <cassert>
#include <numeric>

#include "board.h"
#include "features.h"
#include "mathfuncs.h"

#include "stopwatch.h"
extern stopwatch mtimer, ftimer, stimer;


class net
{
public:
    static net *readFile(const char *fn);
    void writeFile(const char *fn);

    /* Neural net estimate for the equity of the side on roll. */
    float equity(const board &b) noexcept
    {
        features feat{b};

        if constexpr (full_calc)
        {
            feat.calc(input);

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
            feat.calc(inbuf);

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

private:
    constexpr static int N_HIDDEN = 30;
    constexpr static int N_INPUTS = 156;
    constexpr static int stride = 192;
    constexpr static bool full_calc = false;

    constexpr static float MAX_EQUITY = 3.0f;

    constexpr static float delta_equity_to_delta_net(float de)
    {
        return (float)( (de)/ (MAX_EQUITY * 2.0f) );
    }

    constexpr static float net_to_equity(float n)
    {
        return (MAX_EQUITY * 2.0f) * n - MAX_EQUITY;
    }

    void init_play()
    {
        for (int i = 0; i < N_INPUTS; i++)
            input[i] = 0.0f;
        // This makes future marginal calculations work.
        feedForward();
    }

    /*
     * This takes a routine or function object to apply to each weight
     * of the network.  The routine or function object is passed a
     * pointer to the weight.  Depending on what is passed in, we can
     * randomly initialize the network, read the network from a file,
     * clear the network, or write the network to a file.
     */
    template<class Ftn> void applyFunction(Ftn f)
    {
        for (int i = 0; i < N_HIDDEN; i++)
        {
            for (int j = 0; j < N_INPUTS; j++)
                f( weights_1[i][j] );
        }
        for (int i = 0; i < N_HIDDEN; i++)
            f( weights_2[i] );
    }

    // data members
    alignas(16) float inbuf[N_INPUTS];
    alignas(16) float input[N_INPUTS];
    alignas(16) float hidden[N_HIDDEN];
    alignas(16) float pre_hidden[N_HIDDEN];
    alignas(16) float weights_1[N_HIDDEN][stride];
    alignas(16) float weights_2[N_HIDDEN];
    float output;

    unsigned long seed = 0;  // legacy
    long games_trained = 0;  // legacy
    const char *filename;

    inline static float squash(const float f)
    {
        return 1 / (1 + expf(-f));
    }

    /*
     * Have the network evaluate its input.
     */
    float feedForward()
    {
        for (int i = 0; i < N_HIDDEN; i++)
            pre_hidden[i] = dotprod<N_INPUTS>(input, weights_1[i]);

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
                pre_hidden[j] += d * weights_1[j][i];

        }
        
        stimer.start();

        for (int j = 0; j < N_HIDDEN; j++)
            hidden[j] = squash_sse(pre_hidden[j]);

        stimer.stop();

        float f = dotprod<N_HIDDEN>(hidden, weights_2);

        output = squash_sse(f);
        return net_to_equity(output);
    }

};
