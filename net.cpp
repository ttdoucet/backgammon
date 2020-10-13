/* Written by Todd Doucet.  See file copyright.txt.
 */
#include <stdexcept>
#include <fstream>

#include "net.h"

using namespace std;

static void write_float(ostream& fs, float f)
{
    fs.write( reinterpret_cast<char *>(&f), sizeof(f) );
}

bool writeFile(const netv3& n, string fn)
{
    bool quit = false;

    if (n.params.M.isfinite() == false)
    {
        std::cout << "ERROR: M is not finite\n";
        quit = true;
    }

    if (n.params.V.isfinite() == false)
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
    ofs << "hidden nodes: " << n.n_hidden << "\n";
    ofs << "input nodes: " << n.n_inputs << "\n";

    for (int i = 0; i < n.n_hidden; i++)
        for (int j = 0; j < n.n_inputs; j++)
            write_float(ofs, n.params.M(i, j));

    for (int i = 0; i < n.n_hidden; i++)
        write_float(ofs, n.params.V(0, i));

    ofs << "Current seed: " << n.seed << "L\n";            // legacy
    ofs << "Games trained: " << n.games_trained << "L\n";  // legacy

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
bool readFile(netv3 &n, string fn)
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

    for (int i = 0; i < n.n_hidden; i++)
        for (int j = 0; j < n.n_inputs; j++)
            n.params.M(i, j) = read_float(ifs);

    for (int i = 0; i < n.n_hidden; i++)
        n.params.V(0, i) = read_float(ifs);

    if (ifs.fail())
        return false;

    if (n.params.M.isfinite() == false)
        throw runtime_error("M is not finite.");
    if (n.params.V.isfinite() == false)
        throw runtime_error("V is not finite.");

    has(ifs, "Current seed:");

    char L;
    ifs >> n.seed >> L >> ws;
    has(ifs, "Games trained:"); ifs >> n.games_trained;

    ifs.close();
    return ifs.fail() == false;
}
