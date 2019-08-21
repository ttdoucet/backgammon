#include <algorithm>
#include <stdio.h>

#include "board.h"
#include "move.h"
#include "bearoff.h"

struct bear_off bear_off[54264];
const int bsize = 54264;

int p_output;
int perm_data[10];

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
    if (n == 0){
        return (float) (b == 0x0f);
    } else {
        int i = board_to_index(b);
        return bear_off[i].f[n-1];
    }
}

void print_bearoffs()
{
    printf("struct bear_off bear_off[%d] = {\n", p_output);
    for (int i = 0; i < p_output; i++)
    {
        printf("    { 0x%08x,\t%ff,\t{", bear_off[i].board, bear_off[i].expectation);
        for (int j = 0; j < 15; j++)
            printf("%ff, ", bear_off[i].f[j]);
        printf("} },\n");
    }
    printf("};\n");
}

// Encode the current board as a 32-bit number (current, we
// use 28 bits, although 4 are redundant.
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

int extract_point(unsigned int b, int point)
{
    int shift = (4 * point);
    unsigned int mask = (0x0fL << shift);
    return (b & mask) >> shift ;
}

void board_from_32(board &board, unsigned int b, color_t color)
{
    board.clearBoard();
    for (int i = 0; i <= 6; i++){
        int num = extract_point(b, i);
        for (int j = 0; j < num; j++){
            board.moveChecker(color, 0, i);
        }
    }
}

void perm_output()
{
    if ( (p_output % 1000) == 0 )
        fprintf(stderr, "p_output=%d\n", p_output); fflush(stderr);

    board b;
    b.clearBoard();
    for (int i = 0; i <= 6; i++){
        for (int j = 0; j < perm_data[i]; j++)
            b.moveChecker(white, 0, i);
    }

    int l = board_to_32(b, white);
    bear_off[p_output].board = board_to_32(b, white);
    p_output++;
}

// Distribute "n" checkers from "hi" to 0.
void perm(int hi, int n)
{
    if (hi == 0){
        perm_data[0] = n;
        perm_output();
        return;
    }
    for (int i = 0; i <= n; i++){
        perm_data[hi] = i;
        perm(hi - 1, n - i);
    }
}

/* Returns the expected number of rolls to
 * clear the position.
 */
float bearoffExpectation(unsigned int b)
{
    return bear_off[board_to_index(b)].expectation;
}

class bearoffCallBack  : public callBack
{
public:
    float best;
    unsigned int bestBoard;

    int callBackF(const board &b){
        int i = board_to_32(b, white);
        float f = bearoffExpectation(i);
        if (f < best){
            best = f;
            bestBoard = i;
        }
        return 0;
    }
    bearoffCallBack(){
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
unsigned int best_board_for(unsigned int theBoard, int i, int j)
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
    fprintf(stderr, "Calculating fewer(%d)\n", n); fflush(stderr);

    for (int k = 0; k < p_output; k++){
        if ( (k % 1000) == 0){
            fprintf(stderr, "fewer(%d): %d\n", n, k); fflush(stderr);
        }
        unsigned int from = bear_off[k].board;
        double f = 0.0;
        for (int i = 1; i <= 6; i++){
            for (int j = 1; j < i; j++){
                f += 2 * fewerEq(best_board_for(from, i, j), n - 1);
            }
            f += fewerEq(best_board_for(from, i, i), n - 1);
        }
        f /= 36.0;
        int x = board_to_index(from);
        bear_off[x].f[n-1] = (float) f;
    }
}

double compute_expectation(unsigned int bd)
{
    board b;
    board_from_32(b, bd, white);

    if (b.checkersOnPoint(white, 0) == 15)
        return 0.0F;

    double e = 0.0;
    float ff;
    for (int i = 1; i <= 6; i++){
        for (int j = 1; j < i; j++){
            e += 2 * (ff = best_expectation_for(b, i, j));
        }
        e += (ff = best_expectation_for(b, i, i));
    }
    double r = (e / 36.0) + 1.0;
    return r;
}

void calc_expectation()
{
    for (int i = 0; i < p_output; i++){
        if ( (i % 1000) == 0){
            fprintf(stderr, "At %d.\n", i); fflush(stderr);
        }
        unsigned int b = bear_off[i].board;
        bear_off[i].expectation = (float) compute_expectation(b);
    }
}

int main()
{
    p_output = 0;
    perm(6, 15);
    fprintf(stderr, "p_output = %d\n", p_output); fflush(stderr);

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

    print_bearoffs();
    return 0;
}
