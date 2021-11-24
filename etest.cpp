/* Written by Todd Doucet. */
#include <iostream>
#include "netop.h"

using namespace std;

int main(int argc, char *argv[])
{
    float f;

    for (f = 0; f < 100; f = f + 1)
    {
        cout << f << ": " << exp(f)
             << ", sigma: " << logistic::fwd(f)
             << "\n";
    }
}
