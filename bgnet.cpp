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
        vec<100> x;
        vec<200> y;
    };

    struct params
    {
        matrix<200, 100> M;
    };

    activs activ;
    params param, grad;

    Linear<100,200> mapping{activ.x, activ.y, param.M, grad.M};

// I guess template argument deduction also works.
//   Linear mapping{activ.x, activ.y, param.M, grad.M};

    mapping.fwd();
    mapping.bwd();
}
