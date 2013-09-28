/*
 * Copyright (C) 1993, 2008 by Todd Doucet.  All Rights Reserved.
 */

#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//#include <iostream>
#include "console.h"

/*
 * Command-line stuff
 */
void usage()
{
	console << "usage: train [+s] [-s] [-bw][-?] [playerOne] [playerTwo]\n";
	console << "\t+s forces settling.\n";
	console << "\t-s disables settling.\n";
	console << "\t-d display moves.\n";
	console << "\t-w increment white ply level.\n";
	console << "\t-b increment black ply level.\n";
	console << "\t-aAlpha\n";
	console << "\t-lLambda\n";
	console << "\t-nTrials.\n";
	console << "\t-Sseedval\n";
	console << "\n";
	console << "\tBy default, settling is enabled for playoffs, disabled for training.\n";
	console << "\tWith one player, training occurs, with two players, a playoff occurs.\n";
	fatal("");
}

int numPlayers;
const char *player_name[2];

float alpha = 0.02f;
float lambda = 0.85f;
int trials = 500;
int isTraining;
int isServer = 0;
int display_moves;

unsigned long user_seed;
bool explicitSeed = false;

double getFloat(char *s)
{
	float d;
	int i = sscanf(s, "%f", &d);
	if (i == 0)
		fatal("error in getFloat()");
	return d;
}

double getInt(char *s)
{
	int d;
	int i = sscanf(s, "%d", &d);
	if (i == 0)
		fatal("error in getInt()");
	return d;
}

void cmdline(int argc, char *argv[])
{
	int i;
	for (i = 1; i < argc; i++){
		if (argv[i][0] == '-'){
			switch (argv[i][1]){
			case 'n':
				trials = (int) getInt(argv[i] + 2);
				break;
			case 'a':
				alpha = getFloat(argv[i] + 2);
				break;

			case 'l':
				lambda = getFloat(argv[i] + 2);
				break;

			case 'S':
				user_seed = (int) getInt(argv[i] + 2);
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
		} else if (argv[i][0] == '+'){
			switch(argv[i][1]){
			default:
				usage();
				break;
			}
		} else {
			if (numPlayers == 2){
				usage();
			}
			player_name[numPlayers] = argv[i];
			numPlayers++;
		}
	}

	if (numPlayers == 0){
		player_name[numPlayers++] = "net.w";
		isTraining = 1;
	} else if (numPlayers == 1){
		isTraining = 1;
	} else if (numPlayers == 2){
		isTraining = 0;
	} else {
		usage();
	}
}

/*
 *  Main routine. 
 */

#include "game.h"
#include "playernet.h"
#include "human.h"

void setupRNG()
{
	if (explicitSeed == true){
		debug_console << "RNG using user-specified seed: " << user_seed << "\n";
		set_seed(user_seed);
	} else {
		randomize_seed();
	}
}


int main(int argc, char *argv[])
{
	cmdline(argc, argv);
	setupRNG();

	NeuralNetPlayer *whitePlayer = new NeuralNetPlayer("white", player_name[0]);
//	Player *whitePlayer = new HumanPlayer("human");

	Player *blackPlayer = new NeuralNetPlayer("black", player_name[1]);
//	Player *blackPlayer = new NeuralNetLearner("black", player_name[1]);

	playoffSession(trials, whitePlayer, blackPlayer);

	return 0;
}
