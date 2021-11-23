/* Written by Todd Doucet. */
#include <iostream>
#include <iomanip>
#include <fstream>

#include "game.h"
#include "learnernet.h"
#include "cmdline.h"

using namespace std;

class TrainingOptions : public cmdline
{
public:
    // command-line options
    int games = 100000;
    int batchsize = 1;
    int every = 1000;
    uint64_t user_seed = -1;

    string wlearn_fn = "white.w";
    string blearn_fn = "";
    string bplay_fn =  "";

    string output_white = "";
    string output_black = "";

    float alpha  = 0.02f / 36;
    float alpha_end = -1;
    float lambda = 0.85f;
    double decay = 1.0;
    float momentum = 0;

    bool wdual = false;
    bool bdual = false;

    TrainingOptions()
    {
        setopt('n', "--games",   games, "games to play");
        setopt("--batch-size", batchsize, "games per batch (default: "
               + to_string(batchsize) + ")\n");

        setopt('w', "--white-learner", wlearn_fn, "file for white learner");
        setopt('b', "--black-player",  bplay_fn, " file for black player");
        setopt('B', "--black-learner", blearn_fn, "file for black learner\n");
        setopt('d', "--white-dual",    wdual,   "  white observes both sides");
        setopt('D', "--black-dual",    bdual,   "  black observes both sides\n");

        setopt('o', "--out-white", output_white, "file to save white after training.");
        setopt('O', "--out-black", output_black, "file to save black after training.\n");

        setopt('a', "--alpha",   alpha,
               "learning rate at start (default: " + to_string(alpha) + ")"
              );

        setopt('A', "--alpha-end",   alpha_end,
               "learning rate at end (default: alpha)"
              );

        setopt('l', "--lambda",  lambda,
               "temporal discount (default: " + to_string(lambda) + ")\n"
              );

	setopt('p', "--momentum", momentum,
               "factor to exponentiallly average gradient (default: " + to_string(momentum) + ")\n"
              );

        setopt('e', "--every",   every, "Report every n games.");
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

        if (alpha_end < 0)
            alpha_end = alpha;

        if (games > 1)
            decay = std::pow( alpha_end / alpha , 1.0 / (games - 1) );
    }
};

class TrainingSession
{
public:
    TrainingSession(const TrainingOptions& options)
        : opts{options}
    {
    }

    void run()
    {
        BgNetReader bgr;

        if (file_exists(opts.output_white))
        {
            cout << "File already exists: " << opts.output_white << ", skipping.\n";
            return;
        }

        unique_ptr<BgNet> white_net, black_net;
        unique_ptr<PlayerNet> white_player, black_player;
        
        white_net = bgr.read(opts.wlearn_fn);
        white_player = learner_for(*white_net, opts);

        if (opts.blearn_fn.empty() == false)
        {
            cout << "train: "
                 << name_for(opts.wlearn_fn, opts.wdual)
                 << " against learning "
                 << name_for(opts.blearn_fn, opts.bdual)
                 << "\n";
            black_net = bgr.read(opts.blearn_fn);
            black_player = learner_for(*black_net, opts);
        }
        else if (opts.bplay_fn.empty() == false)
        {
            cout << "train: "
                 << name_for(opts.wlearn_fn, opts.wdual)
                 << " against fixed " << opts.bplay_fn
                 << "\n";
            black_net = bgr.read(opts.bplay_fn);
            black_player = player_for(*black_net);
        }
        else
        {
            cout << "self-play: "
                 << name_for(opts.wlearn_fn, opts.wdual)
                 << "\n";
            black_player = player_for(*white_net);
        }

        train(*white_player, *black_player);

        string outfile = opts.output_white.empty() ? opts.wlearn_fn : opts.output_white;
        white_net->writeFile(outfile);
        cout << "white saved: " << outfile << "\n";

        if (opts.blearn_fn.empty() == false)
        {
            string outfile = opts.output_black.empty() ? opts.blearn_fn : opts.output_black;
            black_net->writeFile(outfile);
            cout << "black saved: " << outfile << "\n";
        }
    }

private:
    const TrainingOptions& opts;

    static bool file_exists(string filename)
    {
        ifstream f(filename);
        return f.good();
    }

