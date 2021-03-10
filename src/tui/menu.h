#ifndef _MENU_H_
#define _MENU_H_

#include <ncurses.h>

typedef struct item_t item_t;
typedef struct menu_t menu_t;

struct item_t {
	char *name;
	int val;
	unsigned int index;
};

struct menu_t {
	unsigned int active: 1;
	WINDOW *main_win;
	WINDOW *sub_win;
	
	unsigned int rows;
	unsigned int cols;

	chtype foreground;
	chtype background;

	item_t **items;
	item_t  *cur_item;
	unsigned int max_items;

	unsigned short top_row;
	unsigned short cur_row;
};

enum REQ_MENU_ACTION {
	MENU_SCRL_UP,
	MENU_SCRL_DOWN,

	MENU_NEXT_ITEM,
	MENU_PREV_ITEM,

	MENU_NEXTP_ITEM,
	MENU_PREVP_ITEM,

	MENU_LAST_ITEM,
	MENU_FIRST_ITEM,

	MENU_SELECT_ITEM,

	MENU_UPDATE,
};

/* funcs */
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

item_t *new_item(char *name, int val);
void del_item(item_t *item);

char *item_name(item_t *item);
int   item_val(item_t *item);
item_t *cur_menu_item(menu_t *menu);
int item_index(item_t *item);

void edit_item(item_t *item, char *name, int val);

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

menu_t *new_menu(item_t **items);
int del_menu(menu_t *menu);

void activate_menu(menu_t *menu);
void diactivate_menu(menu_t *menu);

int del_menu_item(menu_t *menu, int ind);

int bind_menu_win(menu_t *menu, WINDOW *win);
int bind_menu_subwin(menu_t *menu, WINDOW *win);

int set_menu_format(menu_t *menu, unsigned int cols, unsigned int rows);

int set_menu_foreground(menu_t *menu, chtype color);
int set_menu_background(menu_t *menu, chtype color);

int menu_driver(menu_t *menu, enum REQ_MENU_ACTION);

#endif
