#include "bgnet.h"
#include <iostream>

using namespace std;

int main(int argc, char *argv[])
{
    for (int i = 1; i < argc; i++)
    {
        auto filename = argv[i];

        unique_ptr<BgNet> bgnet = readBgNet(filename);

        // Presently we have to choose the specific flavor
        // of bgnet to use, and in particular it must
        // be something with parameters of the form we expect.
        // This code is a hold-over from simpler times.

        // auto neural = dynamic_cast<netv3*>(bgnet.get());
        auto neural = dynamic_cast<Fc_ReLU_H60_I3*>(bgnet.get());

        if (neural == nullptr)
            cout << "Could not read net " << filename << "\n";

        cout << filename << "  magnitudes, ";
        cout << "V: " <<  neural->params.V.magnitude() << ", ";
        cout << "M: " << neural->params.M.magnitude() << "\n";
    }
}
