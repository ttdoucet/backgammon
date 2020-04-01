/* Written by Todd Doucet.  See file copyright.txt.
 */
#pragma once

#include "game.h"

class HumanPlayer : public Player
{
public:
    HumanPlayer(const char *name): Player(name){}

    void prepareToPlay(const board& b) override;
    void finalEquity(float e) override;
    void chooseMove(const board& b, moves& choice) override;

protected:
    bool legalToMove(board fromBoard, board toBoard, moves& mv);
    void getUserBoard(board& bd);
    void parseUserMove(const board& fromBoard, board& toBoard, char *s);
};
