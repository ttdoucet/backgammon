#pragma once

#include "board.h"
#include "features.h"
#include "mathfuncs.h"

// Does not belong here.
#include <fstream>

static void write_float(std::ostream& fs, float f)
{
    fs.write( reinterpret_cast<char *>(&f), sizeof(f) );
}


template<int N_INPUTS, int N_HIDDEN>
class net
{
public:
    constexpr static int n_inputs = N_INPUTS;
    constexpr static int n_hidden = N_HIDDEN;

    typedef float input_vector[N_INPUTS];
    typedef float hidden_vector[N_HIDDEN];

// I think this should be a non-member function
// which takes a BgNet in particular to write.
    void writeFile(const char *fn)
    {
        using namespace std;
        ofstream ofs{fn};
        if (!ofs)
            throw runtime_error(string("Cannot open file stream ") + fn + " for writing.");

        ofs << "portable format: " << 0 << "\n"; // legacy
        ofs << "net type: " << 3 << "\n";        // legacy
        ofs << "hidden nodes: " << n_hidden << "\n";
        ofs << "input nodes: " << n_inputs << "\n";

        for (int i = 0; i < n_hidden; i++)
            for (int j = 0; j < n_inputs; j++)
                write_float(ofs, M(i, j));

        for (int i = 0; i < n_hidden; i++)
            write_float(ofs, V(i));

        ofs << "Current seed: " << seed << "L\n";            // legacy
        ofs << "Games trained: " << games_trained << "L\n";  // legacy
    }

    /* Access to model parameters.
     */
    float& M(int r, int c)
    {
        return weights_1[r][c];
    }

    float& V(int i)
    {
        return weights_2[i];
    }

protected:
    constexpr static float MAX_EQUITY = 3.0f;

    constexpr static float net_to_equity(float p)
    {
        return  (2 * p - 1) * MAX_EQUITY;
    }

    /* Have the network evaluate its input.
     */
    virtual float feedForward()
    {
        for (int i = 0; i < N_HIDDEN; i++)
            pre_hidden[i] = dotprod<N_INPUTS>(input, weights_1[i]);

        for (int i = 0; i < N_HIDDEN; i++)
            hidden[i] = squash_sse(pre_hidden[i]);

        auto output = squash_sse(dotprod<N_HIDDEN>(hidden, weights_2));
        return net_to_equity(output);
    }

    /* Activations.
     */
    alignas(16) input_vector input;
    alignas(16) hidden_vector pre_hidden;
    alignas(16) hidden_vector hidden;

    /* Model parameters.
     */
    alignas(16) float weights_1[N_HIDDEN][N_INPUTS];
    alignas(16) float weights_2[N_HIDDEN];

// backgammon stuff should go elsewhere
public:
    unsigned long seed = 0;  // legacy
    long games_trained = 0;  // legacy

    /* Neural net estimate of the equity for the side on roll. */
    float equity(const board &b) noexcept
    {
        features_v3(b, input);
        return feedForward();
    }
};

using netv3 = net<156, 30>;

class netv3_marginal : public netv3
{
    alignas(16) input_vector prev_input;

public:
    float feedForward() override
    {
        static int count = 0;
        if (count-- == 0)
        {
            for (int i = 0; i < n_inputs; i++)
                prev_input[i] = 0;
            for (int i = 0; i < n_hidden; i++)
                pre_hidden[i] = 0;
            count = 500;
        }

        for (int i = 0; i < n_inputs ; i++)
        {
            if (prev_input[i] == input[i])
                continue;

            float d = input[i] - prev_input[i] ;
            prev_input[i] = input[i];

            for (int j = 0; j < n_hidden; j++)
                pre_hidden[j] += d * M(j, i);

        }
        for (int j = 0; j < n_hidden; j++)
            hidden[j] = squash_sse(pre_hidden[j]);

        float f = dotprod<n_hidden>(hidden, weights_2);
        return net_to_equity( squash_sse(f) );
    }
};

//using BgNet = netv3;
using BgNet = netv3_marginal;

BgNet *readFile(const char *fn);
