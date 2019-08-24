/*
 * Copyright (C) 1993 by Todd Doucet.  All Rights Reserved.
 */

#include <string.h>
#include <sstream>

#include "board.h"
#include "net.h"
#include "hits.h"
#include "random.h"
#include "console.h"

void net::compute_hit_danger_v3(const color_t color, float *ib)
{
    float h = ((float) num_hits(color, netboard)) / 36.0f;

    ib[0] = h;
    ib[1] = 1.0f - h;
}

int net::crossovers(color_t color)
{
    int m = 4 * netboard.checkersOnBar(color);
    for (int j = 1; j <= 3; j++)
        for (int i = 6*j + 1; i <= 6 * (j+1); i++)
            m += j * netboard.checkersOnPoint(color, i);
    return m;
}

void net::compute_crossovers(const color_t color, float *ib)
{
    float x_me = (float) crossovers(color);
    float x_him = (float) crossovers(opponentOf(color));

    ib[0] = x_me / 60.0f;  // is 60.0 too high?
    ib[1] = x_him / 60.0f;

    // A crossover lead of 5 corresponds to a value of
    // squash(1), which is about 75 percent.
    ib[2] = squash_sse( (x_me - x_him) / 5.0f);
    ib[3] = 1.0f - ib[2];
}


/*
 * For each weight, compute the partial derivate of the output
 * with respect to that weight, and store it in the appropriate
 * entry of the network "g".  This assumes that feedForward()
 * or feed_board() has already been called with the current input.
 */

#if 1
void net::calcGradient(net *g)
{
    int i, j;
    float oprime, hprime;

    oprime = output * (1 - output) ;
    for (i = 0; i < n_hidden; i++)
        g->weights_2[i] = oprime * hidden[i];

    for (i = 0; i < n_hidden; i++)
    {
        hprime = hidden[i] * (1 - hidden[i]) ;
        float product = hprime * oprime * weights_2[i];
        float *p = g->weights_1[i];
        for (j = 0; j < N_INPUTS; j++)
            p[j] = product * input[j];
    }
}

#else
void net::calcGradient(net *g)
{
    int i, j;
    float oprime, hprime;

    oprime = output * (1 - output) ;
    for (i = 0; i < n_hidden; i++)
        g->weights_2[i] = oprime * hidden[i];

// Why not eliminate the hprime * oprime calculations
// by computing hprime*oprime in the first place?
// Also, would it be advantageous to use float pointers
// with post-increment instead of subscripting by j?
// And I think we can compute n->weights_2[i] once for
// each i-loop (although the compiler might already
// be doing this.  Take a look.

    for (i = 0; i < n_hidden; i++)
    {
        hprime = hidden[i] * (1 - hidden[i]) ;
        for (j = 0; j < N_INPUTS; j++)
            g->weights_1[i][j] = hprime * oprime *
                input[j] * weights_2[i]; 
    }
}
#endif

#define NO_MOVE_YET -99.0f

void net::init_play()
{
    for (int i = 0; i < N_INPUTS; i++)
        input[i] = 0.0f;
    // This makes future marginal calculations work.
    feedForward();
}

void net::init_learning(float a, float l)
{
    if (we_learn == 0)
        return;
    if (acc_grad == 0)
    {
        acc_grad = new net(n_hidden, N_INPUTS);
        delta = new net(n_hidden, N_INPUTS);
        delta->clearNetwork();
        grad = new net(n_hidden, N_INPUTS);
    }
    acc_grad->clearNetwork();
    alpha = a;
    lambda = l;
    last_observation = NO_MOVE_YET;
    saw_final_signal = 0;
}

class fLearn
{
public:
    const float k;
    fLearn(float f) : k(f) {}
    void operator() (float &d, const float &a) const {
        d += (k * a);
    }
};

void net::learn(float pNew, float pOld)
{
    if (last_observation == NO_MOVE_YET)
        return;

    // for each weight in delta, w
    // delta.w += k * acc_grad.w
    /* times alpha, effectively */
    const float k = delta_equity_to_delta_net(pNew - pOld);
    applyToNetworks(delta, acc_grad, fLearn(k));
}

void net::observe(const float p)
{
    if (!we_learn)
        return;

    learn(p, last_observation);
    last_observation = p;
    accumGradient();
}

void net::observe_final(const float p)
{
    if (saw_final_signal || !we_learn)
        return;
    saw_final_signal = 1;
    learn(p, last_observation);
}

class fAccum
{
public:
    const float lambda;
    fAccum(float L) : lambda(L) {}
    void operator() (float &ag, float &g) const
    {
        ag = (lambda * ag) + g;
    }
};

