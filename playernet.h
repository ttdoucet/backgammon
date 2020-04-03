/* Written by Todd Doucet.  See file copyright.txt.
 */
#pragma once

#include <memory>
#include <cmath>

#include "game.h"
#include "net.h"
#include "bearoff.h"

using std::string;

class NeuralNetPlayer : public Player, public callBack
{
public:
    NeuralNetPlayer(string player, string netname)
        : Player(player)
    {
        readFile(neural, netname);
    }

    void chooseMove(const board& b, moves& choice)
    {
        if (isBearingOff(b))
            selectMove(b, choice, &NeuralNetPlayer::bearoffEquity);
        else
            selectMove(b, choice, &NeuralNetPlayer::littleE);
    }

    void save(string filename)
    {
        writeFile(neural, filename);
    }

protected:
    typedef float (NeuralNetPlayer::* evalFunction)(const board& bd);
    evalFunction equityEstimator;

    BgNet neural;

    float bestEquity;
    moves bestMove;

    float bearoffEquity(const board &b)
    {
        return ::bearoffEquity(b);
    }

    int callBackF(const board &b)
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
    //  assert(bd.diceInCup());
        if (gameOver(bd))
            return score(bd, bd.onRoll());

        return neural.equity(bd);
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
    Learner(string player, string netname, float alpha, float lambda)
        : NeuralNetPlayer(player, netname)
    {
        neural.alpha = alpha;
        neural.lambda = lambda;
    }

    void prepareToPlay() override
    {
        neural.clear_gradients();
        started = false;
    }

    void presentBoard(const board& b) override
    {
        float desired = neural.equity(b);
        assert( !b.d1() && !b.d2() );

        if (started)
        {
            float err = previous - desired;
            neural.reconsider(err);
        }
        previous = desired;
        started = true;
    }

    void finalEquity(float e) override
    {
        if (started)
        {
//          std::cout << "finalEquity: V_adj magnitude: " << neural.V_adj.magnitude() << "\n";
//          std::cout << "finalEquity: M_adj magnitude: " << neural.M_adj.magnitude() << "\n";
            neural.reconsider(previous - e);
            neural.update_model();
        }
    }

private:
    float previous;
    bool started;
};

