#include <algorithm>
#include <stdlib.h>
#include <cassert>

#include "bearoff.h"

extern struct bear_off bear_off[bsize];

int board_to_index(unsigned int b)
{
    auto comparison = [](struct bear_off &lhs, unsigned int val) { return lhs.board < val; };

    struct bear_off *i = std::lower_bound(bear_off, bear_off + bsize, b, comparison);
    return i - bear_off;
}

/* Return the probability that the bearoff position denoted
   by "b" will be cleared in n or fewer rolls.
*/
double fewerEq(unsigned int b, int n)
{
	if (n == 0)
            return (float) (b == 0x0f);
        else
        {
            int i = board_to_index(b);
            return bear_off[i].f[n-1];
	}
}

/* Return the probability that the bearoff position denoted
   by "b" will be cleared in n or greater rolls.
*/
double greaterEq(unsigned int b, int n)
{
	if (n == 0.0)
		return 1.0;
	return 1.0 - fewerEq(b, n - 1);
}

/* Return the probability that the bearoff position denoted
   by "b" will be cleared in exactly n rolls.
*/
double exact(unsigned int b, int n)
{
	if (n == 0)
            return b == 0x0f;
	return fewerEq(b, n) - fewerEq(b, n - 1);
}

double bearoffEquity(unsigned int onRoll, unsigned int notOnRoll)
{
	double p = 0.0;
	for (int i = 0; i <= 15; i++)
		p += (exact(onRoll, i) * greaterEq(notOnRoll, i));
	return (2.0 * p) - 1.0;
}

unsigned int board_to_32(const board &b, color_t c)
{
    unsigned int d6 = b.checkersOnPoint(c, 6) << (4 * 6);
    unsigned int d5 = b.checkersOnPoint(c, 5) << (4 * 5);
    unsigned int d4 = b.checkersOnPoint(c, 4) << (4 * 4);
    unsigned int d3 = b.checkersOnPoint(c, 3) << (4 * 3);
    unsigned int d2 = b.checkersOnPoint(c, 2) << (4 * 2);
    unsigned int d1 = b.checkersOnPoint(c, 1) << (4 * 1);
    unsigned int d0 = b.checkersOnPoint(c, 0);
    return d0 | d1 | d2 | d3 | d4 | d5 | d6;
}

float bearoffEquity(const board &b)
{
    unsigned int onRoll = board_to_32(b, b.onRoll());
    unsigned int notOnRoll = board_to_32(b, b.notOnRoll());
    return (float) bearoffEquity(onRoll, notOnRoll);
}