void net::accumGradient()
{
    calcGradient(grad);
    // for each weight in acc_grad, w
    // acc_grad.w = (lambda * acc_grad.w) + grad.w
    applyToNetworks(acc_grad, grad, fAccum(lambda));
}

// this is really the same as fLearn.
class fUpdate
{
public:
    const float alpha;
    fUpdate(float A) : alpha(A) {}
    void operator() (float &n, float &d) const
    {
        n += alpha * d;
    }
};

float net::incorporate_learning()
{
    applyToNetworks(this, this->delta, fUpdate(alpha));

    float r = feedForward();
    delta->clearNetwork();
    return r;
}

void set_f(float &f)
{
    f = random_float(0.50);
}

void net::randomizeNetwork()
{
    applyFunction(set_f);
}

class fSave
{
public:
    FILE *netfp;
    int portable;

    fSave(FILE *fp, int p) : netfp(fp), portable(p){}

    void operator() (float &f) const
    {
        if (portable)
        {
            if (fprintf(netfp, "%.12f\n", (double) f) <= 0)
                fatal( "Error writing to network file.");
        }
        else
        {    // nonportable format saves all the bits in intel float format
            if (fwrite(&f, sizeof(f), 1, netfp) != 1)
                fatal("Error writing to network file.");
        }
    }
};

void net::dump_network(const char *fn, int portable)
{
    FILE *netfp;

    if ( (netfp = fopen(fn, "wb")) == NULL)
        fatal(std::string("Cannot open file ") + fn + " for writing.");

    fprintf(netfp, "portable format: %d\n", portable);
    fprintf(netfp, "net type: %d\n", n_type);
    fprintf(netfp, "hidden nodes: %d\n", n_hidden);
    fprintf(netfp, "input nodes: %d\n", N_INPUTS);

    applyFunction(fSave(netfp, portable));

//    fprintf(netfp, "Current seed: %ldL\n", current_seed());
    fprintf(netfp, "Current seed: %ldL\n", seed);
    fprintf(netfp, "Games trained: %ldL\n", games_trained);

    fclose(netfp);
}

void net::must_have(int ntype, int inputs, int mustval)
{
    if (inputs != mustval)
    {
        std::ostringstream s;
        s << "net type " << ntype << " must have " << mustval << " inputs, but has " << inputs << " instead";
        fatal(s.str());
    }
}


class fRead
{
public:
    FILE *netfp;
    int portable;
    fRead(FILE *fp, int p) : netfp(fp), portable(p){}
    void operator() (float &f) const
    {
        if (portable)
        {
            if (fscanf(netfp, "%f", &f) == 0)
                fatal("Error reading network file.");
        }
        else
        {
            if (fread(&f, 1, sizeof(f), netfp) != sizeof(f))
                fatal("Error reading network file.");
        }
    }
};


/* Allocate and initialize a network based
 * on the information in file fn.
 */

net *net::read_network(const char *fn)
{
    using namespace std;
    unsigned long sd;
    int hidden = 40;

    FILE *netfp;
    cout << "Reading network file: " << fn << "\n";
    if ( (netfp = fopen(fn, "rb")) == NULL)
        fatal(std::string("Cannot open network file: ") + fn);

    int portable = 1;
    int ignore = fscanf(netfp, " portable format: %d\n", &portable);
    cout << "portable format: " << (portable ? "yes" : "no") << "\n";

    int ntype = 0;
    // If ntype remains zero, then we have a really old-style net file.
    ignore = fscanf(netfp, " net type: %d\n", &ntype);

    ignore = fscanf(netfp, " hidden nodes: %d\n", &hidden);
    if (hidden > net::max_hidden)
        fatal(std::string("Too many hidden nodes: ") + std::to_string(hidden));

    int inputs = net::inputsForV2;

    char throwAway;
    ignore = fscanf(netfp, " input nodes: %d%c", &inputs, &throwAway);
    if (throwAway == '\r')
        ignore = fscanf(netfp, "%c", &throwAway);

    if (throwAway != '\n')
        fatal(std::string("Expected newline, got ") + std::to_string(throwAway));

    cout << "inputs=" << inputs << endl;

    assert(ntype == 3);

    net *p = new net_v3(hidden);

    p->filename = fn;
    p->applyFunction(fRead(netfp, portable));

    if (fscanf(netfp, " Current seed: %luL", &sd) != 1)
        fatal("Cannot read seed from network file.");

    p->seed = sd;

    if (fscanf(netfp, " Games trained: %ldL\n", &(p->games_trained)) != 1)
        p->games_trained = 0L;
    fclose(netfp);

    cout << "Finished." << endl;
    p->init_play();
    return p;
}

class clearFtn {
public:
    void operator() (float &f) const
    {
        f = 0.0f;
    }
};

void net::clearNetwork()
{
    applyFunction(clearFtn());
}
