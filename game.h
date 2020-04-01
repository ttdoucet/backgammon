/* Written by Todd Doucet.  See file copyright.txt.
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
    virtual void prepareToPlay(const board& b) { }
    virtual void presentBoard(const board& b) {}
    virtual void chooseMove(const board& b, moves& choice) = 0;
    virtual void finalEquity(float e) { }
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
        whitePlayer.prepareToPlay(b);
        blackPlayer.prepareToPlay(b);

        setupGame();

        moves m;
        while( gameOver(b) == false )
        {
            playerOnRoll().chooseMove(b, m);

            if (verbose)
                reportMove(b, m);
            applyMove(b, m);

            playerOnRoll().presentBoard(b);
            b.setDice( throw_die(), throw_die() );
        }

        auto white_equity = score(b, white);
        whitePlayer.finalEquity( white_equity);
        blackPlayer.finalEquity(-white_equity);

        return white_equity;
    }

    void setupGame()
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
        playerOnRoll().presentBoard(b);
        b.setDice(dwhite, dblack);
    }

protected:
    Player& whitePlayer;
    Player& blackPlayer;
    board b;

    virtual void reportMove(board bd, moves mv) { }

    Player& playerOnRoll() const
    {
        if (b.onRoll() == white)
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
