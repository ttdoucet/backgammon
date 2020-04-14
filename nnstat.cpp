#include "net.h"
#include <iostream>

using namespace std;

BgNet neural;

int main(int argc, char *argv[])
{
    readFile(neural, argv[1]);

    cout << "V: ";
    cout << neural.parms.V;
    cout << "\n\n";

    cout << "M: ";
    cout << neural.parms.M;
    cout << "\n\n";

    cout << "V.magnitude(): " << neural.parms.V.magnitude() << "\n";
    cout << "M.magnitude(): " << neural.parms.M.magnitude() << "\n";

}
