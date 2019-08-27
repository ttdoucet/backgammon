#pragma once

#include "game.h"

class NeuralNetPlayer : public Player, public callBack
{
public:
    NeuralNetPlayer(const char *player, const char *netname);
    void prepareToPlay() override;
    void chooseMove(const board& b, moves& choice) override;
    void finalEquity(double e) override
    {
    }

protected:
    typedef float (NeuralNetPlayer::* evalFunction)(const board& bd);
    NeuralNetPlayer::evalFunction equityEstimator;

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

    static int gameOver(const board &bd);
    static int win_check(const board &nb, color_t side);
    static int gammon_check(const board &nb, color_t winner);
};

