/* Written by Todd Doucet.  See file copyright.txt.
 */
#include <stdexcept>
#include <fstream>

#include "net.h"

using namespace std;

static void write_float(std::ostream& fs, float f)
{
    fs.write( reinterpret_cast<char *>(&f), sizeof(f) );
}

void writeFile(BgNet& n, const char *fn)
{
    ofstream ofs{fn};
    if (!ofs)
        throw runtime_error(string("Cannot open file stream ") + fn + " for writing.");

    ofs << "portable format: " << 0 << "\n"; // legacy
    ofs << "net type: " << 3 << "\n";        // legacy
    ofs << "hidden nodes: " << n.n_hidden << "\n";
    ofs << "input nodes: " << n.n_inputs << "\n";

    for (int i = 0; i < n.n_hidden; i++)
        for (int j = 0; j < n.n_inputs; j++)
            write_float(ofs, n.M(i, j));

    for (int i = 0; i < n.n_hidden; i++)
        write_float(ofs, n.V(i));

    ofs << "Current seed: " << n.seed << "L\n";            // legacy
    ofs << "Games trained: " << n.games_trained << "L\n";  // legacy
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
BgNet *readFile(const char *fn)
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
    has(ifs, "input nodes:"); ifs >> input >> ws;

    auto p = new BgNet();

    for (int i = 0; i < p->n_hidden; i++)
        for (int j = 0; j < p->n_inputs; j++)
            p->M(i, j) = read_float(ifs);

    for (int i = 0; i < p->n_hidden; i++)
        p->V(0, i) = read_float(ifs);

    has(ifs, "Current seed:");
    char L;
    ifs >> p->seed >> L >> ws;
    has(ifs, "Games trained:"); ifs >> p->games_trained;

    return p;
}
