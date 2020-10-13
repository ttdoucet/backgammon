#include "net.h"
#include <iostream>

using namespace std;

netv3 neural;

int main(int argc, char *argv[])
{
    for (int i = 1; i < argc; i++)
    {
        auto filename = argv[i];
        bool st = readFile(neural, filename);
        if (st == false)
            std::cout << "Could not read net " << filename << "\n";

        cout << filename << "  magnitudes, ";
        cout << "V: " <<  neural.params.V.magnitude() << ", ";
        cout << "M: " << neural.params.M.magnitude() << "\n";

/*
        cout << neural.params.M;

        matrix<30, 156> foo;
        std::cin >> foo;

        if (foo == neural.params.M)
            std::cout << "They are equal\n";
        else
            std::cout << "They are not equal\n";
*/
    }
}
