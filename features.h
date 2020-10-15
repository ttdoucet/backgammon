/* Written by Todd Doucet.  See file copyright.txt.
 */
#pragma once 

#include <cassert>

#include "board.h"
#include "hits.h"
#include "mathfuncs.h"

template<typename iter>
class features_v3
{
public:
    constexpr static int count = 156;
    features_v3() = delete;

    static iter calc(const board &b, iter dest)
    {
        iter end = compute_input(b, b.onRoll(), dest);
        assert( (end - dest) == 156);
        return end;
    }

private:
    static iter compute_contact(const board& b, const color_t color, iter ib) // const
    {
        int me = b.highestChecker(color);
        int him = opponentPoint(b.highestChecker(opponentOf(color)));
        int contact = me - him;
        float f = contact < 0.0f ? 0.0f : (contact / 23.0f);

        *ib++ = f;
        *ib++ = (1.0f - f);
        return ib;
    }

    static iter compute_pip(const board& b, const color_t color, iter ib) // const
    {
        int pdiff = b.pipCount(opponentOf(color)) - b.pipCount(color);

        float h = squash(pdiff / 27.0f);
        *ib++ = h;
        *ib++ = 1.0f - h;
        return ib;
    }

    static iter compute_hit_danger_v3(const board& b, const color_t color, iter ib) // const
    {
        float h = num_hits(color, b) / 36.0f;

        *ib++ = h;
        *ib++ = 1.0f - h;
        return ib;
    }

    static iter compute_input_for(const board& b, const color_t color, iter ib) // const
    {
        int i, n;

        *ib++ = b.checkersOnPoint(color, 0);    /* borne off */

        for (i = 1; i <= 24; i++)
        {
            n = b.checkersOnPoint(color, i);
            *ib++ = (n == 1);                   /* blot     */
            *ib++ = (n >= 2);                   /* point    */
            *ib++ = ( (n > 2) ? (n - 2) : 0 );  /* builders */
        }

        *ib++ = b.checkersOnBar(color);         /* on bar   */
        return ib;
    }

    static iter compute_v3_inputs(const board& b, const color_t color, iter ib) // const
    {
        // The first two are the same as net_v2.
        ib = compute_contact(b, color, ib);
        ib = compute_pip(b, color, ib);

        // Here we represent the hit danger and hit attack differently.
        ib = compute_hit_danger_v3(b, color, ib);
        ib = compute_hit_danger_v3(b, opponentOf(color), ib);

        return ib;
    }

    /* Encode the board as the network input.
     */
    static iter compute_input(const board& b, const color_t color, iter ib) // const
    {
        ib = compute_input_for(b, color, ib);
        ib = compute_input_for(b, opponentOf(color), ib);
        ib = compute_v3_inputs(b, color, ib);
        return ib;
    }
};

