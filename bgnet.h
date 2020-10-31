/* Written by Todd Doucet.  See file copyright.txt.
 */
#pragma once

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
    virtual ~BgNet() = default;
};

template<class feature_calc, int N_HIDDEN>
class BackgammonNet :  public BgNet,
                       public net<feature_calc::count, N_HIDDEN>
{
public:
    /* Neural net estimate of the equity for the side on roll.
     */
    float equity(const board &b)
    {
        feature_calc::calc(b, this->input.Data());
        return this->feedForward();
    }

    BackgammonNet()
    {
        assert( feature_calc::count == this->input.Rows() * this->input.Cols() );
    }
};

class netv3 : public BackgammonNet<features_v3<float*>, 30>
{
public:
    bool readFile(std::string filename);
    bool writeFile(std::string filename) const;
    ~netv3() { }
};

/* Factory
 */
std::unique_ptr<BgNet> readBgNet(const std::string filename);

