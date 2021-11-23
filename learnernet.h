/* Written by Todd Doucet. */
#pragma once

#include "playernet.h"

template<class Estimator>
class TemporalDifference
{
public:
    TemporalDifference(Estimator& neural,
		       float a,
		       float lambda,
		       double decay,
		       int batchsize,
	               float momentum
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
		    batch_grad += batch_grad * momentum + grad_adj * (1 - momentum);

            if (++seq == batchsize)
            {
                neural.params += (batch_grad * (-alpha));

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
class Learner : public PlayerNet
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
        : PlayerNet{estimator},
          mine{estimator},
          our_side{mine, alpha, lambda, decay, batchsize, momentum},
          opp_side{mine, alpha, lambda, decay, batchsize, momentum},
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

