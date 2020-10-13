/* Written by Todd Doucet.  See file copyright.txt.
 */
#include <iostream>
#include <iomanip>
#include <fstream>

#include "game.h"
#include "playernet.h"
#include "human.h"
#include "cmdline.h"

using namespace std;

class TrainingOptions : public cmdline
{
public:
    // command-line options
    int games = 100000;
    int every = 100;
    uint64_t user_seed = -1;

    string wlearn_fn = "white.w";
    string blearn_fn = "";
    string bplay_fn =  "";

    string output_white = "";
    string output_black = "";

    float alpha  = 0.02f / 36;
    float lambda = 0.85f;

    bool wdual = false;
    bool bdual = false;

    TrainingOptions()
    {
        setopt('n', "--games",   games, "number of games.");
        setopt('e', "--every",   every, "Report every n games.");

        setopt('w', "--white-learner", wlearn_fn, "file for white learner");
        setopt('b', "--black-player",  bplay_fn, " file for black player");
        setopt('B', "--black-learner", blearn_fn, "file for black learner");
        setopt('d', "--white-dual",    wdual,   "  white observes both sides");
        setopt('D', "--black-dual",    bdual,   "  black observes both sides");
        setopt('o', "--out-white", output_white, "file to save white after training.");
        setopt('O', "--out-black", output_black, "file to save black after training.");

        setopt('a', "--alpha",   alpha,
               "learning rate (default: " + to_string(alpha) + ")"
              );

        setopt('l', "--lambda",  lambda,
               "temporal discount (default: " + to_string(lambda) + ")"
              );
    }

    virtual void usage(const string& s=""s, bool terminate=true) override
    {
        cout << "Train one or two neural nets to play backgammon.\n\n";
        cmdline::usage(s, false);
        cout << "\n";
        cout << "White always learns and the default filename is white.w.\n";
        cout << "By default, white learns through self-play. Optionally,\n";
        cout << "either a black player or a black learner may be specified.\n\n";

        if (terminate)
            exit(1);
    }

    void parse(int argc, char *argv[]) override
    {
        cmdline::parse(argc, argv);

        if (ExtraArgs.size() > 1)
            usage("unrecognized option: " + ExtraArgs[1]);

        if (output_black.empty() && !blearn_fn.empty())
            output_black = blearn_fn;

        if (bdual && blearn_fn.empty())
            usage("no black learner specified for dual learning");

    }

};


template<TrainableEquityEstimator Estimator>
class TrainingSession
{
public:
    TrainingSession(const TrainingOptions& options)
        : opts{options}
    {
    }

    int run()
    {
        if (file_exists(opts.output_white))
        {
            cout << "File already exists: " << opts.output_white << ", skipping.\n";
            return 0;
        }

        Learner<Estimator> whitePlayer(opts.wlearn_fn, opts.alpha, opts.lambda, opts.wdual);

        if (opts.bplay_fn.empty() && opts.blearn_fn.empty())
        {
            cout << "self-play: "
                 << name_for(opts.wlearn_fn, opts.wdual)
                 << "\n";
            train_selfplay(whitePlayer);
        }
        else if (opts.blearn_fn.empty())
        {
            cout << "train: "
                 << name_for(opts.wlearn_fn, opts.wdual)
                 << " against fixed " << opts.bplay_fn
                 << "\n";
            NeuralNetPlayer<Estimator> blackPlayer(opts.bplay_fn);
            train_against(whitePlayer, blackPlayer);
        }
        else
        {
            cout << "train: "
                 << name_for(opts.wlearn_fn, opts.wdual)
                 << " against learning "
                 << name_for(opts.blearn_fn, opts.bdual)
                 << "\n";
            Learner<Estimator> blackPlayer(opts.blearn_fn, opts.alpha, opts.lambda, opts.bdual);
            train_against(whitePlayer, blackPlayer);
            blackPlayer.save(opts.output_black);
            cout << "black saved: " << opts.output_black << "\n";
        }

        string outfile = opts.output_white.empty() ? opts.wlearn_fn : opts.output_white;
        whitePlayer.save(outfile);
        cout << "white saved: " << outfile << "\n";

        return 0;
    }

private:
    const TrainingOptions& opts;

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

    static bool file_exists(string filename)
    {
        ifstream f(filename);
        return f.good();
    }

    string name_for(string fn, bool dual)
    {
        ostringstream ss;
        ss << fn;
        if (dual)
            ss << " (dual)";
        return ss.str();
    }

    void train(Learner<Estimator>& whitePlayer,
               NeuralNetPlayer<Estimator>& blackPlayer,
               bool self_play = true,
               bool black_learns = false)
    {
        Game game(whitePlayer, blackPlayer);

        int numGames;
        double whitePoints = 0.0;

        for (numGames = 1; numGames <= opts.games ; numGames++)
        {
            double white_eq = game.playGame();
            whitePoints += white_eq;

            if (self_play)
                blackPlayer = static_cast<NeuralNetPlayer<Estimator> >(whitePlayer);

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

    void train_selfplay(Learner<Estimator>& whitePlayer)
    {
        NeuralNetPlayer<Estimator> blackPlayer = static_cast<NeuralNetPlayer<Estimator> >(whitePlayer);
        train(whitePlayer, blackPlayer, true);
    }

    void train_against(Learner<Estimator>& whitePlayer, NeuralNetPlayer<Estimator>& blackPlayer)
    {
        train(whitePlayer, blackPlayer, false);
    }
};

int main(int argc, char *argv[])
{
    TrainingOptions opts;
    opts.parse(argc, argv);
    return TrainingSession<netv3>(opts).run();
}
