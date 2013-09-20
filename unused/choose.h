#pragma once

typedef float equityEstimator(board &board);

equityEstimator littleE;
equityEstimator bearoffEquity;
equityEstimator observeE;
equityEstimator bigE;
equityEstimator neural_2ply;


void setNetwork(net *n);


int isBearingOff(board &nb);
float selectMove(board &fromBoard, move &m, equityEstimator equity);

float quickPlay(board &board, move &m);
void slowPlay(board &board, move &m);



float zeroPlyEquity(color_t color, board &nb);

int chooseMove(board &start, move *m);

// I really hate this.
struct mEntry {
	float eqZeroPly;
	float eqOnePly;
	int zeroPlyRank;
	int onePlyRank;
	move m;
};


class candidateMoves : public callBack {
		int nCallB;
		int nitems;
		mEntry *mEnt;
		equityEstimator *equity;

		void init(){
		  nitems = 0;
		  mEnt = new mEntry[2500];	// I can probably do better than this.
		  equity = littleE;
		}
			  
	public:
		int callBackF(board &b){
			float e = -equity(b);
			addEntry(m, e);
			return 0;
		}
		void clear(){
			nitems = 0;
		}
		mEntry *getEntry(int i){
			return mEnt + i;
		}
		void addEntry(move &mv, float e1) {
			mEntry &entry = mEnt[nitems];
			entry.eqZeroPly = e1;
			entry.eqOnePly = -3.0f;
			entry.m = mv;
			nitems++;
		}
		int size() {
			return nitems;
		}

		void sortZeroPly();
		void sortOnePly();
		void setEvaluator(equityEstimator *e){
			equity = e;
			nitems = 0;
		}

		candidateMoves() : nCallB(0){
			init();
		}

		candidateMoves(board &b) : nCallB(0){
			init();
			if (isBearingOff(b)){
				equity = bearoffEquity;
			} else {
				equity = littleE;
			}
		}

	  ~candidateMoves() {
		  delete[] mEnt;
	  }

};

int partialOnePly(board &b, candidateMoves &candidates, int limit);
int getTopCandidates(board &b, mEntry *m, equityEstimator e, int limit);


