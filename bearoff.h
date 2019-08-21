struct bear_off {
	unsigned int board;
	float expectation;
	float f[15];
};



	// Not necessarily exported by the bearoff dll.  But maybe they will be
float bearoffExpectation(unsigned int b);
double fewerEq(unsigned int b, int n);
double greaterEq(unsigned int b, int n);
double exact(unsigned int b, int n);

	// The routine exported by the dll.
double bearoffEquity(unsigned int onRoll, unsigned int notOnRoll);

// This is really a higher-level thing, and beints someplace else, I think.
// double bearoffEquity(board &b);
