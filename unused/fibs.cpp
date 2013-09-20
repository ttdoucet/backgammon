/*
 * This file is broken until we fix the code that uses the board::place()
 * method, which has changed.
 */

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

#if defined(WIN32)
  #define _WIN32_WINNT 0x0502
  #include "windows.h"
  #pragma comment(lib, "ws2_32.lib")
#else
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <netdb.h>
  #include <sys/un.h>
#endif


#include <string.h>




//#include <iostream.h>
//#include <iomanip.h>


#include "random.h"
#include "board.h"
//#include "play.h"
#include "net.h"
#include "choose.h"

// the one currently in train.cpp.  In a state of flux...
extern float select_move(board &b, int ply, move &m);


/*** Fibs stuff  ***/

// Really thrown together.  This needs to be cleaned up!!

struct fibs {
	char my_name[30];
	char his_name[30];
	int match_length;
	int my_points;
	int his_points;

		// Positive numbers represent O's pieces
		// negative numbers represent X's pieces.
	int board[26];

	color_t turn;	// color of player on roll.
	int r1;
	int r2;
	int his_r1;
	int his_r2;
	int cube;
	int may_double;
	int he_may_double;
	int was_doubled;
	color_t color;		// -1 for black, 1 for white (like us)
	int direction;		// -1 normal, +1 for stupid
	int home;		// obsolete
	int bar;		// obsolete
	int borne_off;
	int his_borne_off;
	int on_bar;
	int his_on_bar;
	int can_move;
	int forced_move;	// don't use this token
	int crawford;		// don't use this token
	int redoubles;
} fibs_bd;

#include <setjmp.h>

jmp_buf fibs_parse;



/* Scans a string terminated by colon or null into result,
 * and returns the string past the terminating colon.
 */

char *scan_name(char *s, char *result, int null_ok)
{
	for (; *s && *s != ':'; s++){
		*result++ = *s;
	}
	*result = 0;
	if (*s == 0 && !null_ok){
		longjmp(fibs_parse, 1);
	}
	return ++s;
}

/* Scans a decimal string terminated by colon or null into result,
 * and returns the string past the terminating colon.
 */

char *sc_int(char *s, int &result, int null_ok)
{
	char digits[10];
	int res;
	char *retval = scan_name(s, digits, null_ok);
	if (sscanf(digits, "%d", &res) != 1){
		longjmp(fibs_parse, 1);
	}
	result = res;
	return retval;
}

char *scan_int(char *s, int &result)
{
	return sc_int(s, result, 0);
}

char *scan_color(char *s, int &result, char *description)
{
	char *p = sc_int(s, result, 0);
	if ( (result != (int) white) &&
	     (result != (int) black)){
		fprintf(stderr, "Unexpected color for %s: %d\n",
			  description, result);
		result = white;
		longjmp(fibs_parse, 1);
	}
	return p;
}

char *scan_final_int(char *s, int &result)
{
	return sc_int(s, result, 1);
}

int parse_fibs_board(char *s, struct fibs *f)
{
	char name[200], *p;

	if (setjmp(fibs_parse)){
//		confused = 1;
		return 0;
	}

	p = scan_name(s, name, 0);
	if (strcmp(name, "board"))
		return 0;
	p = scan_name(p, name, 0);

//	if (strcmp(name, "You"))
//		return 0;

	p = scan_name(p, name, 0);
	printf("opponent is %s\n", name);
	strcpy(f->his_name, name);

	p = scan_int(p, f->match_length);
	p = scan_int(p, f->my_points);  // points in match
	p = scan_int(p, f->his_points); // points in match
	for (int i = 0; i < 26; i++)
		p = scan_int(p, f->board[i]);

	p = scan_color(p, (int &) f->turn, "turn");
	printf("f->turn is: %d\n", f->turn);
	p = scan_int(p, f->r1);
	p = scan_int(p, f->r2);
	p = scan_int(p, f->his_r1);
	p = scan_int(p, f->his_r2);
	p = scan_int(p, f->cube);
	p = scan_int(p, f->may_double);
	p = scan_int(p, f->he_may_double);
	p = scan_int(p, f->was_doubled);
	p = scan_color(p, (int &) f->color, "my-color");
	printf("f->color is: %d\n", f->color);
	p = scan_int(p, f->direction);
	p = scan_int(p, f->home);
	p = scan_int(p, f->bar);
	p = scan_int(p, f->borne_off);
	p = scan_int(p, f->his_borne_off);
	p = scan_int(p, f->on_bar);
	p = scan_int(p, f->his_on_bar);
	p = scan_int(p, f->can_move);
	p = scan_int(p, f->forced_move);
	p = scan_int(p, f->crawford);
	p = scan_final_int(p, f->redoubles);
	return 1;
}

// This retrieves the number of men on point n,
// according to the fibs structure.  Positive
// means white, negative means black.

