/* Written by Todd Doucet. */
#pragma once

#include <fstream>
#include <iomanip>
#include <memory>

#include "fc_sig.h"
#include "fc_sig_bias.h"
#include "model.h"

#include "board.h"
#include "features.h"

/* Backgammon-specific class derivations.
 */
class BgNet
{
public:
    using string = std::string;

    virtual float equity(const board& b) = 0;
    virtual bool readFile(string filename) = 0;
    virtual bool writeFile(string filename) const = 0;
    virtual string netname() const = 0;
    virtual ~BgNet() = default;
};

template<class feature_calc, template<int, int...> class Net, int...args>
class BackgammonNet :  public BgNet,
                       public Net<feature_calc::count, args...>
{
public:
    /* Neural net estimate of the equity for the side on roll.
     */
    float equity(const board &b)
    {
        feature_calc::calc(b, this->input().Data());
        return this->feedForward();
    }

    bool readFile(string filename)
    {
        std::ifstream ifs(filename);
        string name;
        ifs >> name >> this->params;
        return ifs.fail() == false;
    }

    bool writeFile(string filename) const
    {
        std::ofstream ofs(filename);
        ofs << this->netname() << "\n" << this->params;
        return ofs.fail() == false;
    }

    BackgammonNet()
    {
        assert( feature_calc::count == this->input().Rows() * this->input().Cols() );
    }
};

/* Fully-connected, sigmoidal activations, 30 hidden units, input features version 3. */
struct netv3 : public BackgammonNet<features_v3, Fc_Sig, 30>
{
    string netname() const { return "netv3"; }
};

// experiment with different input representation.
struct netv5 : public BackgammonNet<features_v5, Fc_Sig, 30>
{
    string netname() const { return "netv5"; }
};


struct Fc_Sig_H60_I3 : public BackgammonNet<features_v3, Fc_Sig, 60>
{
    string netname() const { return "Fc_Sig_H60_I3"; }
};

struct Fc_Sig_H90_I3 : public BackgammonNet<features_v3, Fc_Sig, 90>
{
    string netname() const { return "Fc_Sig_H90_I3"; }
};

struct Fc_Sig_H120_I3 : public BackgammonNet<features_v3, Fc_Sig, 120>
{
    string netname() const { return "Fc_Sig_H120_I3"; }
};

struct Fc_Sig_Bias_H30_I5 : public BackgammonNet<features_v5, Fc_Sig_Bias, 30>
{
    string netname() const { return "Fc_Sig_Bias_H30_I5"; }
};

struct Fc_Sig_Bias_H60_I5 : public BackgammonNet<features_v5, Fc_Sig_Bias, 60>
{
    string netname() const { return "Fc_Sig_Bias_H60_I5"; }
};


struct Experimental : public BackgammonNet<features_v3, ExperimentalModel, 30>
{
    string netname() const { return "Experimental"; }
};


/* Factory */
std::unique_ptr<BgNet> readBgNet(const std::string filename);

