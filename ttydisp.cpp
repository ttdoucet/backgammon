#include <stdio.h>
#include "board.h"
#include "console.h"

static void display_checker(color_t color, int level, int n)
{
	char buf[80];
	char ch =  (color == white) ? 'O' : 'X';

	if (level == 1 && n > 5)
		sprintf(buf, " %-2d", n - 4);
	else if (n >= level)
		sprintf(buf, " %c ", ch);
	else
		sprintf(buf, "   ");
	console << buf;
}


static void display_bar(const board &b, color_t color, int level)
{
	console << " |";
	display_checker(color, level, b.checkersOnBar(color));
	console << "| ";
}

static void display_point(const board& b, color_t color, int pt, int level)
{
	int n;

	if ( (n = b.checkersOnPoint(color, pt)) >= level)
		display_checker(color, level, n);
	else  
		display_checker(opponentOf(color), level,
				b. checkersOnPoint(opponentOf(color), opponentPoint(pt)));
}
	

void display_board(const board &b, color_t color)
{
	int i, j;
	
	console << "\n";
	console << "  24 23 22 21 20 19        18 17 16 15 14 13\n";
	console << "+---------------------------------------------+\n";
	for (j = 1; j <= 5; j++){
		console << "| ";
		for (i = 24; i > 18; i--)
			display_point(b, color, i, j);
		display_bar(b, color, j);
		for (i = 18; i > 12; i--)
			display_point(b, color, i, j);
		console << " |\n";
	}

	console << "|                    |   |                    |\n";
	for (j = 5; j >= 1; j--){
		console << "| ";
		for (i = 1; i < 7; i++)
			display_point(b, color, i, j);
		display_bar(b, opponentOf(color), j);
		for (i = 7; i < 13; i++)
			display_point(b, color, i, j);
		console << " |\n";
	}

	console << "+---------------------------------------------+\n";
	console << "   1  2  3  4  5  6         7  8  9 10 11 12\n\n";

	console << "On roll: " << board::colorname(b.colorOnRoll()) << "\n";

	char offstr[40] = {0};
	if (b.checkersOnPoint(white, 0) || b.checkersOnPoint(black, 0))
		sprintf(offstr, "borne off: w=%d b=%d",
			b.checkersOnPoint(white, 0),
			b.checkersOnPoint(black, 0));
	if (*offstr)
		console << offstr << "\n";
}
