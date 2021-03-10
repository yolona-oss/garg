#ifndef _TUI_H_
#define _TUI_H_

#include <ncurses.h>
#include "../games/gamerec.h"

#define MAX_STATUS_BUF 4096

#define CENTER(s, d) (s-d)/2
#define RIGHT(s, d)  (s-d)

#define CTRL(k) (k & 0x1F)
#define KEY_ESC 0x1b

enum MENU_ACT {
	M_UP,
	M_DOWN,
	M_UPAGE,
	M_DPAGE,

	M_FIRST,
	M_LAST,

	SELECT_ITEM,
	ITEM_INFO_TOGGLE,
	FIND_ITEM,
	EDIT_ITEM,
	DELETE_ITEM,

	ESC,
	NOTHING,
};

typedef struct aval_t aval_t;
struct aval_t {
	enum MENU_ACT action;
	int val;
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

void menu_move(aval_t aval);

/* util */

/* input */
aval_t *key_to_action(const char key);

#endif
