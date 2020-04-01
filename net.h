/* Written by Todd Doucet.  See file copyright.txt.
 */
#pragma once

#include "board.h"
#include "mathfuncs.h"
#include "matrix.h"

#include <iostream>

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
    float feedForward()
    {
        hidden = M * input;

        for (int i = 0; i < N_HIDDEN; i++)
            hidden(i) = squash(hidden(i));

        return  out = squash( V * hidden );
    }

    void backprop(float err)
    {
        auto const f = out * (1 - out);

        auto V_grad = f * hidden.Transpose();
        V_grads *= lambda;
        V_grads += V_grad;

        auto lhs = f * V.Transpose();
        static_assert(lhs.Rows() == N_HIDDEN);

        for (int i = 0; i < lhs.Rows(); i++)
            lhs(i) *= ( hidden(i) * (1 - hidden(i)) );

        auto M_grad = lhs * input.Transpose();

        M_grads *= lambda;
        M_grads += M_grad;

        M_adj += err * M_grads;
        V_adj += err * V_grads;
    }

    input_vector input;

public:
    /* Model parameters.
     */
    W1 M;
    W2 V;

    float lambda = 0.85f;    // Temporal discount.
    float alpha = 0.001f;    // Learning rate.

    void clear_gradients()
    {
        M_grads.clear();
        V_grads.clear();

        M_adj.clear();
        V_adj.clear();
    }

    void update_model()
    {
//      std::cout << "alpha * V_adj: " << (alpha * V_adj) << "\n";
//      std::cout << "V_adj: " << V_adj << "\n";

        M -= alpha * M_adj;
        V -= alpha * V_adj;
    }

private:
public: // public for debugging
    /* State activations maintained after 
     * feedForward() for backpropagation.
     */
    hidden_vector hidden;
    float out;

    /* Sum of gradients with lambda temporal discount.
     */
    W1 M_grads;
    W2 V_grads;

    /* Accumulated adjustments to weights.
     */
    W1 M_adj;
    W2 V_adj;
};


/* Backgammon-specific class derivations.
 */

#include "features.h"

template<class feature_calc, int N_HIDDEN>
class BackgammonNet : public net<feature_calc::count, N_HIDDEN>
{
protected:
    constexpr static int MAX_EQUITY = 3;

    constexpr static float net_to_equity(float p)
    {
        return  (2*p - 1) * MAX_EQUITY;
    }

    constexpr static float delta_equity_to_delta_net(float ediff)
    {
        return  ediff / (2 * MAX_EQUITY);
    }

public:
    /* Neural net estimate of the equity for the side on roll.
     */
    float equity(const board &b) noexcept
    {
        feature_calc{b}.calc(this->input.Data());
        return net_to_equity( this->feedForward() );
    }

    void reconsider(float err)
    {
        this->backprop( delta_equity_to_delta_net(err) );
    }

    BackgammonNet()
    {
        assert( feature_calc::count == this->input.Rows() * this->input.Cols() );
    }

    uint64_t seed = 0;  // legacy
    int64_t games_trained = 0;  // legacy
};

using netv3 = BackgammonNet<features_v3<float*>, 30>;

using BgNet = netv3;

BgNet *readFile(std::string fn);
void writeFile(BgNet& n, std::string fn);
