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
    NeuralNetPlayer(string netname)
    {
        if (netname != "random")
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

#include <iostream>

class Learner : public NeuralNetPlayer
{
public:
    Learner(string netname, float alpha_, float lambda_)
        : NeuralNetPlayer(netname)
    {
        alpha = alpha_;
        lambda = lambda_;

        std::cout << "Learner: alpha = " << this->alpha << "\n";
        std::cout << "Learner: lambda = " << this->lambda << "\n";
    }

    void prepareToPlay() override
    {
        grad_adj.clear();
        grad_sum.clear();
        started = false;
    }

    void presentBoard(const board& b, bool me_on_roll) override
    {
        assert( !b.d1() && !b.d2() );

        if (me_on_roll == false)
            return;

        float equity = neural.equity(b);

        if (started)
        {
            float err = previous - equity;
            reconsider(err);
        }
        previous = equity;
        started = true;
    }

    void finalEquity(float e) override
    {
        if (started)
        {
            reconsider(previous - e);
            neural.update_model( grad_adj * (-alpha) );
        }
    }

private:
    float lambda; // Temporal discount.
    float alpha;  // Learning rate.

    /* Sum of gradients with lambda temporal discount.
     */
    BgNet::Parameters grad_sum;

    /* Accumulated adjustments to weights.
     */
    BgNet::Parameters grad_adj;

    void reconsider(float err)
    {
        BgNet::Parameters grad;
        neural.backprop(grad);

        grad_adj += grad_sum * err;
        grad_sum *= lambda;
        grad_sum += grad;
    }

    float previous;
    bool started;
};
