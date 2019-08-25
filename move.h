#pragma once

#include <string>
#include <tuple>

#include "board.h"
#include "console.h"


struct move {
    int from[4];    // move from[i] - to[i], hit iff hit[i] 
    int to[4];      // for i = 0 to moves-1
    int hit[4];
    int d1, d2;
    int moves;      

    void clear()
    {
        moves = 0;
        for (int i = 0; i < 4; i++)
            to[i] = from[i] = hit[i] = 0;
    }

    move() { clear(); }
};

class callBack
{
public:
    struct move m;
    int nMoves;
    int checkersToPlay;

    virtual int callBackF(const board &b) = 0;
    virtual ~callBack(){ }
    callBack() : nMoves(0), checkersToPlay(0) { }
};

class nullCallBack : public callBack
{
    int callBackF(const board &b) { return 0; }
};

std::string moveStr(struct move& m);

int plays(const board& b, callBack& callB);
void applyMove(board& b,const move& m);
int numMoves(board& b);
int checkersToPlay(board& b);
