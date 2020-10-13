/* Written by Todd Doucet.  See file copyright.txt.
 */
#pragma once

#include "board.h"
#include "mathfuncs.h"
#include "matrix.h"
#include "random.h"

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

    void update_model(const Parameters& adj)
    {
#if 0
        auto M_mag = adj.M.magnitude();

        if (std::isfinite(M_mag) == false)
        {
            std::cout << "adj.M: " << adj.M << "\n";
            std::cout << "adj.V: " << adj.V << "\n";
        }

        assert( adj.M.isfinite() );
        assert( adj.V.isfinite() );
#endif

        params += adj;

#if 0
        assert( params.M.isfinite() );
        assert( params.V.isfinite() );
#endif
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
class BackgammonNet : public net<feature_calc::count, N_HIDDEN>
{
public:
    /* Neural net estimate of the equity for the side on roll.
     */
    float equity(const board &b) noexcept
    {
        feature_calc{b}.calc(this->input.Data());
        return this->feedForward();
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

bool readFile(BgNet& n, std::string fn);
bool writeFile(const BgNet& n, std::string fn);
