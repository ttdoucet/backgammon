/* Written by Todd Doucet.  See file copyright.txt.
 */
#include <stdexcept>
#include <fstream>
#include <iomanip>

#include "bgnet.h"

using namespace std;


static float read_float(istream& ifs)
{
    float f;
    ifs.read( reinterpret_cast<char *>(&f), sizeof(f) );
    return f;
}

static bool has(istream& is, const char *str)
{
    char ch;
    const char *s;

    for(s = str; *s; s++)
        if ( !(is.get(ch)) || (ch != *s) )
            return false;
    return *s == 0;
}

// Legacy format for netv3 parameters.
bool readFile_legacy(string fn, netv3::Parameters& params)
{
    int hidden = 40, portable = 1;
    int ntype = 0, input = 0;

    ifstream ifs(fn, ios::binary);

    if (!ifs)
        throw runtime_error("Cannot open network file: "s + fn);

    has(ifs, "portable format:"); ifs >> portable >> ws;
    assert(portable == 0);
    has(ifs, "net type:"); ifs >> ntype >> ws;
    assert(ntype == 3);
    has(ifs, "hidden nodes:"); ifs >> hidden >> ws;
    has(ifs, "input nodes:"); ifs >> input;

    char ch;
    ifs.get(ch);
    if (ch != '\n')
        return false;

    const int n_hidden = params.M.Rows();
    const int n_inputs = params.M.Cols();

    for (int i = 0; i < n_hidden; i++)
        for (int j = 0; j < n_inputs; j++)
            params.M(i, j) = read_float(ifs);

    for (int i = 0; i < n_hidden; i++)
        params.V(0, i) = read_float(ifs);

    if (ifs.fail())
        return false;

    has(ifs, "Current seed:");

    char L;
    uint64_t seed = 0; // legacy
    ifs >> seed >> L >> ws;
    uint64_t games_trained = 0; // legacy
    has(ifs, "Games trained:"); ifs >> games_trained;

    ifs.close();
    return ifs.fail() == false;
}


/* Factory
 */

static string net_name(const string filename)
{
    const auto max_name_length = 30;
    ifstream ifs(filename, ios::binary);
    string name;
    ifs >> setw(max_name_length) >> name;
    if (name == "portable")
        name = "legacy-v3";
    return name;
}

static unique_ptr<BgNet> initBgNet(const string name)
{
    if (name == "netv3")
        return make_unique<netv3>();

    if (name == "Fc_Sig_H15_I3")
        return make_unique<Fc_Sig_H15_I3>();

    if (name == "Fc_Sig_H60_I3")
        return make_unique<Fc_Sig_H60_I3>();

    if (name == "Fc_Misc_H30_I3")
        return make_unique<Fc_Misc_H30_I3>();

    if (name == "Fc_Sig_H60_I3b")
        return make_unique<Fc_Sig_H60_I3b>();

    // Support other BgNets here. . .

    return nullptr;
}

std::unique_ptr<BgNet> readBgNet(const string filename)
{
    if (auto r = initBgNet(filename))
        return r;

    string name = net_name(filename);
    // cout << "net id: " << name << "\n";

    if (auto r = initBgNet(name))
    {
        if (r->readFile(filename))
            return r;
        throw runtime_error("Error reading net file: " + filename);
    }

    cout << "legacy netv3 file: " << filename << "\n";
    auto r = make_unique<netv3>();

    if (readFile_legacy(filename, r->params) == false)
        throw runtime_error("Error reading legacy net file: " + filename);
    return r;
}
