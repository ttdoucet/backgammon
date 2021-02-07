/* Internal data shared between the bearoff database generator and the bearoff library.
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

int board_to_index(uint32_t b);
double fewerEq(uint32_t b, int n);
uint32_t board_to_32(const board &b, color_t c);

