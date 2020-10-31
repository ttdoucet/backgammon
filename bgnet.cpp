/* Written by Todd Doucet.  See file copyright.txt.
 */
#include <stdexcept>
#include <fstream>

#include "bgnet.h"

using namespace std;

static void write_float(ostream& fs, float f)
{
    fs.write( reinterpret_cast<char *>(&f), sizeof(f) );
}

bool netv3::writeFile(string fn) const
{
    bool quit = false;

    if ((*this).params.M.isfinite() == false)
    {
        std::cout << "ERROR: M is not finite\n";
        quit = true;
    }

    if ((*this).params.V.isfinite() == false)
    {
        std::cout << "ERROR: V is not finite\n";
        quit = true;
    }

    if (quit)
        return false;

    ofstream ofs(fn, ios::binary);
    if (!ofs)
        throw runtime_error(string("Cannot open file stream ") + fn + " for writing.");

    ofs << "portable format: " << 0 << "\n"; // legacy
    ofs << "net type: " << 3 << "\n";        // legacy
    ofs << "hidden nodes: " << (*this).n_hidden << "\n";
    ofs << "input nodes: " << (*this).n_inputs << "\n";

    for (int i = 0; i < (*this).n_hidden; i++)
        for (int j = 0; j < (*this).n_inputs; j++)
            write_float(ofs, (*this).params.M(i, j));

    for (int i = 0; i < (*this).n_hidden; i++)
        write_float(ofs, (*this).params.V(0, i));

    uint64_t seed;
    ofs << "Current seed: " <<seed << "L\n";            // legacy
    uint64_t games_trained;
    ofs << "Games trained: " << games_trained << "L\n";  // legacy

    ofs.close();
    return ofs.fail() == false;
}

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

// Read in a neural net from a file.
bool netv3::readFile(string fn)
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

    for (int i = 0; i < (*this).n_hidden; i++)
        for (int j = 0; j < (*this).n_inputs; j++)
            (*this).params.M(i, j) = read_float(ifs);

    for (int i = 0; i < (*this).n_hidden; i++)
        (*this).params.V(0, i) = read_float(ifs);

    if (ifs.fail())
        return false;

    if ((*this).params.M.isfinite() == false)
        throw runtime_error("M is not finite.");
    if ((*this).params.V.isfinite() == false)
        throw runtime_error("V is not finite.");

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

static BgNet *initBgNet(const string name)
{
    if (name == "netv3")
        return new netv3();

    // Support other BgNets here. . .

    return nullptr;
}

// Presently only supports netv3.
std::unique_ptr<BgNet> readBgNet(const string filename)
{
    BgNet *r;

    if (r = initBgNet(filename))
        return std::unique_ptr<BgNet>(r);

    cout << "default: must be netv3 file\n";
    r = new netv3();
    r->readFile(filename);
    return std::unique_ptr<BgNet>(r);
}