    static void report(int numGames, double whitePoints)
    {
                ostringstream ss;

                ss << "white equity/game = "
                   << setprecision(3) << whitePoints/numGames
                   << " (total "
                   << setprecision(2) << whitePoints
                   << ")\n";

                cout << ss.str();
    }

    string name_for(string fn, bool dual)
    {
        ostringstream ss;
        ss << fn;
        if (dual)
            ss << " (dual)";
        return ss.str();
    }

    void train(PlayerNet& whitePlayer,
               PlayerNet& blackPlayer)
    {
        Game game(whitePlayer, blackPlayer);

        int numGames;
        double whitePoints = 0.0;

        for (numGames = 1; numGames <= opts.games ; numGames++)
        {
            double white_eq = game.playGame();
            whitePoints += white_eq;

            if (opts.every && !(numGames % opts.every))
            {
                ostringstream ss;
                ss << fixed << "Game: " << numGames << ": "
                   << setprecision(2) << setw(5) << white_eq << "... ";
                cout << ss.str();

                report(numGames, whitePoints);
            }
        }

        cout << "Games: " << (numGames - 1)  << ": ";
        report(numGames, whitePoints);
    }

    unique_ptr<PlayerNet> learner_for(BgNet& nn, const TrainingOptions& opts)
    {
        if (auto p = dynamic_cast<netv3*>(&nn))
	    return make_unique<Learner<netv3> > (*p, opts.alpha, opts.lambda, opts.wdual, opts.decay, opts.batchsize, opts.momentum);

        if (auto p = dynamic_cast<netv3tr*>(&nn))
	    return make_unique<Learner<netv3tr> > (*p, opts.alpha, opts.lambda, opts.wdual, opts.decay, opts.batchsize, opts.momentum);

        // experimental, in flux
        if (auto p = dynamic_cast<netv5*>(&nn))
	    return make_unique<Learner<netv5> > (*p, opts.alpha, opts.lambda, opts.wdual, opts.decay, opts.batchsize, opts.momentum);

        if (auto p = dynamic_cast<Fc_Sig_H60_I5*>(&nn))
	    return make_unique<Learner<Fc_Sig_H60_I5> > (*p, opts.alpha, opts.lambda, opts.wdual, opts.decay, opts.batchsize, opts.momentum);



        if (auto p = dynamic_cast<Fc_Sig_H60_I3*>(&nn))
	    return make_unique<Learner<Fc_Sig_H60_I3> > (*p, opts.alpha, opts.lambda, opts.wdual, opts.decay, opts.batchsize, opts.momentum);

        if (auto p = dynamic_cast<Fc_Sig_H60_I3tr*>(&nn))
	    return make_unique<Learner<Fc_Sig_H60_I3tr> > (*p, opts.alpha, opts.lambda, opts.wdual, opts.decay, opts.batchsize, opts.momentum);


        if (auto p = dynamic_cast<Fc_Sig_H90_I3*>(&nn))
            return make_unique<Learner<Fc_Sig_H90_I3> > (*p, opts.alpha, opts.lambda, opts.wdual, opts.decay, opts.batchsize, opts.momentum);

        if (auto p = dynamic_cast<Fc_Sig_H90_I3tr*>(&nn))
            return make_unique<Learner<Fc_Sig_H90_I3tr> > (*p, opts.alpha, opts.lambda, opts.wdual, opts.decay, opts.batchsize, opts.momentum);

        if (auto p = dynamic_cast<Fc_Sig_H120_I3*>(&nn))
            return make_unique<Learner<Fc_Sig_H120_I3> > (*p, opts.alpha, opts.lambda, opts.wdual, opts.decay, opts.batchsize, opts.momentum);

        if (auto p = dynamic_cast<Fc_Sig_H120_I3tr*>(&nn))
            return make_unique<Learner<Fc_Sig_H120_I3tr> > (*p, opts.alpha, opts.lambda, opts.wdual, opts.decay, opts.batchsize, opts.momentum);


        // Support learning in additional neural net players here. . .

        throw runtime_error("Network not yet supported: " + nn.netname());
    }

    unique_ptr<PlayerNet> player_for(BgNet& nn)
    {
      return make_unique<PlayerNet>(nn);
    }
};

int main(int argc, char *argv[])
{
    TrainingOptions opts;
    opts.parse(argc, argv);
    TrainingSession{opts}.run();
}
