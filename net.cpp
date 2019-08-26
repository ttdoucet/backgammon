/*
 * Copyright (C) 1993, 2019 by Todd Doucet.  All Rights Reserved.
 */

#include <string.h>
#include <stdexcept>

#include "board.h"
#include "net.h"

stopwatch mtimer;
stopwatch ftimer;
stopwatch stimer;

class fWrite
{
public:
    FILE *netfp;

    fWrite(FILE *fp, int p) : netfp(fp) { }

    void operator() (float &f) const
    {
        if (fwrite(&f, sizeof(f), 1, netfp) != 1)
            throw std::runtime_error("Error writing to network file.");
    }
};

void net::writeFile(const char *fn)
{
    FILE *netfp;

    if ( (netfp = fopen(fn, "wb")) == NULL)
        throw std::runtime_error(std::string("Cannot open file ") + fn + " for writing.");

    fprintf(netfp, "portable format: %d\n", 0); // legacy
    fprintf(netfp, "net type: %d\n", 3);  // legacy
    fprintf(netfp, "hidden nodes: %d\n", N_HIDDEN);
    fprintf(netfp, "input nodes: %d\n", N_INPUTS);

    applyFunction(fWrite(netfp, 0));
    fprintf(netfp, "Current seed: %ldL\n", seed);
    fprintf(netfp, "Games trained: %ldL\n", games_trained);

    fclose(netfp);
}


class fRead
{
public:
    FILE *netfp;
    int portable;
    fRead(FILE *fp) : netfp(fp) { }
    void operator() (float &f) const
    {
        if (fread(&f, 1, sizeof(f), netfp) != sizeof(f))
            throw std::runtime_error("Error reading network file.");
    }
};


/* Allocate and initialize a network based
 * on the information in file fn.
 */
net *net::readFile(const char *fn)
{
    using namespace std;
    unsigned long sd;
    int hidden = 40;

    FILE *netfp;
    if ( (netfp = fopen(fn, "rb")) == NULL)
        throw std::runtime_error(std::string("Cannot open network file: ") + fn);

    int portable = 1;
    int ignore = fscanf(netfp, " portable format: %d\n", &portable);
    assert(portable == 0);


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
        throw std::runtime_error(std::string("Expected newline, got ") + std::to_string(throwAway));

    assert(ntype == 3);

    net *p = new net();

    p->filename = fn;
    p->applyFunction(fRead(netfp));

    if (fscanf(netfp, " Current seed: %luL", &sd) != 1)
        throw std::runtime_error("Cannot read seed from network file.");

    p->seed = sd;

    if (fscanf(netfp, " Games trained: %ldL\n", &(p->games_trained)) != 1)
        p->games_trained = 0L;

    fclose(netfp);

    p->init_play();
    return p;
}
