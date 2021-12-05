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
    static bool is_named(std::string name) { return name == "netv3"; }
    string netname() const { return "netv3"; }
};

struct netv1 : public BackgammonNet<features_v1, Fc_SigTr, 30>
{
    static bool is_named(std::string name) { return name == "netv1"; }
    string netname() const { return "netv1"; }
};

struct Fc_Sig_H60_I1 : public BackgammonNet<features_v1, Fc_SigTr, 60>
{
    static bool is_named(std::string name) { return name == "Fc_Sig_H60_I1"; }
    string netname() const { return "Fc_Sig_H60_I1"; }
};



struct netv3tr : public BackgammonNet<features_v3, Fc_SigTr, 30>
{
    static bool is_named(std::string name) { return name == "netv3tr"; }
    string netname() const { return "netv3tr"; }
};


// experiment with different input representation.
struct netv5 : public BackgammonNet<features_v5, Fc_Sig, 30>
{
    static bool is_named(std::string name) { return name == "netv5"; }
    string netname() const { return "netv5"; }
};


struct Fc_Sig_H60_I5 : public BackgammonNet<features_v5, Fc_Sig, 60>
{
    static bool is_named(std::string name) { return name == "Fc_Sig_H60_I5"; }
    string netname() const { return "Fc_Sig_H60_I5"; }
};

struct Fc_Sig_H60_I5B : public BackgammonNet<features_v5b, Fc_SigTr, 60>
{
    static bool is_named(std::string name) { return name == "Fc_Sig_H60_I5B"; }
    string netname() const { return "Fc_Sig_H60_I5B"; }
};

struct Fc_Sig_H60_I3 : public BackgammonNet<features_v3, Fc_Sig, 60>
{
    static bool is_named(std::string name) { return name == "Fc_Sig_H60_I3"; }
    string netname() const { return "Fc_Sig_H60_I3"; }
};

 struct Fc_Sig_H60_I3tr : public BackgammonNet<features_v3, Fc_SigTr, 60>
{
    static bool is_named(std::string name) { return name == "Fc_Sig_H60_I3tr"; }
    string netname() const { return "Fc_Sig_H60_I3tr"; }
};


struct Fc_Sig_H90_I3 : public BackgammonNet<features_v3, Fc_Sig, 90>
{
    static bool is_named(std::string name) { return name == "Fc_Sig_H90_I3"; }
    string netname() const { return "Fc_Sig_H90_I3"; }
};

struct Fc_Sig_H90_I3tr : public BackgammonNet<features_v3, Fc_SigTr, 90>
{
    static bool is_named(std::string name) { return name == "Fc_Sig_H90_I3tr"; }
    string netname() const { return "Fc_Sig_H90_I3tr"; }
};


struct Fc_Sig_H120_I3 : public BackgammonNet<features_v3, Fc_Sig, 120>
{
    static bool is_named(std::string name) { return name == "Fc_Sig_H120_I3"; }
    string netname() const { return "Fc_Sig_H120_I3"; }
};

struct Fc_Sig_H120_I3tr : public BackgammonNet<features_v3, Fc_SigTr, 120>
{
    static bool is_named(std::string name) { return name == "Fc_Sig_H120_I3tr"; }
    string netname() const { return "Fc_Sig_H120_I3tr"; }
};

struct Fc_Sig_H240_I3tr : public BackgammonNet<features_v3, Fc_SigTr, 240>
{
    static bool is_named(std::string name) { return name == "Fc_Sig_H240_I3tr"; }
    string netname() const { return "Fc_Sig_H240_I3tr"; }
};


struct Fc_Sig_H30_I3N : public BackgammonNet<features_v3n, Fc_SigTr, 30>
{
    static bool is_named(std::string name) { return name == "Fc_Sig_H30_I3N"; }
    string netname() const { return "Fc_Sig_H30_I3N"; }
};

struct Fc_Sig_H60_I3N : public BackgammonNet<features_v3n, Fc_SigTr, 60>
{
    static bool is_named(std::string name) { return name == "Fc_Sig_H60_I3N"; }
    string netname() const { return "Fc_Sig_H60_I3N"; }
};

struct Fc_Sig_H90_I3N : public BackgammonNet<features_v3n, Fc_SigTr, 90>
{
    static bool is_named(std::string name) { return name == "Fc_Sig_H90_I3N"; }
    string netname() const { return "Fc_Sig_H90_I3N"; }
};

struct Fc_Sig_H120_I3N : public BackgammonNet<features_v3n, Fc_SigTr, 120>
{
    static bool is_named(std::string name) { return name == "Fc_Sig_H120_I3N"; }
    string netname() const { return "Fc_Sig_H120_I3N"; }
};



struct BgNetFactory
{
    template<typename ... NetType>
    struct nnlist
    {
        static std::unique_ptr<BgNet> create(const std::string name)
        {
            std::unique_ptr<BgNet> p;
            ( (NetType::is_named(name) && (p = std::make_unique<NetType>())) || ... );
            return p;
        }
    };

    static std::unique_ptr<BgNet> create(const std::string name)
    {
        using supported = nnlist<netv3, Fc_Sig_H60_I3, Fc_Sig_H90_I3, Fc_Sig_H120_I3,
                                 netv5, Fc_Sig_H60_I5, // Fc_Sig_H90_I5, Fc_Sig_H120_I5,
                                 netv3tr, Fc_Sig_H60_I3tr, Fc_Sig_H90_I3tr, Fc_Sig_H120_I3tr,
                                 Fc_Sig_H240_I3tr,
                                 Fc_Sig_H60_I5B,
                                 netv1, Fc_Sig_H60_I1,
                                 Fc_Sig_H30_I3N, Fc_Sig_H60_I3N, Fc_Sig_H90_I3N, Fc_Sig_H120_I3N
                                 >;

        return supported::create(name);
    }
};

class BgNetReader
{
    static std::string net_name(const std::string filename)
    {
        using namespace std;
        const auto max_name_length = 30;
        ifstream ifs(filename, ios::binary);
        string name;
        ifs >> setw(max_name_length) >> name;
        return name;
    }

public:
    static std::unique_ptr<BgNet> read(const std::string filename)
    {
        if (auto r = BgNetFactory::create(filename))
            return r;

        std::string name = net_name(filename);

        if (auto r = BgNetFactory::create(name))
            if (r->readFile(filename))
                return r;

        throw std::runtime_error("Error reading net file: " + filename);
    }
};
