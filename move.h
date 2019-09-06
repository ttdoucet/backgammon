#pragma once

#include <string>

#include "board.h"

class moves
{
public:
    struct move { int from, to, hit; };

    move& operator[](int i)
    {
//      assert(cnt > i);
        return m[i];
    }

    move  operator[](int i) const
    {
//      assert(cnt > i);
        return m[i];
    }

    int count() const { return cnt; }
    moves() { }

    void push(move mp) { m[cnt++] = mp; }

    move pop()
    {
#if 1
        move r = m[--cnt];
        m[cnt] = {0, 0, 0};
        return r;
#else
        return m[--cnt];
#endif
    }

    void clear()
    {
        cnt = 0;
    }

private:
    move m[4] = {0};
    int cnt = 0;
};

class callBack
{
public:
    moves m;
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

std::string moveStr(moves& m);

int plays(const board& b, callBack& callB);
int numMoves(board& b);
int checkersToPlay(board& b);
void applyMove(board& b,const moves& m);
