/* Written by Todd Doucet. */
#pragma once
#include "move.h"
#include "random.h"

class Player
{
protected:

public:
    Player() {};
    virtual void prepareToPlay() { }
    virtual void presentBoard(const board& b, bool me_on_roll) {}
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
    Game(Player& wh, Player& bl, uint64_t seed = -1)
        : whitePlayer(wh),
          blackPlayer(bl),
          die(seed)
    {
        b.clearBoard();
    }

    // Returns the equity of white at the end of the game.
    int playGame()
    {
        whitePlayer.prepareToPlay();
        blackPlayer.prepareToPlay();

        setupGame();

        moves m;
        while( gameOver(b) == false )
        {
            playerOnRoll().chooseMove(b, m);

            reportMove(b, m);
            applyMove(b, m);

            whitePlayer.presentBoard(b, b.onRoll() == white);
            blackPlayer.presentBoard(b, b.onRoll() == black);

            b.setDice( die.roll(), die.roll() );
        }

        auto white_equity = score(b, white);
        whitePlayer.finalEquity( white_equity);
        blackPlayer.finalEquity(-white_equity);

        return white_equity;
    }

    void setupGame()
    {
        // Set up the checkers.
        b.clearBoard();
        both(24, 2);
        both(13, 5);
        both(8, 3);
        both(6, 5);

        // See who plays first.
        int dwhite = 0, dblack = 0;
        while (dwhite == dblack)
        {
            dwhite = die.roll();
            dblack = die.roll();
        }

        if (dwhite > dblack)
        {
            b.setRoller(white);
            b.setDice(dwhite, dblack);
        }
        else
        {
            b.setRoller(black);
            b.setDice(dblack, dwhite);
        }
    }

protected:
    Player& whitePlayer;
    Player& blackPlayer;
    board b;
    RNG_die die;

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
