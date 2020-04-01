/* Written by Todd Doucet.  See file copyright.txt.
 */
#pragma once

#include <memory>

#include "game.h"
#include "net.h"
#include "bearoff.h"

class NeuralNetPlayer : public Player, public callBack
{
public:
    NeuralNetPlayer(const char *player, std::string netname)
        : Player(player),
          neural( std::make_unique<BgNet>(*readFile(netname)) )
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

    std::unique_ptr<BgNet> neural;
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

        return neural->equity(bd);
    }

    static bool isBearingOff(const board &bd)
    {
        return  bd.checkersOnPoint(white, 0) &&
            bd.checkersOnPoint(black, 0) &&
            (bd.highestChecker(white) <= 6) &&
            (bd.highestChecker(black) <= 6);
    }
};

class Learner : public NeuralNetPlayer
{
public:
    Learner(const char *player, std::string netname)
        : NeuralNetPlayer(player, netname)
    {
    }

    void prepareToPlay() override
    {
        neural->clear_gradients();
        started = false;
    }

    void presentBoard(const board& b) override
    {
        float desired = neural->equity(b);
        assert( !b.d1() && !b.d2() );

        if (started)
        {
            float err = previous - desired;
            neural->reconsider(err);
        }
        previous = desired;
        started = true;
    }

    void finalEquity(float e) override
    {
        if (started)
        {
            neural->reconsider(previous - e);
            neural->update_model();
        }
    }

private:
    float previous;
    bool started;
};

