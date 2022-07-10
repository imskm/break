#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <termios.h>
#include <unistd.h>

#define WIDTH 	100
#define HEIGHT 	40

#define BAR "##########"
#define SQUARE '@'
#define SQUARE_X 45
#define SQUARE_Y 30
#define BAR_Y 32 /* BAR_X will not be needed, place the bar in center */
#define BAR_LEN (sizeof(BAR) - 1)
#define BAR_LEN_HALF (BAR_LEN / 2)


bool game_loop = true;
char screen[WIDTH * HEIGHT];
int square_x = SQUARE_X;
int square_y = SQUARE_Y;
int x_sign = 1;
int y_sign = 1;

void restore_terminal(void);
void game_screen_init(char *screen, int w, int h);
void game_render(const char *screen, int w, int h);
void game_handle_square(void);

struct termios old_termios;
struct termios new_termios;


int main(int argc, char **argv)
{
	char in;
	int key;
	fd_set rset, allset;
	struct timeval tv;
	int nready, nfds;

	if (tcgetattr(STDIN_FILENO, &old_termios) == -1) {
		perror("tcgetattr");
		return EXIT_FAILURE;
	}

	new_termios = old_termios;
	new_termios.c_lflag &= ~(ICANON | ECHO);
	if (tcsetattr(STDIN_FILENO, TCSANOW, &new_termios) == -1) {
		perror("tcssetattr");
		return EXIT_FAILURE;
	}

	atexit(restore_terminal);

	FD_ZERO(&allset);
	FD_SET(0, &allset);
	nfds = 1;

	printf("\033[?25l");
	game_screen_init(screen, WIDTH, HEIGHT);
	// Draw square
	screen[WIDTH * SQUARE_Y + SQUARE_X] = SQUARE;
	game_render(screen, WIDTH, HEIGHT);

	while (game_loop) {
		rset = allset;
		tv.tv_sec = 0;
		tv.tv_usec = 50000;
		// printf("waiting...\n");
		nready = select(nfds, &rset, NULL, NULL, &tv);
		// printf("ready %d.\n", nready);
		if (nready == -1) {
			perror("select()");
			return EXIT_FAILURE;
		}


		/* 1. Read KEY: If there is any input then read it otherwise don't */
		if (nready == 1) {
			key = getchar();
			if (key == 'q') {
				game_loop = false;
			}
		}

		/* 2. Update game */
		game_screen_init(screen, WIDTH, HEIGHT);

		game_handle_square();

		/* Place the bar */
		for (int i = 0; i < BAR_LEN; ++i) {
			int x = WIDTH / 2 - BAR_LEN_HALF + i;
			screen[WIDTH * BAR_Y + x] = BAR[i];
		}



		/* 3. Render game */
		game_render(screen, WIDTH, HEIGHT);
		//exit(EXIT_SUCCESS);
	}

	return 0;
}

void
game_screen_init(char *screen, int w, int h)
{
	for (int i = 0; i < h; ++i) {
		for (int j = 0; j < w; ++j) {
			/* Corner */
			if ((i == 0 && j == 0) || (i == 0 && j == w - 1) || (i == h - 1 && j == 0) || (i == h - 1 && j == w - 1))
				screen[w * i + j] = '+';
			/* Edges: top and bottom */
			else if (i == 0 || i == h - 1)
				screen[w * i + j] = '-';
			/* Edges: left and right */
			else if (j == 0 || j == w - 1)
				screen[w * i + j] = '|';
			else
				screen[w * i + j] = ' ';
		}
	}
}

void
game_render(const char *screen, int w, int h)
{
	/* Position the cursor at Line 0 and Column 0 to prevent scroll */
	printf("\033[99A\033[100D");

	fwrite(screen, w * h, 1, stdout);
}

void
restore_terminal(void)
{
	printf("\033[?25h");
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &old_termios);
}

void
game_handle_square(void)
{
	if (square_x >= WIDTH)  x_sign = -1;
	if (square_y >= HEIGHT) y_sign = -1;
	if (square_x < 1) 		x_sign = 1;
	if (square_y < 1) 		y_sign = 1;

	square_x += 1 * x_sign;
	square_y += 1 * y_sign;

	screen[WIDTH * square_y + square_x] = SQUARE;
}
