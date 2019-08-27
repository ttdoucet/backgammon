#pragma once

#include "game.h"
#include "net.h"

class NeuralNetPlayer : public Player, public callBack
{
public:
    NeuralNetPlayer(const char *player, const char *netname);
    void chooseMove(const board& b, moves& choice) override;

protected:
    // consider alias instead for this
    // and do I really want "pointer to member"?
    typedef float (NeuralNetPlayer::* evalFunction)(const board& bd);
    evalFunction equityEstimator;

    int callBackF(const board &b) override;
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
};

