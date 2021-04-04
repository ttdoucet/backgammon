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
        vec<200> linear_1; //pre_hidden;
        vec<200> hidden_1;
        vec<1>   linear_2;
        vec<1>   hidden_2;
        vec<1>   output;
    };

    struct params
    {
        matrix<200, 100> M;
        matrix<1, 200> V;
    };

    activs act;
    params param, grad;

    auto Op_1 = Linear(act.input, act.linear_1, param.M, grad.M);
    auto Op_2 = Termwise<logistic>(act.linear_1, act.hidden_1);
    auto Op_3 = Linear(act.hidden_1, act.linear_2, param.V, grad.V);
    auto Op_4 = Termwise<bipolar_sigmoid>(act.linear_2, act.hidden_2);
    auto Op_5 = Termwise<affine<3,0>>(act.hidden_2, act.output);

    // Forward propagation.
    Op_1.fwd();
    Op_2.fwd();
    Op_3.fwd();
    Op_4.fwd();
    Op_5.fwd();
    // Then act.output is the output.

    // Back propagation.
    act.output(0,0) = 1; // or alpha
    Op_5.bwd();
    Op_4.bwd();
    Op_3.bwd();
    Op_2.bwd();
    Op_1.bwd_param();
    // Then grad has the gradient.
}
