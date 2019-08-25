/*
 * Copyright (C) 1993 by Todd Doucet.  All Rights Reserved.
 */

#include <stdexcept>

#include "board.h"
#include "hits.h"

#include "stopwatch.h"
stopwatch htimer;

struct dice {
    char low, hi;
};

class hitProblem
{
public:
    int num_hits(color_t color);
    hitProblem(const board &b): bd(b)
    {
        clear_hits();
    }

private:
    const board &bd;
    char rhits[7][7];
    char nhits;

    int blocked(color_t color, int attacker, int low, int hi);
    int hops_blocked(color_t color, int attacker, int n, int r);
    int can_hit(color_t color, int blot, int distance, char low, char hi);
    void clear_hits();
    void record_hit(char low, char hi);
    void find_hits(color_t color, int blot, int distance);

    static int possibleHit[ ];
    static dice *(hits[ ]);
    static dice h1[ ], h2[ ], h3[ ],  h4[ ],  h5[ ],  h6[ ],
                h7[ ], h8[ ], h9[ ], h10[ ], h11[ ], h12[ ],
                h15[ ], h16[ ], h18[ ], h20[ ], h24[ ],
                hnone[ ];
};

/* Color has a checker located at attacker.  Return whether
 * he is blocked from using both dice from that position.
 */
inline int hitProblem::blocked(color_t color, int attacker, int low, int hi)
{
    color_t opponent = opponentOf(color);
    return (bd.checkersOnPoint(opponent, opponentPoint(attacker-low)) >= 2) &&
           (bd.checkersOnPoint(opponent, opponentPoint(attacker-hi)) >= 2);
}

/* Color has a checker at attacker.  Return whether
 * he is blocked from using n rolls of r from
 * that position.
 */
inline int hitProblem::hops_blocked(color_t color, int attacker, int n, int r)
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
inline int hitProblem::can_hit(color_t color, int blot, int distance, char low, char hi)
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

    default:
        throw std::runtime_error("Error in can_hit()");
    }
    return 0;
}

dice hitProblem::h1[] =  { {1,1}, {1,2}, {1,3}, {1,4}, {1,5}, {1,6}, {0,0} };
dice hitProblem::h2[] =  { {1,2}, {2,2}, {2,3}, {2,4}, {2,5}, {2,6}, {1,1}, {0,0} };
dice hitProblem::h3[] =  { {1,3}, {2,3}, {3,3}, {3,4}, {3,5}, {3,6}, {1,2}, {1,1}, {0,0}};
dice hitProblem::h4[] =  { {1,4}, {2,4}, {3,4}, {4,4}, {4,5}, {4,6}, {1,3}, {2,2},
                           {1,1}, {0,0} };
dice hitProblem::h5[] =  { {1,5}, {2,5}, {3,5}, {4,5}, {5,5}, {5,6}, {1,4}, {2,3}, {0,0}};
dice hitProblem::h6[] =  { {1,6}, {2,6}, {3,6}, {4,6}, {5,6}, {6,6}, {1,5}, {2,4},
                           {3,3}, {2,2}, {0,0} };
dice hitProblem::h7[] =  { {1,6}, {2,5}, {3,4}, {0,0} };
dice hitProblem::h8[] =  { {2,6}, {3,5}, {4,4}, {2,2}, {0,0} };
dice hitProblem::h9[] =  { {3,6}, {4,5}, {3,3}, {0,0} };
dice hitProblem::h10[] = { {4,6}, {5,5}, {0,0} };
dice hitProblem::h11[] = { {5,6}, {0,0} };
dice hitProblem::h12[] = { {6,6}, {3,3}, {4,4}, {0,0} };
dice hitProblem::h15[] = { {5,5}, {0,0} };
dice hitProblem::h16[] = { {4,4}, {0,0} };
dice hitProblem::h18[] = { {6,6}, {0,0} };
dice hitProblem::h20[] = { {5,5}, {0,0} };
dice hitProblem::h24[] = { {6,6}, {0,0} };
dice hitProblem::hnone[] = { {0,0} };

dice *(hitProblem::hits[]) =
{
    hnone, h1, h2, h3, h4, h5, h6, h7, h8, h9, h10, h11, h12,
    hnone, hnone, h15, h16, hnone, h18, hnone, h20, hnone,
    hnone, hnone, h24
};

int hitProblem::possibleHit[ ] =
{
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
    0, 0, 15, 16, 0, 18, 0, 20, 0, 0, 0, 24
};

inline void hitProblem::clear_hits()
{
    for (int i = 0; i < 7; i++)
        for (int j = 0; j < 7; j++)
            rhits[i][j] = 0;
    nhits = 0;
}

inline void hitProblem::record_hit(char low, char hi)
{
    if (rhits[low][hi]++ == 0)
        nhits += (1 + (low != hi)) ;
}


/*  We have a blot and an opponent's checker located distance away.
 *  Find all the numbers that hit.
 */
inline void hitProblem::find_hits(color_t color, int blot, int distance)
{
    dice *h;
    for (h = hits[distance]; h->low; h++)
    {
        if (!rhits[h->low][h->hi] && can_hit(color, blot, distance, h->low, h->hi))
            record_hit(h->low, h->hi);
    }
}


/* Compute how many numbers hit color, given that
 * color's opponent rolls next.
 */
inline int hitProblem::num_hits(color_t color)
{
    int i, j;

    for (i = 24; i; i--)
    {
        if (bd.checkersOnPoint(color, i) != 1)
            continue;
        // we have a blot on i
        for (j = 1; j <= i; j++)
        {
            if (possibleHit[j] && bd.checkersOnPoint(opponentOf(color), opponentPoint(i-j)))
                find_hits(color, i, j);
        }
    }
    return nhits;
}

int num_hits(color_t color, const board &b)
{
    htimer.start();

    hitProblem hp(b);

//  htimer.start();

    auto v = hp.num_hits(color);

    htimer.stop();

    return v;
}
