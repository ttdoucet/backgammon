#pragma once

#include "game.h"
#include "net.h"
#include "bearoff.h"

class NeuralNetPlayer : public Player, public callBack
{
public:
    NeuralNetPlayer(const char *player, const char *netname)
        : Player(player),
          neural(*net::readFile(netname))
    {
    }

    void chooseMove(const board& b, moves& choice)
    {
        if (isBearingOff(b))
            selectMove(b, choice, &NeuralNetPlayer::bearoffEquity);
        else
            selectMove(b, choice, &NeuralNetPlayer::littleE);
    }

protected:
    typedef float (NeuralNetPlayer::* evalFunction)(const board& bd);
    evalFunction equityEstimator;

    net& neural;
    float bestEquity;
    moves bestMove;

    float bearoffEquity(const board &b)
    {
        return ::bearoffEquity(b);
    }

    int callBackF(const board &b)
    {
        float e = - (this->*equityEstimator)(b);
        if (e > bestEquity)
        {
            bestEquity = e;
            bestMove = m;   // the current move.
        }
        return 0;
    }

    void selectMove(const board &b, moves &mv, evalFunction func)
    {
        equityEstimator = func;
        bestEquity = -10.0f;
        bestMove.clear();

        plays(b, *this);

        mv = bestMove;
    }

    float littleE(const board &bd)
    {
    //  assert(bd.diceInCup());
        if (gameOver(bd))
            return score(bd, bd.onRoll());

        return  neural.equity(bd);
    }

    static bool isBearingOff(const board &bd)
    {
        return  bd.checkersOnPoint(white, 0) &&
            bd.checkersOnPoint(black, 0) &&
            (bd.highestChecker(white) <= 6) &&
            (bd.highestChecker(black) <= 6);
    }
};

