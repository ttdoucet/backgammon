/* Written by Todd Doucet.  See file copyright.txt.
 */
#pragma once

#include <memory>
#include <cmath>

#include "equity.h"

#include "game.h"
#include "net.h"
#include "bearoff.h"

using std::string;

static_assert(EquityEstimator<netv3>);
static_assert(TrainableEquityEstimator<netv3>);


template<EquityEstimator Estimator>
class NeuralNetPlayer : public Player, public callBack
{
public:
    NeuralNetPlayer(string netname)
    {
        bool okay = true;
        if (netname != "random")
            okay = readFile(neural, netname);

        if (okay == false)
            throw std::runtime_error(string("Error reading network: ") + netname + "\n");

    }

    void chooseMove(const board& b, moves& choice) override
    {
        if (isBearingOff(b))
            selectMove(b, choice, &NeuralNetPlayer::bearoffEquity);
        else
            selectMove(b, choice, &NeuralNetPlayer::littleE);
    }

    void save(string filename)
    {
        bool okay = writeFile(neural, filename);
        if (okay == false)
            throw std::runtime_error(string("Error writing network: ") + filename + "\n");
    }

protected:
    typedef float (NeuralNetPlayer::* evalFunction)(const board& bd);
    evalFunction equityEstimator;

    Estimator neural;

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


template<TrainableEquityEstimator Estimator>
class TemporalDifference
{
public:
    TemporalDifference(Estimator& neural, float alpha, float lambda)
        : neural{neural},
          alpha{alpha},
          lambda{lambda}
    {
    }

    void start()
    {
        grad_adj.clear();
        grad_sum.clear();
        started = false;
    }

    void observe(const board& b)
    {
        float equity = neural.equity(b);

        if (started)
        {
            float err = previous - equity;
            reconsider(err);
        }
        previous = equity;
        started = true;
    }

    void final(float e)
    {
        if (started)
        {
            reconsider(previous - e);
            neural.update_model( grad_adj * (-alpha) );
        }
    }

private:
    Estimator& neural;
    float lambda; // Temporal discount.
    float alpha;  // Learning rate.

    typename Estimator::Parameters grad_sum;
    typename Estimator::Parameters grad_adj;

    float previous;
    bool started;

    void reconsider(float err)
    {
        typename Estimator::Parameters grad;
        neural.backprop(grad);

        grad_adj += grad_sum * err;
        grad_sum *= lambda;
        grad_sum += grad;
    }
};

template<TrainableEquityEstimator Estimator>
class Learner : public NeuralNetPlayer<Estimator>
{
public:
    Learner(string netname, float alpha, float lambda, bool dual=false)
        : NeuralNetPlayer<Estimator>(netname),
          our_side(this->neural, alpha, lambda),
          opp_side(this->neural, alpha, lambda),
          dual{dual}
    {
    }

    void prepareToPlay() override
    {
        our_side.start();
        if (dual)
            opp_side.start();
    }

    void presentBoard(const board& b, bool me_on_roll) override
    {
        assert( !b.d1() && !b.d2() );

        if (me_on_roll)
            our_side.observe(b);
        else if (dual)
            opp_side.observe(b);
    }

    void finalEquity(float e) override
    {
        our_side.final(e);
        if (dual)
            opp_side.final(-e);
    }

private:
    bool dual;
    TemporalDifference<Estimator> our_side;
    TemporalDifference<Estimator> opp_side;
};
