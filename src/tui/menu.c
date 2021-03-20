#include <stdlib.h>

#include "../utils/eprintf.h"
#include "menu.h"

/* static int save_menu_position(menu_t *menu); */
static void draw_menu(menu_t *menu);

/* static int */
/* save_menu_position(menu_t *menu) */
/* { */

/* 	return 0; */
/* } */

item_t *
new_item(char *name, int val)
{
	item_t *item = (item_t *)ecalloc(1, sizeof*item);
	item->name = name;
	item->val  = val;
	return item;
}

void
del_item(item_t *item)
{
	free(item);
}

item_t *
cur_menu_item(menu_t *menu)
{
	return menu->items[menu->cur_item_id];
}

int
item_index(item_t *item)
{
	return item->index;
}

char *
item_name(item_t *item)
{
	return item->name;
}

int
item_val(item_t *item)
{
	return item->val;
}

void
edit_item(item_t *item, char *name, int val)
{
	item->name = name;
	item->val = val;
}

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

menu_t *
new_menu(item_t **items)
{
	int x, y;
	menu_t *menu;
	menu = (menu_t *)ecalloc(1, sizeof*menu);

	getmaxyx(stdscr, y, x);
	
	menu->main_win = newwin(0, 0, x, y);
	menu->sub_win  = newwin(0, 0, x, y);

	menu->cols = 1;
	menu->win_rows = y/2;

	menu->foreground = COLOR_WHITE;
	menu->background = COLOR_BLACK;

	menu->top_row = 0;
	menu->cur_row = 0;

	menu->active = 0;

	int i;
	for (i = 0; items[i]; i++) {
		items[i]->index = i;
	}
	menu->items = items;
	menu->items_count = i;
	menu->cur_item_id = 0;
	
	return menu;
}

int
del_menu(menu_t *menu)
{
	delwin(menu->main_win);
	delwin(menu->sub_win);

	free(menu);

	return 0;
}

static void
draw_menu(menu_t *menu)
{
	int i, j, sel_row;
	if (menu->cur_row > menu->win_rows-1) {
		sel_row = menu->win_rows + menu->top_row -1;
	} else {
		sel_row = menu->cur_row;
	}
	for (j = 0, i = menu->top_row; j < menu->win_rows; i++, j++)
	{
		if (i == sel_row)
			wattron(menu->sub_win, A_STANDOUT);
		else
			wattroff(menu->sub_win, A_STANDOUT);
		mvwprintw(menu->sub_win, j, 0, "%s", item_name(menu->items[i]));
	}
}

void
activate_menu(menu_t *menu)
{
	menu->active = 1;
	draw_menu(menu);
	wnoutrefresh(menu->sub_win);
}

void
diactivate_menu(menu_t *menu)
{
	menu->active = 0;
	wclear(menu->sub_win);
}

int
bind_menu_win(menu_t *menu, WINDOW *win)
{
	menu->main_win = win;
	return 0;
}

int
bind_menu_subwin(menu_t *menu, WINDOW *win)
{
	menu->sub_win = win;
	return 0;
}

int
set_menu_foreground(menu_t *menu, chtype color)
{
	menu->foreground = color;
	return 0;
}

int
set_menu_background(menu_t *menu, chtype color)
{
	menu->foreground = color;
	return 0;
}

int
set_menu_format(menu_t *menu, unsigned int cols, unsigned int rows)
{
	menu->cols = cols;
	menu->win_rows = rows;
	return 0;
}

int
menu_driver(menu_t *menu, enum REQ_MENU_ACTION act)
{
	if (!menu->active)
		return 1;
	switch (act)
	{
		case MENU_SCRL_UP:
			if (menu->top_row > 0)
			{
				menu->top_row--;
				draw_menu(menu);
			}
			break;
		case MENU_SCRL_DOWN:
			if (item_index(cur_menu_item(menu))+menu->win_rows < menu->items_count)
			{
				menu->top_row++;
				draw_menu(menu);
			}
			break;

		case MENU_NEXT_ITEM:
			if (item_index(cur_menu_item(menu)) < menu->items_count)
			{
				menu->cur_item_id++;
				menu->cur_row++;
				if (menu->cur_row > (menu->win_rows-1)) {
					menu->top_row++;
				}
				draw_menu(menu);
			}
			break;
		case MENU_PREV_ITEM:
			if (item_index(cur_menu_item(menu)) > 0)
			{
				menu->cur_item_id--;
				menu->cur_row--;
				if (menu->cur_row == (menu->win_rows)) {
					menu->top_row--;
				}
				draw_menu(menu);
			}
			break;

		case MENU_NEXTP_ITEM:
			break;
		case MENU_PREVP_ITEM:
			break;

		case MENU_LAST_ITEM:
			menu->cur_item_id = menu->items_count;
			menu->cur_row = menu->win_rows;
			/* menu->top_row = */ 
			draw_menu(menu);
			break;
		case MENU_FIRST_ITEM:
			menu->cur_item_id = 0;
			menu->cur_row = 0;
			draw_menu(menu);
			break;

		case MENU_SELECT_ITEM:
			break;
	}
	wrefresh(menu->sub_win);

	return 0;
}
