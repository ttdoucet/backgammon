/* Written by Todd Doucet. */
#pragma once

#include <memory>
#include <cmath>

#include "game.h"
#include "bgnet.h"
#include "bearoff/bearoff.h"

class PlayerNet : public Player, public callBack
{
public:
    PlayerNet(BgNet& nn)
        : neural{nn}
    {
    }

    void chooseMove(const board& b, moves& choice) override
    {
        if (isBearingOff(b))
            selectMove(b, choice, &PlayerNet::bearoffEquity);
        else
            selectMove(b, choice, &PlayerNet::littleE);
    }

protected:
    typedef float (PlayerNet::* evalFunction)(const board& bd);
    evalFunction equityEstimator;

    BgNet& neural;
    float bestEquity;
    moves bestMove;

    float bearoffEquity(const board &b)
    {
        return ::bearoffEquity(b);
    }

    int callBackF(const board &b) override
    {
        float e = - (this->*equityEstimator)(b);

        assert( std::isnan(e) == false);
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
        if (gameOver(bd))
            return score(bd, bd.onRoll());

        return neural.equity(bd);
    }

    static bool isBearingOff(const board &bd)
    {
        return  bd.checkersOnPoint(white, 0)  &&
                bd.checkersOnPoint(black, 0)  &&
                bd.highestChecker(white) <= 6 &&
                bd.highestChecker(black) <= 6;
    }
};

class PlayerNetFactory
{
public:
    static auto create(BgNet& nn)
    {
        return std::make_unique<PlayerNet>(nn);
    }
};
