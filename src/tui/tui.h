#ifndef _TUI_H_
#define _TUI_H_

#include <ncurses.h>
#include "../games/gamerec.h"

typedef struct buf_t buf_t;
struct buf_t {
	char val[4096];
	short status; /* busyness */
};

enum MOVEMENT {
	UP,
	DOWN,
	LEFT,
	RIGHT,
	IDLE,
};

/* funcs */
void resizeHandler(int sig);
void init_tui();

void show_init_scr();
void show_status_bar();
void show_game_list(enum MOVEMENT d);
char *input_field();

void update_game_list();
void update_status_bar();

/* util */

#endif
