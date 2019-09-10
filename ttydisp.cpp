/* Written by Todd Doucet.  See file copyright.txt.
 */

#include <string>
#include <iostream>
#include <iomanip>

#include "board.h"

using std::cout;

static void display_checker(color_t color, int level, int n)
{
    char ch =  (color == white) ? 'O' : 'X';
    std::ostringstream buf;

    if (level == 1 && n > 5)
        buf << " " << std::left << std::setw(2) << n - 4;
    else if (n >= level)
        buf << " " << ch << " ";
    else
        buf << "   ";

    cout << buf.str();
}

static void display_bar(const board &b, color_t color, int level)
{
    cout << " |";
    display_checker(color, level, b.checkersOnBar(color));
    cout << "| ";
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
        
    cout << "\n";
    cout << "  24 23 22 21 20 19        18 17 16 15 14 13\n";
    cout << "+---------------------------------------------+\n";
    for (j = 1; j <= 5; j++)
    {
        cout << "| ";
        for (i = 24; i > 18; i--)
            display_point(b, color, i, j);
        display_bar(b, color, j);
        for (i = 18; i > 12; i--)
            display_point(b, color, i, j);
        cout << " |\n";
    }

    cout << "|                    |   |                    |\n";
    for (j = 5; j >= 1; j--)
    {
        cout << "| ";
        for (i = 1; i < 7; i++)
            display_point(b, color, i, j);
        display_bar(b, opponentOf(color), j);
        for (i = 7; i < 13; i++)
            display_point(b, color, i, j);
        cout << " |\n";
    }

    cout << "+---------------------------------------------+\n";
    cout << "   1  2  3  4  5  6         7  8  9 10 11 12\n\n";

    cout << "On roll: " << board::colorname(b.onRoll()) << "\n";

    if (b.checkersOnPoint(white, 0) || b.checkersOnPoint(black, 0))
    {
        cout << "borne off: w=" << b.checkersOnPoint(white, 0)
               << " b=" << b.checkersOnPoint(black, 0) << "\n";
    }
}
