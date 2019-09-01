/*
 * Copyright (C) 1993, 2019 by Todd Doucet.  All Rights Reserved.
 */
#include <stdexcept>
#include <fstream>

#include "net.h"

using namespace std;

struct fWrite
{
    void operator() (float &f) const
    {
        fs.write( reinterpret_cast<char *>(&f), sizeof(f) );
    }
    fWrite(ofstream& ofs) : fs(ofs) { }
    ostream& fs;
};

void net::writeFile(const char *fn)
{
    ofstream ofs{fn};
    if (!ofs)
        throw runtime_error(string("Cannot open file stream ") + fn + " for writing.");

    ofs << "portable format: " << 0 << "\n"; // legacy
    ofs << "net type: " << 3 << "\n";        // legacy
    ofs << "hidden nodes: " << N_HIDDEN << "\n";
    ofs << "input nodes: " << N_INPUTS << "\n";

    applyFunction(fWrite(ofs));

    ofs << "Current seed: " << seed << "L\n";            // legacy
    ofs << "Games trained: " << games_trained << "L\n";  // legacy
}

struct fRead
{
    void operator() (float &f) const
    {
        fs.read( reinterpret_cast<char *>(&f), sizeof(f) );
    }
    fRead(istream& ifs) : fs(ifs) { }
    istream& fs;
};

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
    p->applyFunction(fRead(ifs));

    has(ifs, "Current seed:");
    char L;
    ifs >> p->seed >> L >> ws;
    has(ifs, "Games trained:"); ifs >> p->games_trained;

    return p;
}
