/* Written by Todd Doucet. */
#pragma once

#include "playernet.h"

struct LearningOptions
{
    float alpha;
    float lambda;
    bool dual;
    double decay;
    int batchsize;
    float momentum;

    LearningOptions(float alpha, float lambda, bool dual, double decay, int batchsize, float momentum)
        : alpha{alpha / batchsize},
          lambda{lambda}, dual{dual}, decay{decay}, batchsize{batchsize},momentum{momentum}
    {
        assert(alpha > 0);
    }
};

template<class Estimator>
class TemporalDifference
{
public:
    TemporalDifference(Estimator& neural, LearningOptions const& opts)
        : neural{neural},
          alpha{opts.alpha},
          lambda{opts.lambda},
          decay{opts.decay},
          batchsize{opts.batchsize},
          momentum{opts.momentum}
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
class LearnerNet : public PlayerNet
{
    Estimator& mine;
    TemporalDifference<Estimator> our_side;
    TemporalDifference<Estimator> opp_side;
    bool dual;

public:
    LearnerNet(Estimator& estimator, LearningOptions const& opts)
        : PlayerNet{estimator},
          mine{estimator},
          our_side{mine, opts},
          opp_side{mine, opts},
          dual{opts.dual}
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

class LearnerNetFactory
{
    template<typename T>
    static std::unique_ptr<LearnerNet<T>> downcast(BgNet& nn, LearningOptions const& opts)
    {
        if (auto p = dynamic_cast<T*>(&nn))
            return std::make_unique<LearnerNet<T> > (*p, opts);
        else
            return nullptr;
    }

    template<typename ... NetType>
    struct nnlist
    {
        static auto create(BgNet& nn, const LearningOptions& opts)
        {
            std::unique_ptr<PlayerNet> p;
            ( (p = downcast<NetType>(nn, opts)) || ... );
            return p;
        }
    };

public:
    static auto create(BgNet& nn, const LearningOptions& opts)
    {
        using supported = nnlist<netv3, Fc_Sig_H60_I3, Fc_Sig_H90_I3, Fc_Sig_H120_I3,
                                 netv5, Fc_Sig_H60_I5, // Fc_Sig_H90_I5, Fc_Sig_H120_I5,
                                 netv3tr, Fc_Sig_H60_I3tr, Fc_Sig_H90_I3tr, Fc_Sig_H120_I3tr,
                                 Fc_Sig_H240_I3tr,
                                 Fc_Sig_H60_I5B,
                                 netv1, Fc_Sig_H60_I1,
                                 Fc_Sig_H30_I3N, Fc_Sig_H60_I3N, Fc_Sig_H90_I3N, Fc_Sig_H120_I3N
                                 >;

        return supported::create(nn, opts);
    }
};
