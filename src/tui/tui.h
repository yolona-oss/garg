#ifndef _TUI_H_
#define _TUI_H_

#include <ncurses.h>
#include "../games/gamerec.h"

#define CENTER(s, d) (s-d)/2
#define RIGHT(s, d) (s-d)

#define CTRL(k) (k & 0x1F)

enum MENU_ACT {
	M_UP,
	M_DOWN,
	M_UPAGE,
	M_DPAGE,

	SELECT,
	BACK,

	NOTHING,
};

/* vars */
extern int done;

/* funcs */
void resizeHandler(int sig);

void init_tui(void);
void init_game_menu(void);
void destroy_game_menu(void);

void show_status_bar(void);
void menu_move(enum MENU_ACT d);

/* util */

/* input */
void input_handle(void);

char start_ibuf(void);
void append_ibuf(char c);
void drop_ibuf(void);

#endif
