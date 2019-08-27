/*
 * Copyright (C) 1993, 2008 by Todd Doucet.  All Rights Reserved.
 */

#include <cstdlib>
#include "game.h"
#include "playernet.h"
#include "human.h"
#include "console.h"

/*
 * Command-line stuff
 */
void usage()
{
    console << "usage: train  [flags] [playerOne] [playerTwo]\n";
    console << "\t-d display moves.\n";
    console << "\t-nTrials.\n";
    console << "\t-Sseedval\n";
    std::exit(1);
}

int numPlayers;
const char *player_name[2];

float alpha = 0.02f;  // not hooked up
float lambda = 0.85f; // not hooked up
int trials = 500;
int isTraining;  // not hooked up
int display_moves;  // hack

unsigned long user_seed;
bool explicitSeed = false;

void cmdline(int argc, char *argv[])
{
    int i;
    for (i = 1; i < argc; i++)
    {
        if (argv[i][0] == '-')
        {
            switch (argv[i][1])
            {
            case 'n':
                trials = std::stoi(argv[i] + 2);
                break;
            case 'a':
                alpha = std::stof(argv[i] + 2);
                console << "alpha: " << alpha << "\n";
                break;

            case 'l':
                lambda = std::stof(argv[i] + 2);
                console << "lambda: " << lambda << "\n";
                break;

            case 'S':
                user_seed = std::stoi(argv[i] + 2);
                explicitSeed = true;
                break;

            case 'd':
                display_moves++;
                break;

            default:
            case '?':
                usage();
                break;
            }
        }
        else
        {
            if (numPlayers == 2)
                usage();
            player_name[numPlayers] = argv[i];
            numPlayers++;
        }
    }

    if (numPlayers == 0)
    {
        player_name[numPlayers++] = "net.w";
        isTraining = 1;
    }
    else if (numPlayers == 1)
        isTraining = 1;
    else if (numPlayers == 2)
        isTraining = 0;
    else
        usage();
}

void setupRNG()
{
    if (explicitSeed == true)
    {
        console << "RNG using user-specified seed: " << user_seed << "\n";
        set_seed(user_seed);
    }
    else
        randomize_seed();
}

#include "stopwatch.h"
extern stopwatch mtimer, ftimer, stimer, htimer;
stopwatch timer;

#include <iomanip>
using namespace std;

int main(int argc, char *argv[])
{
    cmdline(argc, argv);
    setupRNG();

    NeuralNetPlayer whitePlayer("white", player_name[0]);
//  HumanPlayer whitePlayer("white", player_name[0]);

    NeuralNetPlayer blackPlayer("black", player_name[1]);
//  HumanPlayer blackPlayer("black", player_name[1]);

    timer.start();
    playoffSession(trials, whitePlayer, blackPlayer, display_moves);
    timer.stop();

#if 0
    cerr << "ftimer: " << setw(12) << ftimer.elapsed() << " ns\n";
    cerr << "htimer: " << setw(12) << htimer.elapsed() << " ns\n";
    cerr << "\n";
    cerr << "mtimer: " << setw(12) << mtimer.elapsed() << " ns\n";
    cerr << "stimer: " << setw(12) << stimer.elapsed() << " ns\n";
    cerr << "\n";
    cerr << " timer: " <<  setw(12) << timer.elapsed() << " ns\n";

    cerr << "\n";
    cerr << "ftimer count: " << ftimer.count() << "\n";
    cerr << "mtimer count: " << mtimer.count() << "\n";
#endif

    return 0;
}
