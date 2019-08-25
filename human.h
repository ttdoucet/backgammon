#pragma once

#include "game.h"

class HumanPlayer : public Player
{
public:
    HumanPlayer(const char *name): Player(name){}
    void prepareToPlay();
    void finalEquity(double e);

    void chooseMove(const board& b, moves& choice);

protected:
    bool legalToMove(board fromBoard, board toBoard, moves& mv);
    void getUserBoard(board& bd);
    void parseUserMove(const board& fromBoard, board& toBoard, char *s);

};


