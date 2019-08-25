#pragma once

#include "game.h"

class NeuralNetPlayer : public Player, public callBack
{
public:
    NeuralNetPlayer(const char *player, const char *netname);
    virtual void prepareToPlay();
    virtual void finalEquity(double e) { }
    virtual void chooseMove(const board& b, moves& choice);

protected:
    typedef float (NeuralNetPlayer::* evalFunction)(const board& bd);
    NeuralNetPlayer::evalFunction equityEstimator;

    int callBackF(const board &b);
    void selectMove(const board &b, moves& m, evalFunction func);

protected:
    class net *neural;

    float bestEquity;
    moves bestMove;

protected:

    float littleE(const board &bd);
    float bearoffEquity(const board &b);

    static unsigned long board_to_32(const board &b, color_t c);
    static bool isBearingOff(const board &bd);

    static int gameOver(const board &bd);
    static int win_check(const board &nb, color_t side);
    static int gammon_check(const board &nb, color_t winner);
};

