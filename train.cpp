/* Written by Todd Doucet.  See file copyright.txt.
 */
#include <iostream>
#include <iomanip>

#include "game.h"
#include "playernet.h"
#include "human.h"

using std::cout;

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


/*
 * Command-line stuff
 */
void usage()
{
    cout << "usage: train  [flags] [playerOne] [playerTwo]\n";
    cout << "\t-d display moves.\n";
    cout << "\t-nTrials.\n";
    cout << "\t-Sseedval\n";
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
                cout << "alpha: " << alpha << "\n";
                break;

            case 'l':
                lambda = std::stof(argv[i] + 2);
                cout << "lambda: " << lambda << "\n";
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
        cout << "RNG using user-specified seed: " << user_seed << "\n";
        set_seed(user_seed);
    }
    else
        randomize_seed();
}

using namespace std;

int main(int argc, char *argv[])
{
    cmdline(argc, argv);
    setupRNG();

    NeuralNetPlayer whitePlayer("white", player_name[0]);
//  HumanPlayer whitePlayer("white");

    NeuralNetPlayer blackPlayer("black", player_name[1]);

    playoffSession(trials, whitePlayer, blackPlayer, display_moves);

    return 0;
}