int fibs_point(struct fibs *f, int n)
{
	int i = (f->direction == -1) ? n : 25 - n;
	return f->board[i];
}

// This takes a board as described by the fibs structure
// and copies it to the indicated board.

void copy_fibs_board(struct fibs *f, board &b)
{
	b.clearBoard();
//	color_t fcolor = (f->color == 1) ? white : black;

	for (int i = 1; i <= 24; i++){
		int num = fibs_point(f, i);
		color_t color = (num > 0) ? white : black;
		if (num < 0)
			num = -num;
		if (color == white)
			b.place(color, i, num);
		else
			b.place(color, opponentPoint(i), num);
	}
// this seems to be fucked up.
	printf("f->borne_off: %d\n", f->borne_off);
	printf("f->on_bar: %d\n", f->on_bar);

	printf("f->his_borne_off: %d\n", f->his_borne_off);
	printf("f->his_on_bar: %d\n", f->his_on_bar);

	printf("fibs point 0: %d\n", fibs_point(f, 0));
	printf("fibs point 25: %d\n", fibs_point(f, 25));

	b.place(white, 0, f->borne_off);
	b.place(black, 0, f->his_borne_off);

//	b.place(white, 25, f->on_bar);
//	b.place(black, 25, f->his_on_bar);
	b.place(white, 25, fibs_point(f, 25));
	b.place(black, 25, -fibs_point(f, 0));




	b.setRoller( (f->turn == 1) ? white : black );
	b.setDice(f->r1, f->r2);
}

void show_fibs_stats(struct fibs *f)
{
	printf("Color is %d.\n", f->color);
	printf("On roll: %d.\n", f->turn);
	printf("Roll is %d-%d.\n", f->r1, f->r2);
	printf("his-Roll is %d-%d.\n", f->his_r1, f->his_r2);
	printf("Cube is at %d.\n", f->cube);
	printf("Direction is %s\n", (f->direction == 1) ? "stupid" : "normal");
	printf("can_move is %d.\n", f->can_move);
}

void setup_server_session(char *netname)
{
// This is broken until we incorporate the new player stuff.

/*
	whiteNet = read_network(netname);
	set_seed(user_seed? user_seed : whiteNet->seed);
	whiteNet->init_play();
	setNetwork(whiteNet);
*/
}


/*** Socket stuff ***/

void server_mode(char *netname)
{
	setup_server_session(netname);
	int sock;

	int r;

	sock = socket(PF_INET, SOCK_STREAM, 0);
	if (sock < 0){
		fprintf(stderr, "Could not create socket.\n");
		exit(-1);
	}
	int v = 1;
	r = setsockopt( sock, SOL_SOCKET, SO_REUSEADDR, (char *) &v, sizeof(v) );
	if (r){
		fprintf(stderr, "Could set socket options.\n");
		exit(-1);
	}

/*
	struct hostent *h;
	if ( (h = gethostbyname("localhost")) == 0){
		fprintf(stderr, "Could not get host by name.\n");
		exit(-1);
	}

*/

	struct in_addr ia;
#if !defined(WIN32)
// the code is broken for windows.

	if (inet_aton("192.168.0.40", &ia) == 0){
		fprintf(stderr, "could not find address\n");
		exit(-1);
	}
#endif

	struct sockaddr_in myaddr;
	myaddr.sin_family = AF_INET;
	myaddr.sin_port = htons(8888);
	myaddr.sin_addr = ia;

	if (bind(sock, (struct sockaddr *) &myaddr, sizeof(myaddr)) != 0){
		fprintf(stderr, "Could not bind socket.\n");
		exit(-1);
	}
	listen(sock, 5);

	printf("waiting for a connection...\n"); fflush(stdout);
	int client = accept(sock, 0, 0);
	if (client < 0){
		fprintf(stderr, "Error in accept()\n");
		exit(-1);
	}
	printf("client: %d\n", client);

	char buf[300];
	int n;
	int hack = 0;
	while ( (n = recv(client, buf, 200, 0)) >= 0){
		printf("recv() returns %d\n",n);
		buf[n] = 0;
		printf("got: %s\n", buf);
		if (parse_fibs_board(buf, &fibs_bd)){
			printf("That's a fibs board!!\n");
			show_fibs_stats(&fibs_bd);
			board b;
			copy_fibs_board(&fibs_bd, b);
			b.display_board(b.colorOnRoll());

			move m;
			hack++;

/*			if( (hack % 50) == 0){
				printf("init_play()\n");
				printf("learning: %d\n", whiteNet->learns());
				whiteNet->init_play();
			}
*/
			select_move(b, 2, m);
//			partialOnePlySelect(b, m);


			char bb[30];
			sprintf(bb, "%s\n", move_str(m));
			printf("my move: %s\n", bb);
			fflush(stdout);
			send(client, bb, strlen(bb), 0);

		} else {
			printf("That is NOT a fibs board.\n");
			exit(-1);
		}

	}
	printf("final n: %d\n", n);


}
