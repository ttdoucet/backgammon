#pragma once

#include <emmintrin.h>
#include <xmmintrin.h>

#include <cmath>
#include <cassert>
#include <numeric>

#include "board.h"
#include "hits.h"
#include "console.h"

template<int N>
inline float dotprod(float *vec1, float *vec2)
{
    float r = 0.0f;
 
    float sum = 0.0f;
    for (int i = 0; i < N; ++i)
        sum += (vec1[i] * vec2[i]);

    return sum;
}

//#define FULL_CALC

static const __m128 maxx = _mm_set1_ps(87);
static const __m128 minx = _mm_set1_ps(-87);
static const __m128 one = _mm_set_ss(1);
static const __m128 c = _mm_set_ss(-8388608/0.6931471806);
static const __m128 b = _mm_set_ss(1065353216);

class net {
    constexpr static int N_HIDDEN = 30;
    constexpr static int N_INPUTS = 156;
    constexpr static int stride = 192;

    constexpr static float MAX_EQUITY = 3.0f;

    constexpr static float delta_equity_to_delta_net(float de)
    {
        return (float)( (de)/ (MAX_EQUITY * 2.0f) );
    }

    constexpr static float net_to_equity(float n)
    {
        return (MAX_EQUITY * 2.0f) * n - MAX_EQUITY;
    }


public:
    static net *read_network(const char *fn);
    void dump_network(const char *fn, int portable = 0);
    void init_play();
    unsigned long get_seed() const { return seed; }

    /* Apply the current backgammon board to the input,
     * and compute the net's output.
     */
    float feedBoard(const board &b)
    {
        netboard = b;
#ifdef FULL_CALC
        compute_input(b.colorOnRoll(), input);
        return  feedForward();
#else
        compute_input(b.colorOnRoll(), inbuf);
        return  feedForward_marginal();
#endif
    }

    net()
    {
        for (int i = 0; i < N_INPUTS; i++)
            input[i] = 0.0f;

        console << "net_v3(hidden=" << N_HIDDEN << ")\n";
    }

private:
    void compute_v3_inputs(const color_t color, float *ib)
    {
        // The first two are the same as net_v2.
        compute_contact(color, ib);
        compute_pip(color, ib + metrics::contact);

        // Here we represent the hit danger and hit attack differently.

        compute_hit_danger_v3(color, ib + metrics::contact + metrics::pip);
        compute_hit_danger_v3(opponentOf(color), ib + metrics::contact +
                              metrics::pip + metrics::hit_v3);
    }

    void compute_input(const color_t color, float *inbuf)
    {
        compute_input_for(color, inbuf);
        compute_input_for(opponentOf(color), inbuf + metrics::n_check);

        compute_v3_inputs(color, inbuf + (2 * metrics::n_check) );
    }

    enum /* class */ metrics {
        hit = 9,
        contact  = 2,
        pip  = 2,
        n_rel = (contact + pip + 2*hit ),
        hit_v3  = 2,
        cross_v4 = 4,
        // blot, point, builders for each point, plus bar, borne-off.
        n_check = (3*24 + 2),
        inputsForV1 = 2 * n_check,
        inputsForV2 = inputsForV1 + contact + pip + 2 * metrics::hit,
        inputsForV3 = inputsForV1 + contact + pip + 2 * hit_v3,
    };

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
#ifndef FULL_CALC
    alignas(16) float inbuf[N_INPUTS];
#endif
    alignas(16) float input[N_INPUTS];
    alignas(16) float hidden[N_HIDDEN];
    alignas(16) float pre_hidden[N_HIDDEN];
    alignas(16) float weights_1[N_HIDDEN][stride];
    alignas(16) float weights_2[N_HIDDEN];
    float output;

    unsigned long seed;  // This is a bad idea.
    const char *filename;
    board netboard;

    // no reason for this to me even a static method?
    static inline float squash_sse(const float x)
    {
#if 0
        return (float) (1 / (1 + expf(-x)) );
#else
        const __m128 y = _mm_max_ss(minx, _mm_min_ss(maxx, _mm_set_ss(x))); // clamp to [-87,87]
        const __m128 z = _mm_add_ss(_mm_mul_ss(y, c), b);
        const __m128i i = _mm_cvtps_epi32(z);
        const float r = _mm_cvtss_f32(_mm_rcp_ss(_mm_add_ss(_mm_load_ps((const float *)&i), one)));
        // assert(std::abs(1/(1+expf(-x)) - r) < 1.48e-2);  // minimum accuracy on floats is 1.48e-2
        return r;
#endif
    }

    inline static float squash(const float f)
    {
        return (float) (1 / (1 + expf(-f)) );
    }

    /*
     * Have the network evaluate its input.
     */
    float feedForward()
    {
        for (int i = 0; i < N_HIDDEN; i++)
            pre_hidden[i] = dotprod<N_INPUTS>(input, weights_1[i]);

        for (int i = 0; i < N_HIDDEN; i++)
            hidden[i] = squash_sse(pre_hidden[i]);

        output = squash_sse(dotprod<N_HIDDEN>(hidden, weights_2));
        return net_to_equity(output);
    }

#ifndef FULL_CALC
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
        for (int j = 0; j < N_HIDDEN; j++)
            hidden[j] = squash_sse(pre_hidden[j]);

        float f = dotprod<N_HIDDEN>(hidden, weights_2);

        output = squash_sse(f);
        return net_to_equity(output);
    }
#endif

    /*
     * Encode the board as the network input.
     * 
     * The input can be divided into three types:
     *  1. Functions of our checker configuration. (N_CHECK inputs)
     *  2. Functions of the opponent's checker configuration. (N_CHECK)
     *  3. Functions of the relationship of the two checker positions. 
     * 
     */
    void compute_input_for(const color_t color, float *ib)
    {
        int i, n;

        *ib++ = (float) netboard.checkersOnPoint(color, 0); /* borne off */

        for (i = 1; i <= 24; i++){
            n = netboard.checkersOnPoint(color, i);
            *ib++ = (float) (n == 1);                   /* blot     */
            *ib++ = (float) (n >= 2);                   /* point    */
            *ib++ = (float) ( (n > 2) ? (n - 2) : 0 );  /* builders */
        }
        *ib = (float) netboard.checkersOnBar(color);          /* on bar   */
    }

    void compute_contact(const color_t color, float *ib)
    {
        int me = netboard.highestChecker(color);
        int him = opponentPoint(netboard.highestChecker(opponentOf(color)));
        int contact = me - him;
        float f = contact < 0.0f ? 0.0f : (contact / 23.0f);

        *ib++ = f;
        *ib++ = (1.0f - f);
    }

    void compute_pip(const color_t color, float *ib)
    {
        int pdiff = netboard.pipCount(opponentOf(color)) - netboard.pipCount(color);

        ib[0] = squash_sse( ((float) pdiff) / 27.0f);
        ib[1] = 1.0f - ib[0];
    }

    void compute_hit_danger_v3(const color_t color, float *ib)
    {
        float h = ((float) num_hits(color, netboard)) / 36.0f;

        ib[0] = h;
        ib[1] = 1.0f - h;
    }

} ;
