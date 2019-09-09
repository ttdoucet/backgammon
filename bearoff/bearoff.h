/* Written by Todd Doucet.  See file copyright.txt.
 */
#include "board.h"

struct bear_off
{
    unsigned int board;
    float expectation;
    float f[15];
};

constexpr int bsize = 54264;
extern bear_off bearoff[bsize];

float bearoffEquity(const board &b);
