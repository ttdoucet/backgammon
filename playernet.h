/* Written by Todd Doucet.  See file copyright.txt.
 */
#pragma once

#include <memory>
#include <cmath>

#include "game.h"
#include "bgnet.h"
#include "bearoff/bearoff.h"

using std::string;

class NeuralNetPlayer : public Player, public callBack
{
public:
    NeuralNetPlayer(BgNet& nn)
        : neural(nn)
    {
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
        bool okay = neural.writeFile(filename);
        if (okay == false)
            throw std::runtime_error(string("Error writing network: ") + filename + "\n");
    }

protected:
    typedef float (NeuralNetPlayer::* evalFunction)(const board& bd);
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

template<class Estimator>
class TemporalDifference
{
public:
    TemporalDifference(Estimator& neural,
		       float a,
		       float lambda,
		       double decay,
		       int batchsize,
	               float momentum=0
    )
        : neural{neural},
          alpha{a},
          lambda{lambda},
          decay{decay},
          batchsize{batchsize},
	  momentum{momentum}
    {
        alpha /= decay;
        batch_grad.clear();
        seq = 0;
    }

    void start()
    {
        grad_adj.clear();
        grad_sum.clear();
        started = false;
        alpha *= decay;
    }

    void observe(const board& b)
    {
        float equity = neural.equity(b);

        if (started)
        {
            float err = previous - equity;
            backprop(err);
        }
        previous = equity;
        started = true;
    }

    void final(float e)
    {
        if (started)
        {
            backprop(previous - e);

	    if (momentum == 0)
		    batch_grad += grad_adj;
	    else
		    batch_grad = batch_grad * momentum + grad_adj * (1 - momentum);

            if (++seq == batchsize)
            {
                neural.update_model(batch_grad * (-alpha));
                seq = 0;
                batch_grad.clear();
            }
        }
    }

private:
    Estimator& neural;
    float lambda; // Temporal discount.
    float alpha;  // Learning rate.
    double decay; // Rate to decay learning rate.
    int batchsize; // number of games per batch
    float momentum; // exponential decay factor for gradient

    typename Estimator::Parameters grad_sum;
    typename Estimator::Parameters grad_adj;
    typename Estimator::Parameters batch_grad;

    float previous;
    bool started;
    int seq = 0;

    void backprop(float err)
    {
        typename Estimator::Parameters grad;
        neural.gradient(grad);

        grad_adj += grad_sum * err;
        grad_sum *= lambda;
        grad_sum += grad;
    }
};

template<typename Estimator>
class Learner : public NeuralNetPlayer
{
    Estimator& mine;
    TemporalDifference<Estimator> our_side;
    TemporalDifference<Estimator> opp_side;
    bool dual;

public:
    Learner(Estimator& estimator,
	    float alpha,
	    float lambda,
	    bool dual,
	    double decay,
	    int batchsize,
	    float momentum
     )
        : NeuralNetPlayer(estimator),
          mine{estimator},
          our_side(mine, alpha, lambda, decay, batchsize, momentum),
          opp_side(mine, alpha, lambda, decay, batchsize, momentum),
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
};
