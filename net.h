/* Written by Todd Doucet.  See file copyright.txt.
 */
#pragma once

#include "board.h"
#include "mathfuncs.h"
#include "matrix.h"
#include "random.h"

class BgNet
{
public:
    float equity(const board& b);
    bool readFile(std::string filename);
    bool writeFile(std::string filename);
    virtual ~BgNet() = default;
};


template<int N_INPUTS, int N_HIDDEN>
class net
{
public:
    constexpr static int n_inputs = N_INPUTS;
    constexpr static int n_hidden = N_HIDDEN;

    typedef matrix<N_INPUTS, 1> input_vector;
    typedef matrix<N_HIDDEN, 1> hidden_vector;

    typedef matrix<N_HIDDEN, N_INPUTS> W1;
    typedef matrix<1, N_HIDDEN> W2;

protected:
    constexpr static int MAX_EQUITY = 3;

    constexpr static float net_to_equity(float p)
    {
        return  (2*p - 1) * MAX_EQUITY;
    }

    float feedForward()
    {
        hidden = params.M * input;

        for (int i = 0; i < N_HIDDEN; i++)
            hidden(i, 0) = squash(hidden(i, 0));

        float x =  params.V * hidden;
        out = squash(x);

#if 1
        if (std::isfinite(out) == false)
        {
            std::cout << "WARNING: out is not a finite number:";
            if (std::isnan(out))
                std::cout << " nan";
            if (std::isinf(out))
                std::cout << " inf";
            std::cout << ", x = " << x << "\n";

            std::cout << "V: " << params.V << "\n";
            std::cout << "hidden: " << hidden << "\n";
        }
#endif

        return net_to_equity(out);
    }

    /* I think we should gather things into Activations like
     * we did for Parameters, and this would include the input
     * and the output.
     */

    input_vector input;

public:
    /* Model parameters.
     */
    struct Parameters
    {
        W1 M;
        W2 V;

        void clear()
        {
            M.clear();
            V.clear();
        }

        Parameters& operator+=(const Parameters &rhs)
        {
            M += rhs.M;
            V += rhs.V;
            return *this;
        }

        Parameters& operator*=(float scale)
        {
            M *= scale;
            V *= scale;
            return *this;
        }

        Parameters operator*(float scale) const
        {
            Parameters r(*this);
            return r *= scale;
        }
    };
    Parameters params;

    /* Computes dy/dx, where y is the scalar output
     * of the net during the last forward calculation,
     * and x is any learning parameter.
     */
    void backprop(Parameters& grad)
    {
        auto const f = 2 * MAX_EQUITY * out * (1 - out);

        grad.V = f * hidden.Transpose();

        matrix<N_HIDDEN, 1> lhs = f * params.V.Transpose();

        for (int i = 0; i < lhs.Rows(); i++)
            lhs(i, 0) *= ( hidden(i, 0) * (1 - hidden(i, 0)) );

        grad.M = lhs * input.Transpose();
    }

// I think this can move to TemporalDifferenceNet.
    void update_model(const Parameters& adj)
    {
        params += adj;
    }

    net()
    {
        RNG_normal rand1(0, 1.0 / N_INPUTS);
        for (int r = 0; r < N_HIDDEN; r++)
            for (int c = 0; c < N_INPUTS; c++)
                params.M(r, c) = rand1.random();

        RNG_normal rand2(0, 1.0 / N_HIDDEN);
        for (int c = 0; c < N_HIDDEN; c++)
            params.V(0, c) = rand2.random();
    }

protected:
    /* State activations maintained after 
     * feedForward() for backpropagation.
     */
    hidden_vector hidden;
    float out;
};

/* Backgammon-specific class derivations.
 */

#include "features.h"

template<class feature_calc, int N_HIDDEN>
class BackgammonNet :  // public BgNet,
                       public net<feature_calc::count, N_HIDDEN>
{
public:
    /* Neural net estimate of the equity for the side on roll.
     */
    float equity(const board &b)
    {
        feature_calc::calc(b, this->input.Data());
        return this->feedForward();
    }

    BackgammonNet()
    {
        assert( feature_calc::count == this->input.Rows() * this->input.Cols() );
    }
};

class netv3 : public BackgammonNet<features_v3<float*>, 30>
{
public:
    bool readFile(std::string filename);
    bool writeFile(std::string filename) const;
    ~netv3() { }
};


/*************************************/


template<class N>
class TemporalDifferenceNet
{
public:
    using Parameters = typename N::Parameters;

    TemporalDifferenceNet(float alpha, float lambda)
        : alpha{alpha},
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
        float eqty = N::equity(b);

        if (started)
        {
            float err = previous - eqty;
            reconsider(err);
        }
        previous = eqty;
        started = true;
    }

    void final(float e)
    {
        if (started)
        {
            reconsider(previous - e);
            update_model( grad_adj * (-alpha) );
        }
    }

private:
    float lambda; // Temporal discount.
    float alpha;  // Learning rate.

    Parameters grad_sum;
    Parameters grad_adj;

    float previous;
    bool started;

    void reconsider(float err)
    {
        Parameters grad;
        backprop(grad);

        grad_adj += grad_sum * err;
        grad_sum *= lambda;
        grad_sum += grad;
    }
};


typedef TemporalDifferenceNet<netv3> netv3_trainable;
