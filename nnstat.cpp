#include "net.h"
#include <iostream>

using namespace std;

BgNet neural;

int main(int argc, char *argv[])
{
    for (int i = 1; i < argc; i++)
    {
#if 0
        auto filename = argv[i];
        readFile(neural, filename);

        if (neural.parms.M.isfinite() == false)
            cout << filename << ": M not finite\n";

        if (neural.parms.V.isfinite() == false)
            cout << filename << ": V not finite\n";

        continue;
#endif

        cout << "V: ";
        cout << neural.parms.V;
        cout << "\n\n";

        cout << "M: ";
        cout << neural.parms.M;
        cout << "\n\n";

        cout << "V.magnitude(): " << neural.parms.V.magnitude() << "\n";
        cout << "M.magnitude(): " << neural.parms.M.magnitude() << "\n";
    }
}
