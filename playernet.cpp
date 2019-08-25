#include <assert.h>
#include "game.h"
#include "net.h"

#include "playernet.h"

NeuralNetPlayer::NeuralNetPlayer(const char *player, const char *netname) : Player(player)
{
    neural = net::read_network(netname);
}

void NeuralNetPlayer::prepareToPlay()
{
}

int NeuralNetPlayer::callBackF(const board &b)
{
    float e = - (this->*equityEstimator)(b);
    if (e > bestEquity)
    {
        bestEquity = e;
        bestMove = m;   // the current move.
    }
    return 0;
}

// The issue is that play() modifies the board, so is not const, but
// it puts it back after it is through.  I don't know how to make that
// work well with const in C++.  The safe thing to is to make a copy.
// Casting away const is not safe, but also works in this case.
// The copy does not seem to impact performance measurably, at least
// for the current 1-ply searches.
void NeuralNetPlayer::selectMove(const board &b, move &mv, evalFunction func)
{
    equityEstimator = func;
    bestEquity = -10.0f;
    bestMove.clear();

    plays(b, *this);

    mv = bestMove;
}

void NeuralNetPlayer::chooseMove(const board& b, move& choice)
{
    if (isBearingOff(b))
        selectMove(b, choice, &NeuralNetPlayer::bearoffEquity);
    else
        selectMove(b, choice, &NeuralNetPlayer::littleE);
}

// Q: Why do we insist that checkers on both sides have
// actually  been taken off before we return true?
// Is this required by bearoffEquity()?
//
bool NeuralNetPlayer::isBearingOff(const board &bd)
{
    return  bd.checkersOnPoint(white, 0) &&
        bd.checkersOnPoint(black, 0) &&
        (bd.highestChecker(white) <= 6) &&
        (bd.highestChecker(black) <= 6);
}

//
// DUPLICATED: make it not so.
//
// Encode the bearoff board as a 32-bit number (currently,
// we use 28 bits, although 4 are redundant.
unsigned long NeuralNetPlayer::board_to_32(const board &b, color_t c)
{
    unsigned long d6 = b.checkersOnPoint(c, 6) << (4 * 6);
    unsigned long d5 = b.checkersOnPoint(c, 5) << (4 * 5);
    unsigned long d4 = b.checkersOnPoint(c, 4) << (4 * 4);
    unsigned long d3 = b.checkersOnPoint(c, 3) << (4 * 3);
    unsigned long d2 = b.checkersOnPoint(c, 2) << (4 * 2);
    unsigned long d1 = b.checkersOnPoint(c, 1) << (4 * 1);
    unsigned long d0 = b.checkersOnPoint(c, 0);
    return d0 | d1 | d2 | d3 | d4 | d5 | d6;
}

float NeuralNetPlayer::bearoffEquity(const board &b)
{
    // in beardll.dll
    double  bearoffEquity(unsigned long, unsigned long);

    assert(isBearingOff(b));
    unsigned long onRoll = board_to_32(b, b.onRoll());
    unsigned long notOnRoll = board_to_32(b, b.notOnRoll());
    return (float) bearoffEquity(onRoll, notOnRoll);
}

float NeuralNetPlayer::littleE(const board &bd)
{
    assert(bd.diceInCup());
    assert(neural != 0);

    int e = gameOver(bd);
    if (e)
        return (double) e;
    return  neural->equity(bd);
}

// winner won the game.  Return the equity counting gammons & backgammons.
int NeuralNetPlayer::gammon_check(const board &nb, color_t winner)
{
    color_t loser = opponentOf(winner);
    int equity = 1;
    if (nb.checkersOnPoint(loser, 0) == 0)
    {
        equity++;       // gammon
        if (nb.highestChecker(loser) > 18)
            equity++;       // backgammon
    }
    return equity;
}

int NeuralNetPlayer::win_check(const board &nb, color_t side)
{
    if (nb.checkersOnPoint(side, 0) == 15)
        return gammon_check(nb, side);
    else
        return 0;
}

/* Returns nonzero if the game is over, in which case the
 * returned value is the equity of the player on roll.
 */
int NeuralNetPlayer::gameOver(const board &bd)
{
    int e;
    if ( (e = win_check(bd, bd.onRoll())) != 0)
        return e;
    else if ( (e = win_check(bd, bd.notOnRoll())) != 0)
        return -e;
    else
        return 0;
}

