#pragma once 

#include <cassert>

#include "board.h"
#include "hits.h"
#include "mathfuncs.h"

template<typename iter>
class features
{
public:
    constexpr static int count = 156;

    features(const board &b) : netboard(b) {}

    iter calc(iter dest) const
    {
        compute_input(netboard.onRoll(), dest);
        return dest + features::count;
    }

private:
    const board netboard;

    iter compute_contact(const color_t color, iter ib) const
    {
        int me = netboard.highestChecker(color);
        int him = opponentPoint(netboard.highestChecker(opponentOf(color)));
        int contact = me - him;
        float f = contact < 0.0f ? 0.0f : (contact / 23.0f);

        *ib++ = f;
        *ib++ = (1.0f - f);
        return ib;
    }

    static float squash(const float f)
    {
        return 1 / (1 + expf(-f));
    }

    iter compute_pip(const color_t color, iter ib) const
    {
        int pdiff = netboard.pipCount(opponentOf(color)) - netboard.pipCount(color);

        float h = squash_sse(pdiff / 27.0f);
        *ib++ = h;
        *ib++ = 1.0f - h;
        return ib;
    }

    iter compute_hit_danger_v3(const color_t color, iter ib) const
    {
        float h = num_hits(color, netboard) / 36.0f;

        *ib++ = h;
        *ib++ = 1.0f - h;
        return ib;
    }

    iter compute_input_for(const color_t color, iter ib) const
    {
        int i, n;

        *ib++ = netboard.checkersOnPoint(color, 0); /* borne off */

        for (i = 1; i <= 24; i++)
        {
            n = netboard.checkersOnPoint(color, i);
            *ib++ = (n == 1);                   /* blot     */
            *ib++ = (n >= 2);                   /* point    */
            *ib++ = ( (n > 2) ? (n - 2) : 0 );  /* builders */
        }
        *ib++ = netboard.checkersOnBar(color);          /* on bar   */
        return ib;
    }

    iter compute_v3_inputs(const color_t color, iter ib) const
    {
        // The first two are the same as net_v2.
        ib = compute_contact(color, ib);
        ib = compute_pip(color, ib);

        // Here we represent the hit danger and hit attack differently.
        ib = compute_hit_danger_v3(color, ib);
        ib = compute_hit_danger_v3(opponentOf(color), ib);

        return ib;
    }

    /*
     * Encode the board as the network input.
     */
    void compute_input(const color_t color, iter ib) const
    {
        iter start = ib;

        ib = compute_input_for(color, ib);
        ib = compute_input_for(opponentOf(color), ib);
        ib = compute_v3_inputs(color, ib);
        
        assert( (ib - start) == features::count);
    }
};

template<typename V>
inline void features_v3(const board& b, V dest)
{
//    features<typename V::iterator>{b}.calc(dest.begin() );
    features<float *>{b}.calc(dest );

}
