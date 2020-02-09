/* Written by Todd Doucet.  See file copyright.txt.
 */
#include <iostream>
#include <iomanip>

#include "game.h"
#include "playernet.h"
#include "human.h"
#include "cmdline.h"

using namespace std;

class AnnotatedGame : public Game
{
public:
    AnnotatedGame(Player& wh, Player& bl) : Game(wh, bl) {}

protected:
    void reportMove(board bd, moves mv) override
    {
        std::string s = moveStr(mv);
        cout << board::colorname(b.onRoll()) << " rolls "
             << bd.d1() << " " <<  bd.d2()
             << " and moves " << s << '\n';
    }
};

void playoffSession(int trials, Player& whitePlayer, Player& blackPlayer, bool verbose)
{
    AnnotatedGame game(whitePlayer, blackPlayer);

    int numGames;
    double whitePoints = 0.0;

    for (numGames = 1; numGames <= trials ; numGames++)
    {
        double white_eq = game.playGame(verbose);
        whitePoints += white_eq;

        std::ostringstream ss;

        ss << std::fixed << "Game " << numGames << ": "
           << std::setprecision(2) << std::setw(5) << white_eq << "... ";

        ss << "white equity/game = "
           << std::setprecision(3) << whitePoints/numGames
           << " (total "
           << std::setprecision(2) << whitePoints
           << ")\n";

        cout << ss.str();
    }
}

void setupRNG(uint64_t user_seed)
{
    if (user_seed != -1)
    {
        cout << "RNG using user-specified seed: " << user_seed << "\n";
        set_seed(user_seed);
    }
    else
        randomize_seed();
}

class cmdopts : public cmdline
{
public:
    int trials = 500;
    bool display_moves = false;
    bool explicit_seed = false;
    uint64_t user_seed = -1;

    cmdopts()
    {
        setopt('n', "--games", trials,        "number of trials.");
        setopt('s', "--seed",  user_seed,     "seed for random-number generator.");
        setopt('d',            display_moves, "display moves.");
    }

};

int main(int argc, char *argv[])
{
    string player_name[2];

    cmdopts opts;
    opts.parse(argc, argv);
    if (opts.ExtraArgs.size() == 1)
    {
        player_name[0] = "net.w";
        player_name[1] = "net.w";
    }
    else if (opts.ExtraArgs.size() == 2)
    {
        player_name[0] = opts.ExtraArgs[1];
        player_name[1] = "net.w";
    }
    else if (opts.ExtraArgs.size() == 3)
    {
        player_name[0] = opts.ExtraArgs[1];
        player_name[1] = opts.ExtraArgs[2];
    }

    setupRNG(opts.user_seed);

    cout << "white: " << player_name[0].c_str() << endl;
    cout << "black: " << player_name[1].c_str() << endl;

    NeuralNetPlayer whitePlayer("white", player_name[0].c_str());
//  HumanPlayer whitePlayer("white");

    NeuralNetPlayer blackPlayer("black", player_name[1].c_str());

    playoffSession(opts.trials, whitePlayer, blackPlayer, opts.display_moves);

    return 0;
}
