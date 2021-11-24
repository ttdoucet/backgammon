/* Written by Todd Doucet. */
#include <iostream>
#include <iomanip>
#include <fstream>

#include "playernet.h"

using namespace std;

template<class Fc_Sig>
int transpose(BgNet& pp)
{
    auto np = dynamic_cast<Fc_Sig*>(&pp);
    if (np == 0)
        return 0;

    cout << pp.netname() << "tr\n";
    cout << np->params.M.Transpose() << "\n";
    cout << np->params.V.Transpose() << "\n";

    return 1;
}

int main(int argc, char *argv[])
{
    auto filename = argv[1];
    unique_ptr<BgNet> pp = BgNetReader::read(filename);

    return transpose<netv3>(*pp) ||
           transpose<Fc_Sig_H60_I3>(*pp) ||
           transpose<Fc_Sig_H90_I3>(*pp) ||
           transpose<Fc_Sig_H120_I3>(*pp);
}
