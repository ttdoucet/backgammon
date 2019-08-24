/*
 * Copyright (C) 1993 by Todd Doucet.  All Rights Reserved.
 */

#include <string.h>

#include "board.h"
#include "net.h"
#include "hits.h"
#include "random.h"
#include "console.h"

class fSave
{
public:
    FILE *netfp;
    int portable;

    fSave(FILE *fp, int p) : netfp(fp), portable(p){}

    void operator() (float &f) const
    {
        if (portable)
        {
            if (fprintf(netfp, "%.12f\n", (double) f) <= 0)
                fatal( "Error writing to network file.");
        }
        else
        {    // nonportable format saves all the bits in intel float format
            if (fwrite(&f, sizeof(f), 1, netfp) != 1)
                fatal("Error writing to network file.");
        }
    }
};

void net::dump_network(const char *fn, int portable)
{
    FILE *netfp;

    if ( (netfp = fopen(fn, "wb")) == NULL)
        fatal(std::string("Cannot open file ") + fn + " for writing.");

    fprintf(netfp, "portable format: %d\n", portable);
    fprintf(netfp, "hidden nodes: %d\n", N_HIDDEN);
    fprintf(netfp, "input nodes: %d\n", N_INPUTS);

    applyFunction(fSave(netfp, portable));
    fprintf(netfp, "Current seed: %ldL\n", seed);

    fclose(netfp);
}


class fRead
{
public:
    FILE *netfp;
    int portable;
    fRead(FILE *fp, int p) : netfp(fp), portable(p){}
    void operator() (float &f) const
    {
        if (portable)
        {
            if (fscanf(netfp, "%f", &f) == 0)
                fatal("Error reading network file.");
        }
        else
        {
            if (fread(&f, 1, sizeof(f), netfp) != sizeof(f))
                fatal("Error reading network file.");
        }
    }
};


/* Allocate and initialize a network based
 * on the information in file fn.
 */
net *net::read_network(const char *fn)
{
    using namespace std;
    unsigned long sd;
    int hidden = 40;

    FILE *netfp;
    cout << "Reading network file: " << fn << "\n";
    if ( (netfp = fopen(fn, "rb")) == NULL)
        fatal(std::string("Cannot open network file: ") + fn);

    int portable = 1;
    int ignore = fscanf(netfp, " portable format: %d\n", &portable);
    cout << "portable format: " << (portable ? "yes" : "no") << "\n";

    int ntype = 0;
    // If ntype remains zero, then we have a really old-style net file.
    ignore = fscanf(netfp, " net type: %d\n", &ntype);

    ignore = fscanf(netfp, " hidden nodes: %d\n", &hidden);

    int inputs;

    char throwAway;
    ignore = fscanf(netfp, " input nodes: %d%c", &inputs, &throwAway);
    if (throwAway == '\r')
        ignore = fscanf(netfp, "%c", &throwAway);

    if (throwAway != '\n')
        fatal(std::string("Expected newline, got ") + std::to_string(throwAway));

    cout << "inputs=" << inputs << endl;

    assert(ntype == 3);

    net *p = new net();

    p->filename = fn;
    p->applyFunction(fRead(netfp, portable));

    if (fscanf(netfp, " Current seed: %luL", &sd) != 1)
        fatal("Cannot read seed from network file.");

    p->seed = sd;
    fclose(netfp);

    cout << "Finished." << endl;
    p->init_play();
    return p;
}
