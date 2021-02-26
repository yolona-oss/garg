#ifndef _TUI_H_
#define _TUI_H_

#include <ncurses.h>
#include "../games/gamerec.h"

#define MAX_STATUS_BUF 4096

#define CENTER(s, d) (s-d)/2
#define RIGHT(s, d) (s-d)

#define CTRL(k) (k & 0x1F)

enum MENU_ACT {
	M_UP,
	M_DOWN,
	M_UPAGE,
	M_DPAGE,

	M_FIRST,
	M_LAST,

	SELECT,
	EXPAND,

	NOTHING,
};

/* vars */
extern int done;

/* funcs */
void resizeHandler(int sig);

void init_tui(void);
void destroy_tui(void);

void init_game_menu(void);
void destroy_game_menu(void);

void show_status_bar(void);
void add_str_status_buf(int pos, const char *str);
void show_header(void);

void menu_move(enum MENU_ACT d);

/* util */

/* input */
enum MENU_ACT input_command(void);

#endif
