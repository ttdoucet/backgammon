/* Written by Todd Doucet.  See file copyright.txt.
 */
#pragma once

#include <fstream>
#include <iomanip>
#include <memory>

#include "net.h"
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
class netv3 : public BackgammonNet<features_v3, 30, SigmoidNet>
{
public:
    std::string netname() const { return "netv3"; }
};

// Fully-connected, sigmoidal activations, 15 hidden units, input features version 3.
class Fc_Sig_H15_I3 : public BackgammonNet<features_v3, 15, SigmoidNet>
{
public:
    std::string netname() const { return "Fc_Sig_H15_I3"; }
};

// Fully-connected, sigmoidal activations, 60 hidden units, input features version 3.
class Fc_Sig_H60_I3 : public BackgammonNet<features_v3, 60, SigmoidNet>
{
public:
    std::string netname() const { return "Fc_Sig_H60_I3"; }
};

// Fully-connected, sigmoidal activations, 60 hidden units, input features version 3b.
class Fc_Sig_H60_I3b : public BackgammonNet<features_v3b, 60, SigmoidNet>
{
public:
    std::string netname() const { return "Fc_Sig_H60_I3b"; }
};




// Fully-connected, sigmoidal activations, 60 hidden units, input features version 3.
class Fc_Misc_H30_I3 : public BackgammonNet<features_v3, 30, MiscNet>
{
public:
    std::string netname() const { return "Fc_Misc_H30_I3"; }
};




/* Factory
 */
std::unique_ptr<BgNet> readBgNet(const std::string filename);

