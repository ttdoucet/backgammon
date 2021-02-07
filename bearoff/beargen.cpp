/* Written by Todd Doucet.  See file copyright.txt.
 */

#include <sstream>
#include <string>
#include <iostream>
#include <iomanip>
#include <cstdint>

#include "move.h"
#include "bearoff.h"

struct bear_off bear_off[bsize];

int p_output;
int perm_data[10];

// shared between this generator and the main code
extern int board_to_index(uint32_t b);
extern double fewerEq(uint32_t b, int n);
extern uint32_t board_to_32(const board &b, color_t c);

std::string print_bearoffs()
{
    using std::hex, std::setfill, std::setw, std::fixed;

    std::ostringstream s;
    s << "struct bear_off bear_off[" << p_output << "] = {\n";

    s.precision(6);
    s << fixed << setfill('0');

    for (int i = 0; i < p_output; i++)
    {
        s << "    { 0x" << setw(8) << hex << bear_off[i].board
          << ",\t" << bear_off[i].expectation << "f,\t{" ;

        for (int j = 0; j < 15; j++)
            s << bear_off[i].f[j] << "f, ";
        s << "} },\n";
    }
    s << "};\n";

    return s.str();
}

int extract_point(uint32_t b, int point)
{
    int shift = (4 * point);
    uint32_t mask = (0x0fL << shift);
    return (b & mask) >> shift ;
}

void board_from_32(board &board, uint32_t b, color_t color)
{
    board.clearBoard();
    for (int i = 0; i <= 6; i++)
    {
        int num = extract_point(b, i);
        for (int j = 0; j < num; j++)
            board.moveChecker(color, 0, i);
    }
}

void perm_output()
{
    if ( (p_output % 1000) == 0 )
        std::cerr << "p_output=" << p_output << "\n";

    board b;
    b.clearBoard();
    for (int i = 0; i <= 6; i++)
        for (int j = 0; j < perm_data[i]; j++)
            b.moveChecker(white, 0, i);

    int l = board_to_32(b, white);
    bear_off[p_output].board = board_to_32(b, white);
    p_output++;
}

// Distribute "n" checkers from "hi" to 0.
void perm(int hi, int n)
{
    if (hi == 0)
    {
        perm_data[0] = n;
        perm_output();
        return;
    }
    for (int i = 0; i <= n; i++)
    {
        perm_data[hi] = i;
        perm(hi - 1, n - i);
    }
}

/* Returns the expected number of rolls to
 * clear the position.
 */
float bearoffExpectation(uint32_t b)
{
    return bear_off[board_to_index(b)].expectation;
}

class bearoffCallBack  : public callBack
{
public:
    float best;
    uint32_t bestBoard;

    int callBackF(const board &b) override
    {
        int i = board_to_32(b, white);
        float f = bearoffExpectation(i);
        if (f < best)
        {
            best = f;
            bestBoard = i;
        }
        return 0;
    }

    bearoffCallBack()
    {
        best = 100.0F;
    }
};

// Given the current board and the roll i-j, return the
// expected number of moves to bearoff of the best legal move.
float best_expectation_for(board b, int i, int j)
{
    bearoffCallBack cb;
    b.setRoller(white);
    b.setDice(i, j);
    plays(b, cb);
    return cb.best;
}

// Given the current board and the roll i-j, return the best
// board position resulting from a legal play.
uint32_t best_board_for(uint32_t theBoard, int i, int j)
{
    board b;
    board_from_32(b, theBoard, white);
    bearoffCallBack cb;
    b.setRoller(white);
    b.setDice(i, j);
    plays(b, cb);
    return cb.bestBoard;
}

// Assumes that calc_fewer(i), i < n has been called.
void calc_fewer(int n)
{
    std::cerr << "Calculating fewer(" << n << ")\n";

    for (int k = 0; k < p_output; k++)
    {
        if ( (k % 1000) == 0)
            std::cerr << "fewer(" << n << "): " << k << "\n";

        uint32_t from = bear_off[k].board;
        double f = 0.0;
        for (int i = 1; i <= 6; i++)
        {
            for (int j = 1; j < i; j++)
                f += 2 * fewerEq(best_board_for(from, i, j), n - 1);
            f += fewerEq(best_board_for(from, i, i), n - 1);
        }
        f /= 36.0;
        int x = board_to_index(from);
        bear_off[x].f[n-1] = f;
    }
}

double compute_expectation(uint32_t bd)
{
    board b;
    board_from_32(b, bd, white);

    if (b.checkersOnPoint(white, 0) == 15)
        return 0.0F;

    double e = 0.0;
    float ff;
    for (int i = 1; i <= 6; i++)
    {
        for (int j = 1; j < i; j++)
            e += 2 * (ff = best_expectation_for(b, i, j));
        e += (ff = best_expectation_for(b, i, i));
    }
    double r = (e / 36.0) + 1.0;
    return r;
}

void calc_expectation()
{
    for (int i = 0; i < p_output; i++)
    {
        if ( (i % 1000) == 0)
            std::cerr << "At " << i << ".\n";

        uint32_t b = bear_off[i].board;
        bear_off[i].expectation = compute_expectation(b);
    }
}

int main()
{
    std::ios_base::sync_with_stdio(false);

    p_output = 0;
    perm(6, 15);
    std::cerr << "p_output = " << p_output << "\n";

    calc_expectation();

    calc_fewer(1);
    calc_fewer(2);
    calc_fewer(3);
    calc_fewer(4);
    calc_fewer(5);
    calc_fewer(6);

    calc_fewer(7);
    calc_fewer(8);
    calc_fewer(9);
    calc_fewer(10);
    calc_fewer(11);
    calc_fewer(12);
    calc_fewer(13);

    calc_fewer(14);
    calc_fewer(15);

    std::cout << print_bearoffs();
    return 0;
}
