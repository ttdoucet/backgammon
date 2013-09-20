struct bear_off {
	unsigned int board;
	float expectation;
	float f[15];

/*
    bear_off(){
        board = 15;
        expectation = 0;
        for (int i = 0; i < 15; i++)
            f[i] = 0;
    }
*/
};

//unsigned long board_to_32(board &b, color_t c);
//void board_from_32(board &board, unsigned long b, color_t color);

int board_to_index(unsigned int b);

	// Not necessarily exported by the bearoff dll.  But maybe they will be
float bearoffExpectation(unsigned int b);
double fewerEq(unsigned int b, int n);
double greaterEq(unsigned int b, int n);
double exact(unsigned int b, int n);

	// The routine exported by the dll.
double bearoffEquity(unsigned int onRoll, unsigned int notOnRoll);

// This is really a higher-level thing, and beints someplace else, I think.
// double bearoffEquity(board &b);
