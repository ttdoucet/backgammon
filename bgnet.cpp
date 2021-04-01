/* Written by Todd Doucet.  See file copyright.txt.
 */
#include <stdexcept>
#include <fstream>
#include <iomanip>

#include "bgnet.h"

using namespace std;

/* Factory
 */

static string net_name(const string filename)
{
    const auto max_name_length = 30;
    ifstream ifs(filename, ios::binary);
    string name;
    ifs >> setw(max_name_length) >> name;
    return name;
}

static unique_ptr<BgNet> initBgNet(const string name)
{
    if (name == "netv3")
        return make_unique<netv3>();

    if (name == "Fc_Sig_H60_I3")
        return make_unique<Fc_Sig_H60_I3>();

    if (name == "Fc_Sig_H90_I3")
        return make_unique<Fc_Sig_H90_I3>();

    if (name == "Fc_Sig_H120_I3")
        return make_unique<Fc_Sig_H120_I3>();

    if (name == "Fc_Sig_H1024_I3")
        return make_unique<Fc_Sig_H1024_I3>();

    if (name == "Fc_Misc_H30_I3")
        return make_unique<Fc_Misc_H30_I3>();


    // Support other BgNets here. . .

    return nullptr;
}

std::unique_ptr<BgNet> readBgNet(const string filename)
{
    if (auto r = initBgNet(filename))
        return r;

    string name = net_name(filename);

    if (auto r = initBgNet(name))
        if (r->readFile(filename))
            return r;

    throw runtime_error("Error reading net file: " + filename);
}

/*
 *  Temporary code.
 */

void testingit()
{
    struct activs
    {
        vec<100> input;
        vec<200> pre_hidden;
        vec<200> hidden;
        vec<1>   pre_output;
        vec<1>   output;
    };

    struct params
    {
        matrix<200, 100> M;
        matrix<1, 200> V;
    };

    activs activ;
    params param, grad;

    auto Op_1 = Linear(activ.input, activ.pre_hidden, param.M, grad.M);
    auto Op_2 = Termwise<logistic>(activ.pre_hidden, activ.hidden);
    auto Op_3 = Linear(activ.hidden, activ.pre_output, param.V, grad.V);
    auto Op_4 = Termwise<logistic>(activ.pre_output, activ.output);

    Op_1.fwd();
    Op_2.fwd();
    Op_3.fwd();
    Op_4.fwd();

    Op_4.bwd();
    Op_3.bwd();
    Op_2.bwd();
    Op_1.bwd();
}
