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
    virtual void prepareToPlay() { }
    virtual void finalEquity(double e) { }
    virtual void chooseMove(const board& b, moves& choice) = 0;
    virtual void presentBoard(const board&b) {}
    virtual ~Player(){}
};

inline bool gameOver(const board& bd)
{
    return (bd.checkersOnPoint(white, 0) == 15) || (bd.checkersOnPoint(black, 0) == 15);
}

inline int score(const board& b, color_t color)
{
    color_t winner;
    int equity;

    if (b.checkersOnPoint(color, 0) == 15)
    {
        winner = color;
        equity = 1;
    }
    else
    {
        winner = opponentOf(color);
        equity = -1;
    }
    const color_t loser = opponentOf(winner);
    bool gammon = (b.checkersOnPoint(loser, 0) == 0);
    bool backgammon = (gammon && b.highestChecker(loser) > 18);

    if (backgammon)
        equity *= 3;
    else if (gammon)
        equity *= 2;

    return equity;
}

class Game
{
public:
    Game(Player& wh, Player& bl) : whitePlayer(wh), blackPlayer(bl)
    {
        b.clearBoard();
    }

    // Returns the equity of white at the end of the game.
    double playGame(bool verbose)
    {
        whitePlayer.setColor(white);
        blackPlayer.setColor(black);

        whitePlayer.prepareToPlay();
        blackPlayer.prepareToPlay();

        setupGame();
        moves m;

        while( gameOver(b) == false )
        {
            playerFor(b.onRoll()).chooseMove(b, m);

            if (verbose)
                reportMove(b, m);
            applyMove(b, m);
    //      if (verbose)
    //          display_board(b, white);
            b.setDice( throw_die(), throw_die() );
        }

        return score(b, white);
    }

    Player& setupGame()
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

protected:
    Player& whitePlayer;
    Player& blackPlayer;
    board b;

    virtual void reportMove(board bd, moves mv) { }

    Player& playerFor(color_t color) const
    {
        if (color == white)
            return whitePlayer;
        else
            return blackPlayer;
    }

    void both(int pt, int num)
    {
        for(int i = 0; i < num; i++)
        {
            b.moveChecker(white, 0, pt);
            b.moveChecker(black, 0, pt);
        }
    }
};
