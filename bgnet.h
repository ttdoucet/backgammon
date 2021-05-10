/* Written by Todd Doucet. */
#pragma once

#include <fstream>
#include <iomanip>
#include <memory>

#include "fc_sig.h"
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

template<class feature_calc, template<int, int...> class Net, int...Args>
class BackgammonNet :  public BgNet,
                       public Net<feature_calc::count, Args...>
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

/* Factory */
std::unique_ptr<BgNet> readBgNet(const std::string filename);

