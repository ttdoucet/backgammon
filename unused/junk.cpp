#if 0
// This  is being moved to game.cpp.
struct player {
	char *name;
//	void init();
	virtual void init_play(){}  
	virtual void get_move(board &b, move &m) = 0;
	player(char *n) : name(n) {}
};


static double whitePoints = 0.0;

struct computer_player : public player {
	net *neural;
	computer_player(char *n, char *netname) : player(n) {
		neural = read_network(netname);
		neural->init_play();
		set_seed(neural->seed);  // hack
	}

	void init_play(){
		neural->init_play();
	}
	
	void get_move(board &b, move &m);
};




void computer_player::get_move(board &b, move &m)
{
	setNetwork(neural);
	if (isBearingOff(b)){
		selectMove(b, m, bearoffEquity);
	} else {
		selectMove(b, m, littleE);
	}
}



//computer_player *cplayer = new computer_player("zero-ply", "net.w");

player *white_player; // = cplayer;
player *black_player; // = cplayer;

player *thePlayer;

board b;

net *trainNet;
net *whiteNet, *blackNet;


/*  Training session stuff is being moved to the new Game/Player
 *  system.
 */


static void setPlayerForRoller(board &b)
{
	color_t roller = b.colorOnRoll();
	if (roller == white){
		thePlayer = white_player;
	} else if (roller == black){
		thePlayer = black_player;
	} else {
		tAssert(0);
	}
}


static void setNetForRoller(board &b)
{
	color_t roller = b.colorOnRoll();
	if (roller == white){
		setNetwork(whiteNet);
	} else if (roller == black){
		setNetwork(blackNet);
	} else {
		tAssert(0);
	}
}




static void initialReport(board &b)
{
	int d1 = b.d1();
	int d2 = b.d2();
	b.setDice(-1, -1);

	printf("Init, roller=%s, eq=%.2f: ", colorname(b.colorOnRoll()), littleE(b));
	b.setDice(d1, d2);
}


static void initTrainGame(board &b)
{
	trainNet->init_play();

	trainNet->learns(1);
	trainNet->init_learning(alpha, lambda);

	whiteNet = trainNet;
	blackNet = trainNet;
	
	setupGame(b);
	setNetForRoller(b);
    setPlayerForRoller(b);

	initialReport(b);


	// Now play the opening roll.
	move m;
	zeroPlySelect(b, m);
	b.applyMove(m);
	b.pickupDice();
    // When this exits, the first move has been played,
    // and the second player is on roll.

}

static int numGames = 0;

/*
 * This returns the (perhaps fractional) number of points
 * that white won this game.
 */

float observeFinal(board &b)
{
	int gStatus;
	float equity;

	if (gStatus = gameOver(b)){
		equity = (float) gStatus;
	} else {
		equity = bearoffEquity(b);
		if (display_moves){
			printf("    (settle)\n");
			// b.display_board(b.colorOnRoll());
		}
	}


		// We want white's equity.
	if (b.colorOnRoll() == black){
		equity = -equity;
	}

	if (trainNet){
			// Ignored when not learning.
		trainNet->observe_final(equity);
	}

	whitePoints += equity;
	numGames++;
	return equity;
}




	// We enable settling for playoffs to reduce variance.
	// For training, we allow settling or not depending on a cmdline flag.
static int willSettle(board &b)
{
	if (settle){
		return isBearingOff(b);
	} else {
		return 0;
	}
}




void dump_k(net *n)
{
	char fn[50];
	sprintf(fn, "n%ld.k", n->games_trained / 1000L);
	n->dump_network(fn, 0);
}

void updateNetwork()
{
	trainNet->games_trained++;
	trainNet->incorporate_learning();


	if ((trainNet->games_trained % 1000) == 0L){
		printf("\n\nbacking up net.w at games_trained = %ld\n", trainNet->games_trained);
		trainNet->dump_network("net.w", 0);
	}


	if ((trainNet->games_trained % 10000) == 0L){
		printf("\n\ndumping net at games_trained = %ld\n", trainNet->games_trained);
		dump_k(trainNet);
		printf("done.\n");
	}

}


float playTrainGame(board &b)
{
	initTrainGame(b);

	float whiteEquity = playPosition(b);
	updateNetwork();
	return whiteEquity;
}

unsigned long user_seed;


void trainingSession(char *netname)
{
	printf("trainingSession(%s), settle = %d\n", netname, settle);

	white_player = new computer_player("zero-ply", netname);
	black_player = white_player;

	trainNet = read_network(netname);
	setNetwork(trainNet);
	set_seed(user_seed ? user_seed : trainNet->seed);


	printf("setting the training sessions seed to %ld (from network)\n", trainNet->seed);
	printf("alpha=%f, lambda = %f\n", alpha, lambda);


	int i;
	for (i = 0; 1; i++){
		float whiteEquity = playTrainGame(b);
		printf("%d: white gets: %5.2f, cumulative: %.2f\n",
			   i, whiteEquity, whitePoints
		);
	}
}

/*  Training session stuff is being moved to the new Game/Player
 *  system.
 */


/* Copied from mbgview.cpp */
float onePlyEquity(board b, mEntry *mEnt)
{
	b.applyMove(mEnt->m);
//	b.pickupDice();
	return -bigE(b);
}



/* Copied from mbgview.cpp */
static void partialOnePlySelect(board &b, move &onePlyMove)
{

	candidateMoves candidates(b);
	b.play(candidates);

	candidates.sortZeroPly();

	int limit = 8;
	for (int j = 0; j < candidates.size(); j++){
		mEntry *m = candidates.getEntry(j);
		if (j < limit){
			m->eqOnePly = onePlyEquity(b, m);
		} else {
			m->eqOnePly = -10.0F;
		}
	}
	candidates.sortOnePly();

	mEntry *entry = candidates.getEntry(0);
//	printf("partialOnePlySelect equity: %f\n", entry->eqOnePly);
	onePlyMove = entry->m;
}



/*
	This chooses the best 1-ply move, after picking an
	appropriate equity estimator to use throughout the
	search.  It is important not to mix and match
	equity estimators during a search.
*/

static void zeroPlySelect(board &b, move &m)
{
	if (isBearingOff(b)){
		selectMove(b, m, bearoffEquity);
	} else {
		selectMove(b, m, littleE);
	}
}


// apparently experimental
void chooseIt(board &b, move &m)
{
	int mover = b.colorOnRoll();

	float eq;
	// ???
	if (isTraining){
		zeroPlySelect(b, m);
	} else {
		int pl = plyLevelFor(b.colorOnRoll());
		switch(pl){
			case 1:
				zeroPlySelect(b, m);
				break;
			case 2:
				partialOnePlySelect(b, m);
				break;

			case 3:
				eq = select_move(b, 1, m);
//				printf("case 3 eq: %f\n", eq);
				break;
				
			case 4:
				select_move(b, 2, m);
				break;

			case 5:
				select_move(b, 0, m);
				break;
			default:
				printf("nyi: plylevel=%d\n", pl);
				exit(0);
		}
	}
	if (display_moves){
		reportMove(b, m);
	}
}

int blackPlyLevel = 1;
int whitePlyLevel = 1;

int plyLevelFor(color_t color)
{
	if (color == white)
		return whitePlyLevel;
	else
		return blackPlyLevel;
}







#endif
