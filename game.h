/*
 *  Game and Player data types.
 */

#pragma once

#include "move.h"
#include "random.h"

class Player
{
protected:
    const char *playerName;
    color_t myColor;

public:
    Player(const char *nameOfPlayer) : playerName(nameOfPlayer) {};
    void setColor(color_t clr){ myColor = clr; }
    virtual void prepareToPlay() = 0;
    virtual void finalEquity(double e) = 0;
    virtual void chooseMove(const board& b, moves& choice) = 0;
    virtual void presentBoard(const board&b) {}
    virtual ~Player(){}
};

void playoffSession(int trials, Player& wh, Player& bl);

class Game
{
public:
    Game(Player& wh, Player& bl);

    // Returns the equity of white at the end of the game.
    double playGame(bool verbose = false);

protected:
    virtual void reportMove(board bd, moves mv) { }

    Player& whitePlayer;
    Player& blackPlayer;
    board b;

    color_t winningColor() const
    {
        if (b.checkersOnPoint(white, 0) == 15)
            return white;
        else
            return black;
    }

    Player *playerFor(color_t color) const
    {
        if (color == white)
            return &whitePlayer;
        else
            return &blackPlayer;
    }

    bool gameOver() const
    {
        return (b.checkersOnPoint(white, 0) == 15) || (b.checkersOnPoint(black, 0) == 15);
    }

    void both(int pt, int num)
    {
        for(int i = 0; i < num; i++)
        {
            b.moveChecker(white, 0, pt);
            b.moveChecker(black, 0, pt);
        }
    }

    Player *setupGame()
    {
        b.clearBoard();
        // Set up the checkers.
        both(24, 2);
        both(13, 5);
        both(8, 3);
        both(6, 5);

        // See who plays first.
        int dwhite = 0, dblack = 0;
        while (dwhite == dblack)
        {
            dwhite = throw_die();
            dblack = throw_die();
        }
        b.setRoller( (dwhite > dblack) ? white : black);
        b.setDice(dwhite, dblack);
        return playerFor(b.onRoll());
    }
};

class AnnotatedGame;


