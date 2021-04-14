/* Written by Todd Doucet.  See file copyright.txt.
 */
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
    virtual float equity(const board& b) = 0;
    virtual bool readFile(std::string filename) = 0;
    virtual bool writeFile(std::string filename) const = 0;
    virtual std::string netname() const = 0;
    virtual ~BgNet() = default;
};

template<class feature_calc, int N_HIDDEN, template<int, int> class Net>
class BackgammonNet :  public BgNet,
                       public Net<feature_calc::count, N_HIDDEN>
{
public:
    /* Neural net estimate of the equity for the side on roll.
     */
    float equity(const board &b)
    {
        feature_calc::calc(b, this->input().Data());
        return this->feedForward();
    }

    bool readFile(std::string filename)
    {
        std::ifstream ifs(filename);
        std::string name;
        ifs >> name >> this->params.M >> this->params.V;
        return ifs.fail() == false;
    }

    bool writeFile(std::string filename) const
    {
        std::ofstream ofs(filename);
        ofs << this->netname() << "\n" << this->params.M << this->params.V;
        return ofs.fail() == false;
    }

    BackgammonNet()
    {
        assert( feature_calc::count == this->input().Rows() * this->input().Cols() );
    }
};

// Fully-connected, sigmoidal activations, 30 hidden units, input features version 3.
struct netv3 : public BackgammonNet<features_v3, 30, Fc_Sig>
{
    std::string netname() const { return "netv3"; }
};

// Fully-connected, sigmoidal activations, 60 hidden units, input features version 3.
struct Fc_Sig_H60_I3 : public BackgammonNet<features_v3, 60, Fc_Sig>
{
    std::string netname() const { return "Fc_Sig_H60_I3"; }
};

struct Fc_Sig_H90_I3 : public BackgammonNet<features_v3, 90, Fc_Sig>
{
    std::string netname() const { return "Fc_Sig_H90_I3"; }
};


struct Fc_Sig_H120_I3 : public BackgammonNet<features_v3, 120, Fc_Sig>
{
    std::string netname() const { return "Fc_Sig_H120_I3"; }
};

struct Fc_Sig_H1024_I3 : public BackgammonNet<features_v3, 1024, Fc_Sig>
{
    std::string netname() const { return "Fc_Sig_H1024_I3"; }
};

/* Factory
 */
std::unique_ptr<BgNet> readBgNet(const std::string filename);

