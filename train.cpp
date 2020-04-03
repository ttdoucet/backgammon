/* Written by Todd Doucet.  See file copyright.txt.
 */
#include <iostream>
#include <iomanip>

#include "game.h"
#include "playernet.h"
#include "human.h"
#include "cmdline.h"

using namespace std;

class cmdopts : public cmdline
{
public:
    int games = 1000;
    bool verbose = false;
    int every = 0;
    uint64_t user_seed = -1;
    string whitenet = "white.w";
    string blacknet = "black.w";
    float alpha = 0.01f;
    float lambda = 0.85f;

    cmdopts()
    {
        setopt('n', "--games",   games,       "number of games.");

        setopt('w', "--white",   whitenet,    "Filename for white network, default white.w");
        setopt('b', "--black",   blacknet,    "Filename for black network, default black.w");

        setopt('a', "--alpha",   alpha,       "Learning rate.");
        setopt('l', "--lambda",  lambda,      "Temporal discount.");

        setopt('e', "--every",   every,       "Report every n games.");

    }
};

cmdopts opts;

static void report(int numGames, double whitePoints)
{
            ostringstream ss;

            ss << "white equity/game = "
               << std::setprecision(3) << whitePoints/numGames
               << " (total "
               << std::setprecision(2) << whitePoints
               << ")\n";

            cout << ss.str();
}

static void trainingSession(Learner& whitePlayer, NeuralNetPlayer& blackPlayer)
{
    Game game(whitePlayer, blackPlayer);

    int numGames;
    double whitePoints = 0.0;

    for (numGames = 1; numGames <= opts.games ; numGames++)
    {
        double white_eq = game.playGame();
        whitePoints += white_eq;

        blackPlayer = (NeuralNetPlayer) whitePlayer;

        if (opts.every && !(numGames % opts.every))
        {
            ostringstream ss;
            ss << std::fixed << "Game: " << numGames << ": "
               << std::setprecision(2) << std::setw(5) << white_eq << "... ";
            cout << ss.str();

            report(numGames, whitePoints);
        }
    }

    cout << "Games: " << (numGames - 1)  << ": ";
    report(numGames, whitePoints);
}

int main(int argc, char *argv[])
{
    opts.parse(argc, argv);

    cout << "white: " << opts.whitenet << endl;
    cout << "black: " << opts.blacknet  << endl;

    Learner         whitePlayer("white", opts.whitenet, opts.alpha, opts.lambda);
    // Learner         blackPlayer("black", opts.blacknet, opts.alpha, opts.lambda);
    NeuralNetPlayer blackPlayer("black", opts.blacknet);

    trainingSession(whitePlayer, blackPlayer);


    whitePlayer.save("white.w");
//  blackPlayer.save("black.w");

    return 0;
}
