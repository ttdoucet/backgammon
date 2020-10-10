#include "net.h"
#include <iostream>

using namespace std;

BgNet neural;

int main(int argc, char *argv[])
{
    for (int i = 1; i < argc; i++)
    {
        auto filename = argv[i];
        bool st = readFile(neural, filename);
        if (st == false)
            std::cout << "Could not read net " << filename << "\n";

        cout << filename << "  magnitudes, ";
        cout << "V: " <<  neural.parms.V.magnitude() << ", ";
        cout << "M: " << neural.parms.M.magnitude() << "\n";

        cout << "M: " << neural.parms.M;
    }
}
