/* Written by Todd Doucet.  */
#pragma once 
#include <cassert>

#include "board.h"
#include "hits.h"

class features_v3
{
public:
    constexpr static int count = 156;
    features_v3() = delete;

    static float* calc(board const& b, float *dest)
    {
        float *ib = dest;
        const color_t on_roll = b.onRoll();

        ib = board_features(b, on_roll, ib);
        ib = board_features(b, opponentOf(on_roll), ib);

        ib = contact_features(b, on_roll, ib);
        ib = pip_features(b, on_roll, ib);

        ib = hit_features(b, on_roll, ib);
        ib = hit_features(b, opponentOf(on_roll), ib);

        assert( (ib - dest) == 156 );
        return ib;
    }

protected:
    static float contact(board const & b, color_t color)
    {
        int me = b.highestChecker(color);
        int him = opponentPoint(b.highestChecker(opponentOf(color)));
        int contact_ = me - him;
        return  contact_ < 0.0f ? 0.0f : (contact_ / 23.0f);
    }

    static float pip_difference(board const& b, color_t color)
    {
        int pdiff =  b.pipCount(opponentOf(color)) - b.pipCount(color);
        return logistic::fwd(pdiff / 27.0f);
    }

    static float hits(board const& b, color_t color)
    {
        return num_hits(color, b) / 36.0f;
    }

private:
    static float *contact_features( board const& b, color_t color, float *ib)
    {
        float f = contact(b, color);

        *ib++ = f;
        *ib++ = (1.0f - f);
        return ib;
    }

    static float *pip_features(board const& b, color_t color, float *ib)
    {
        float h = pip_difference(b, color);

        *ib++ = h;
        *ib++ = 1.0f - h;
        return ib;
    }

    static float *hit_features(board const& b, color_t color, float *ib)
    {
        float h = hits(b, color);

        *ib++ = h;
        *ib++ = 1.0f - h;
        return ib;
    }

protected:
    static float *board_features(board const& b, color_t color, float *ib)
    {
        *ib++ = b.checkersOnPoint(color, 0);    /* borne off */

        for (int i = 1; i <= 24; i++)
        {
            int n = b.checkersOnPoint(color, i);
            *ib++ = (n == 1);                   /* blot     */
            *ib++ = (n >= 2);                   /* point    */
            *ib++ = ( (n > 2) ? (n - 2) : 0 );  /* builders */
        }

        *ib++ = b.checkersOnBar(color);         /* on bar   */
        return ib;
    }
};

class features_v5 : protected features_v3
{
public:
    constexpr static int count = 152;
    features_v5() = delete;

    static float* calc(board const& b, float *dest)
    {
        float *ib = dest;
        const color_t on_roll = b.onRoll();

        ib = board_features(b, on_roll, ib);
        ib = board_features(b, opponentOf(on_roll), ib);

        *ib++ = contact(b, on_roll);
        *ib++ = pip_difference(b, on_roll);

        *ib++ = hits(b, on_roll);
        *ib++ = hits(b, opponentOf(on_roll));

        assert( (ib - dest) == count );
        return ib;
    }
};

class features_v5b : protected features_v5
{
public:
    constexpr static int count = 153;
    features_v5b() = delete;

    static float* calc(board const& b, float *dest)
    {
        float *ib = features_v5::calc(b, dest);
        *ib++ = 1.0f;  // bias

        assert( (ib - dest) == count );
        return ib;
    }
};
