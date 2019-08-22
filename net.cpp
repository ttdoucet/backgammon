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

void net::compute_v2_inputs(const color_t color, float *ib)
{
    compute_contact(color, ib);
    compute_pip(color, ib + net::METRICS_CONTACT);
    compute_hit_danger(color, ib + net::METRICS_CONTACT + net::METRICS_PIP);
    compute_hit_danger(opponentOf(color), ib + net::METRICS_CONTACT +
                       net::METRICS_PIP + net::METRICS_HIT);
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


const char *net::checker_names_self[] =
{
    "off",
    "1-blot", "1-point", "1-build",
    "2-blot", "2-point", "2-build",
    "3-blot", "3-point", "3-build",
    "4-blot", "4-point", "4-build",
    "5-blot", "5-point", "5-build",
    "6-blot", "6-point", "6-build",
    "7-blot", "7-point", "7-build",
    "8-blot", "8-point", "8-build",
    "9-blot", "9-point", "9-build",
    "10-blot", "10-point", "10-build",
    "11-blot", "11-point", "11-build",
    "12-blot", "12-point", "12-build",
    "13-blot", "13-point", "13-build",
    "14-blot", "14-point", "14-build",
    "15-blot", "15-point", "15-build",
    "16-blot", "16-point", "16-build",
    "17-blot", "17-point", "17-build",
    "18-blot", "18-point", "18-build",
    "19-blot", "19-point", "19-build",
    "20-blot", "20-point", "20-build",
    "21-blot", "21-point", "21-build",
    "22-blot", "22-point", "22-build",
    "23-blot", "23-point", "23-build",
    "24-blot", "24-point", "24-build",
    "bar"
};

const char *net::checker_names_other[] =
{
    "opp-off",
    "opp-1-blot", "opp-1-point", "opp-1-build",
    "opp-2-blot", "opp-2-point", "opp-2-build",
    "opp-3-blot", "opp-3-point", "opp-3-build",
    "opp-4-blot", "opp-4-point", "opp-4-build",
    "opp-5-blot", "opp-5-point", "opp-5-build",
    "opp-6-blot", "opp-6-point", "opp-6-build",
    "opp-7-blot", "opp-7-point", "opp-7-build",
    "opp-8-blot", "opp-8-point", "opp-8-build",
    "opp-9-blot", "opp-9-point", "opp-9-build",
    "opp-10-blot", "opp-10-point", "opp-10-build",
    "opp-11-blot", "opp-11-point", "opp-11-build",
    "opp-12-blot", "opp-12-point", "opp-12-build",
    "opp-13-blot", "opp-13-point", "opp-13-build",
    "opp-14-blot", "opp-14-point", "opp-14-build",
    "opp-15-blot", "opp-15-point", "opp-15-build",
    "opp-16-blot", "opp-16-point", "opp-16-build",
    "opp-17-blot", "opp-17-point", "opp-17-build",
    "opp-18-blot", "opp-18-point", "opp-18-build",
    "opp-19-blot", "opp-19-point", "opp-19-build",
    "opp-20-blot", "opp-20-point", "opp-20-build",
    "opp-21-blot", "opp-21-point", "opp-21-build",
    "opp-22-blot", "opp-22-point", "opp-22-build",
    "opp-23-blot", "opp-23-point", "opp-23-build",
    "opp-24-blot", "opp-24-point", "opp-24-build",
    "opp-bar",
};

const char *net_v2::v2_names[] =
{
    "Not-Broken", "Broken",
    "Pip-Ahead", "Pip-Behind",

    "Hit-Dang-0", "Hit-Dang-1", "Hit-Dang-2", "Hit-Dang-3", "Hit-Dang-4",
    "Hit-Dang-5", "Hit-Dang-6", "Hit-Dang-7", "Hit-Dang-8",

    "Hit-Attk-0", "Hit-Attk-1", "Hit-Attk-2", "Hit-Attk-3", "Hit-Attk-4",
    "Hit-Attk-5", "Hit-Attk-6", "Hit-Attk-7", "Hit-Attk-8",
};

const char *net_v3::v3_names[] =
{
    "Not-Broken", "Broken",
    "Pip-Ahead", "Pip-Behind",

    "Hit-Dang", "!Hit-Dang",
    "Hit-Attk", "!Hit-Attk"
};

const char *net_v4::v4_names[] =
{
    "Not-Broken", "Broken",
    "Pip-Ahead", "Pip-Behind",

    "Hit-Dang", "!Hit-Dang",
    "Hit-Attk", "!Hit-Attk",

    "X-me", "X-him",
    "X-Behind",
    "X-Ahead"
};

const char *net::input_name(int n)
{
    fatal("input_name() on illegal net.");
    return "";
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
        for (j = 0; j < n_inputs; j++)
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
        for (j = 0; j < n_inputs; j++)
            g->weights_1[i][j] = hprime * oprime *
                input[j] * weights_2[i]; 
    }
}
#endif

#define NO_MOVE_YET -99.0f

void net::init_play()
{
    for (int i = 0; i < n_inputs; i++)
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
        acc_grad = new net(n_hidden, n_inputs);
        delta = new net(n_hidden, n_inputs);
        delta->clearNetwork();
        grad = new net(n_hidden, n_inputs);
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
    fprintf(netfp, "input nodes: %d\n", n_inputs);

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

void net::check_input_value(int ntype, int inputs)
{
    switch(ntype){
    case 1:
        must_have(ntype, inputs, net::inputsForV1);
        break;
    case 2:
        must_have(ntype, inputs, net::inputsForV2);
        break;
    case 3:
        must_have(ntype, inputs, net::inputsForV3);
        break;
    case 4:
        must_have(ntype, inputs, net::inputsForV4);
        break;
    default:
        fatal(std::string("invalid net type: ") + std::to_string(ntype) );
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

    if (ntype == 0)
    {  // We have an old-style net file.
        if (inputs == net::inputsForV2)
            ntype = 2;
        else
            ntype = 1;
    }

    // Do a consistency check.
    check_input_value(ntype, inputs);

    net *p;
    switch (ntype)
    {
    case 1: p = new net_v1(hidden); break;
    case 2: p = new net_v2(hidden); break;
    case 3: p = new net_v3(hidden); break;
    case 4: p = new net_v4(hidden); break;
    default:
        fatal(std::string("Unknown net type: ") + std::to_string(ntype));
    }
    p->filename = fn;
//      net::applyToNetwork(p, fRead(netfp, portable));
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
