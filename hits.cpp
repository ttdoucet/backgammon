/*
 * Copyright (C) 1993 by Todd Doucet.  All Rights Reserved.
 */
#include "board.h"
#include "hits.h"

class HitProblem
{
public:
    HitProblem(const board &b): bd(b) { }
    int num_hits(color_t color);
    struct dice { int low, hi; };

private:
    const board &bd;
    char rhits[7][7] = {0};
    char nhits = 0;

    int blocked(color_t color, int attacker, int low, int hi) const;
    int hops_blocked(color_t color, int attacker, int n, int r) const;
    int can_hit(color_t color, int blot, int distance, char low, char hi) const;

    void record_hit(char low, char hi);
    void find_hits(color_t color, int blot, int distance);

    static const int possibleHit[];
    static const dice *hits[];
    static const dice h1[],  h2[],  h3[],  h4[],  h5[],  h6[],
                      h7[],  h8[],  h9[], h10[], h11[], h12[],
                     h15[], h16[], h18[], h20[], h24[],
                   hnone[];
};

/* Color has a checker located at attacker.  Return whether
 * he is blocked from using both dice from that position.
 */
inline int HitProblem::blocked(color_t color, int attacker, int low, int hi)
    const
{
    color_t opponent = opponentOf(color);
    return (bd.checkersOnPoint(opponent, opponentPoint(attacker-low)) >= 2) &&
           (bd.checkersOnPoint(opponent, opponentPoint(attacker-hi)) >= 2);
}

/* Color has a checker at attacker.  Return whether
 * he is blocked from using n rolls of r from
 * that position.
 */
inline int HitProblem::hops_blocked(color_t color, int attacker, int n, int r)
    const
{
    color_t oc = opponentOf(color);
    for (; n; n--)
        if (bd.checkersOnPoint(oc, opponentPoint(attacker - n*r)) >= 2)
            return 1;
    return 0;
}

/*  We have a blot and an opposing checker located
 *  distance away.  Return whether that checker can
 *  hit our blot.
 */
inline int HitProblem::can_hit(color_t color, int blot, int distance, char low, char hi)
    const
{
    color_t oc = opponentOf(color);
    int hit_using, other, men_on_bar = bd.checkersOnBar(oc);
    int attacker = blot - distance;

    if (distance == low)
    {
        hit_using = 1;
        other = hi;
    }
    else if (distance == hi)
    {
        hit_using = 1;
        other = low;
    }
    else if (distance == (low + hi))
    {
        hit_using = 2;
    }
    else
        hit_using = distance / low;     // doubles

    switch(hit_using)
    {
    case 1:
        if (men_on_bar == 0) return 1;
        if (attacker == 0) return 1;
        if (bd.checkersOnPoint(color, other) >= 2) return 0;
        if (men_on_bar == 1) return 1;
        if (low != hi) return 0;
        if (men_on_bar >= 4) return 0;
        return 1;

    case 2:
        if (low != hi)
        {
            if (men_on_bar > 1) return 0;
            if (men_on_bar == 1 && attacker != 0)
                return 0;
            return !blocked(oc, opponentPoint(attacker),
                            low, hi);
        }
        // fall through on doubles
    case 3:
    case 4:  
        if (men_on_bar)
        {
            if (bd.checkersOnPoint(color, low) >= 2)
                return 0;
            if ((men_on_bar + hit_using - (attacker == 0)) > 4)
                return 0;
        }
        return !hops_blocked(oc, opponentPoint(attacker),
                             hit_using - 1, low);
    }
    return 0;
}

using dice = HitProblem::dice;

const dice HitProblem::h1[] =  { {1,1}, {1,2}, {1,3}, {1,4}, {1,5}, {1,6}, {0,0} };
const dice HitProblem::h2[] =  { {1,2}, {2,2}, {2,3}, {2,4}, {2,5}, {2,6}, {1,1}, {0,0} };
const dice HitProblem::h3[] =  { {1,3}, {2,3}, {3,3}, {3,4}, {3,5}, {3,6}, {1,2}, {1,1}, {0,0}};
const dice HitProblem::h4[] =  { {1,4}, {2,4}, {3,4}, {4,4}, {4,5}, {4,6}, {1,3}, {2,2},
                                 {1,1}, {0,0} };
const dice HitProblem::h5[] =  { {1,5}, {2,5}, {3,5}, {4,5}, {5,5}, {5,6}, {1,4}, {2,3}, {0,0}};
const dice HitProblem::h6[] =  { {1,6}, {2,6}, {3,6}, {4,6}, {5,6}, {6,6}, {1,5}, {2,4},
                                 {3,3}, {2,2}, {0,0} };
const dice HitProblem::h7[] =  { {1,6}, {2,5}, {3,4}, {0,0} };
const dice HitProblem::h8[] =  { {2,6}, {3,5}, {4,4}, {2,2}, {0,0} };
const dice HitProblem::h9[] =  { {3,6}, {4,5}, {3,3}, {0,0} };
const dice HitProblem::h10[] = { {4,6}, {5,5}, {0,0} };
const dice HitProblem::h11[] = { {5,6}, {0,0} };
const dice HitProblem::h12[] = { {6,6}, {3,3}, {4,4}, {0,0} };
const dice HitProblem::h15[] = { {5,5}, {0,0} };
const dice HitProblem::h16[] = { {4,4}, {0,0} };
const dice HitProblem::h18[] = { {6,6}, {0,0} };
const dice HitProblem::h20[] = { {5,5}, {0,0} };
const dice HitProblem::h24[] = { {6,6}, {0,0} };
const dice HitProblem::hnone[] = { {0,0} };

const dice *(HitProblem::hits[]) =
{
    hnone, h1, h2, h3, h4, h5, h6, h7, h8, h9, h10, h11, h12,
    hnone, hnone, h15, h16, hnone, h18, hnone, h20, hnone,
    hnone, hnone, h24
};

const int HitProblem::possibleHit[ ] =
{
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
    0, 0, 15, 16, 0, 18, 0, 20, 0, 0, 0, 24
};

inline void HitProblem::record_hit(char low, char hi)
{
    if (rhits[low][hi]++ == 0)
        nhits += (1 + (low != hi)) ;
}

/*  We have a blot and an opponent's checker located distance away.
 *  Find all the numbers that hit.
 */
inline void HitProblem::find_hits(color_t color, int blot, int distance)
{
    const dice *h;
    for (h = hits[distance]; h->low; h++)
    {
        const int& low = h->low;
        const int& hi = h->hi;
        if (!rhits[low][hi] && can_hit(color, blot, distance, low, hi))
            record_hit(low, hi);
    }
}

/* Compute how many numbers hit color, given that
 * color's opponent rolls next.
 */
inline int HitProblem::num_hits(color_t color)
{
    for (int i = 24; i; i--)
    {
        if (bd.checkersOnPoint(color, i) != 1)
            continue;
        // we have a blot on i
        const color_t opponent = opponentOf(color);
        const int opp_point = opponentPoint(i);
        for (int dist = 1; dist <= i; dist++)
        {
            if (possibleHit[dist] && bd.checkersOnPoint(opponent, opp_point + dist))
                find_hits(color, i, dist);
        }
    }
    return nhits;
}

int num_hits(color_t color, const board &b)
{
    HitProblem hp(b);
    return hp.num_hits(color);
}
