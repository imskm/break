#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <termios.h>
#include <unistd.h>

#define WIDTH 	100
#define HEIGHT 	40

#define BAR_CHAR '#'
#define SQUARE '@'
#define SQUARE_X 45
#define SQUARE_Y 30
#define BAR_Y 32 /* BAR_X will not be needed, place the bar in center */
#define BAR_LEN 10
#define BAR_LEN_HALF (BAR_LEN / 2)

typedef struct game_ball_t {
	int x;
	int y;
	int x_sign;
	int y_sign;
	int ball; /* ball symbol */
} game_ball_t;

typedef struct game_bar_t {
	int x;
	int y;
	int size;
	int bar_char;
} game_bar_t;


bool game_loop = true;
char screen[WIDTH * HEIGHT];

void game_terminal_init(void);
void game_terminal_restore(void);
void game_screen_init(char *screen, int w, int h);
void game_screen_render(const char *screen, int w, int h);
void game_draw_ball(game_ball_t *ball);
void game_draw_bar(game_bar_t *bar, int key);

struct termios old_termios;
struct termios new_termios;


int main(int argc, char **argv)
{
	char in;
	int key;
	fd_set rset, allset;
	struct timeval tv;
	int nready, nfds;

	game_ball_t ball = {
		.x=SQUARE_X,
		.y=SQUARE_Y,
		.x_sign=1,
		.y_sign=1,
		.ball=SQUARE,
	};

	game_bar_t bar = {
		.x=WIDTH / 2 - BAR_LEN_HALF,
		.y=BAR_Y,
		.size=BAR_LEN,
		.bar_char=BAR_CHAR,
	};

	game_terminal_init();

	atexit(game_terminal_restore);

	FD_ZERO(&allset);
	FD_SET(0, &allset);
	nfds = 1;

	game_screen_init(screen, WIDTH, HEIGHT);
	// Draw ball
	screen[WIDTH * SQUARE_Y + SQUARE_X] = SQUARE;
	game_screen_render(screen, WIDTH, HEIGHT);

	while (game_loop) {
		key = 0;
		rset = allset;
		tv.tv_sec = 0;
		tv.tv_usec = 50000;
		nready = select(nfds, &rset, NULL, NULL, &tv);
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

		game_draw_ball(&ball);
		game_draw_bar(&bar, key);

		/* 3. Render game */
		game_screen_render(screen, WIDTH, HEIGHT);
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
game_screen_render(const char *screen, int w, int h)
{
	/* Position the cursor at Line 0 and Column 0 to prevent scroll */
	printf("\033[2J");
	printf("\033[1;1H");

	fwrite(screen, w * h, 1, stdout);
	fflush(stdout);
}

void
game_terminal_restore(void)
{
	/* Show cursor */
	printf("\033[?25h");
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &old_termios);
}

void
game_draw_ball(game_ball_t *ball)
{
	if (ball->x >= WIDTH)    ball->x_sign = -1;
	if (ball->y >= HEIGHT) 	 ball->y_sign = -1;
	if (ball->x < 1) 		 ball->x_sign = 1;
	if (ball->y < 1) 		 ball->y_sign = 1;

	ball->x += 1 * ball->x_sign;
	ball->y += 1 * ball->y_sign;

	screen[WIDTH * ball->y + ball->x] = ball->ball;
}

void
game_draw_bar(game_bar_t *bar, int key)
{
	if ((key == 'A' || key == 'a') && bar->x > 1)
		bar->x = bar->x - 2;
	else if ((key == 'D' || key == 'd') && bar->x < WIDTH - bar->size  - 1)
		bar->x = bar->x + 2;
	for (int i = 0; i < bar->size; ++i)
		screen[WIDTH * bar->y + bar->x + i] = bar->bar_char;
}

void
game_terminal_init(void)
{
	if (tcgetattr(STDIN_FILENO, &old_termios) == -1) {
		perror("tcgetattr");
		exit(EXIT_FAILURE);
	}

	new_termios = old_termios;
	new_termios.c_lflag &= ~(ICANON | ECHO);
	if (tcsetattr(STDIN_FILENO, TCSANOW, &new_termios) == -1) {
		perror("tcssetattr");
		exit(EXIT_FAILURE);
	}
	/* Hid cursor */
	printf("\033[?25l");
}
