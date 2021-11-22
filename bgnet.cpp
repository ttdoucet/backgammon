/* Written by Todd Doucet. */
#include <stdexcept>
#include <fstream>
#include <iomanip>

#include "bgnet.h"

using namespace std;

/* Factory
 */

static string net_name(const string filename)
{
    const auto max_name_length = 30;
    ifstream ifs(filename, ios::binary);
    string name;
    ifs >> setw(max_name_length) >> name;
    return name;
}

std::unique_ptr<BgNet> readBgNet(const string filename)
{
    net_factory nf;

    if (auto r = nf.initBgNet(filename))
        return r;

    string name = net_name(filename);

    if (auto r = nf.initBgNet(name))
        if (r->readFile(filename))
            return r;

    throw runtime_error("Error reading net file: " + filename);
}
