#include "net.h"
#include <iostream>

using namespace std;

BgNet neural;

int main(int argc, char *argv[])
{
    readFile(neural, argv[1]);

    cout << "V: ";
    cout << neural.V;
    cout << "\n\n";

    cout << "M: ";
    cout << neural.M;
    cout << "\n\n";

    cout << "V.magnitude(): " << neural.V.magnitude() << "\n";
    cout << "M.magnitude(): " << neural.M.magnitude() << "\n";

}
