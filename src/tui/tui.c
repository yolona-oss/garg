#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>

#include <unistd.h>

#include "../main.h"
#include "../utils/eprintf.h"
#include "../utils/util.h"
#include "../db/dbman.h"
#include "menu.h"
#include "tui.h"

/* status - [ilsu] */
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
/* [NAME]           | [GENER] | [status] */
/* [NAME]           | [GENER] | [status] */
/* [NAME]           | [GENER] | [status] */
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
/* [NGAMES]                        [YXX] */
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

/* vars */
static char status_buf[3][MAX_STATUS_BUF] = {0};

static int COLUMNS, ROWS;

static int g_max_gn,
		   g_max_gen,
		   g_max_stat;

static int g_perc_field_gn   = 60,
		   g_perc_field_gen  = 30,
		   g_perc_field_stat = 10;

static WINDOW *w_game_list,
			  *w_status_bar,
			  *w_header;

/* funcs */
static char *rec_status(game_t gr);
static char *game_entry(game_t gr);

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

static char *
rec_status(game_t gr)
{
	char res[5] = {};

	if (gr.properties.icon == 1) res[0] = '-';
	else res[0] = 'i';
	if (gr.properties.location == 1) res[1] = '-';
	else res[1] = 'l';
	if (gr.properties.start_point == 1) res[2] = '-';
	else res[2] = 's';
	if (gr.properties.uninstaller == 1) res[3] = '-';
	else res[3] = 'u';

	res[4] = '\0';

	return bprintf("[%s]", res);
}

static char *
game_entry(game_t gr)
{
	char entry[COLUMNS+1];
	char *gn, *gener, *stat;

	add_str_status_buf(0, itoa(gr_tab.ngames, 10));

	memset(entry, ' ', COLUMNS);

	gn    = entry;
	gener = gn + g_max_gn - 1;
	stat  = gener + g_max_gen;

	char *tmp = cut(gr.name, g_max_gn);
	memmove(gn, tmp, strlen(tmp));
	memmove(gener + CENTER(g_max_gen, 5), "GENER", 5);
	memmove(stat + CENTER(g_max_stat, 6), rec_status(gr), 6);

	entry[COLUMNS] = '\0';

	return bprintf("%s", entry);
}

void
resizeHandler(void)
{
	getmaxyx(stdscr, ROWS, COLUMNS);
}

void
init_tui()
{
	if (!initscr()) {
		exit(1);
	}
	cbreak();
	timeout(0);  /* input delay */
	curs_set(0); /* invisible cursor */
	noecho();    /* no echo input */
	keypad(stdscr, 1);

	getmaxyx(stdscr, ROWS, COLUMNS);

	/* creating windows */
	w_game_list  = newwin(ROWS-2, COLUMNS, 1, 0);
	w_status_bar = newwin(1, COLUMNS, ROWS-1, 0);
	w_header     = newwin(1, COLUMNS, 0, 0);

	g_max_gn   = COLUMNS - PERC_OF(COLUMNS, (100-g_perc_field_gn));
	g_max_gen  = COLUMNS - PERC_OF(COLUMNS, (100-g_perc_field_gen));
	g_max_stat = COLUMNS - PERC_OF(COLUMNS, (100-g_perc_field_stat));

	/* check stdscr size TODO */
}

void
destroy_tui()
{
	curs_set(1);
	echo();

	endwin();
}

static void
draw_menu_title(menu_t *menu) {
	wattron(w_game_list, A_UNDERLINE);
	whline(menu->main_win, ' ', COLUMNS);
	mvwprintw(menu->main_win, 0, 0, "%s", cut("Game name", g_max_gn));
	mvwprintw(menu->main_win, 0, g_max_gn+CENTER(g_max_gen, 5)-1, "%s", cut("Gener", g_max_gen));
	mvwprintw(menu->main_win, 0, g_max_gn+g_max_gen+CENTER(g_max_stat, 6)-1, "Status");
	wattroff(menu->main_win, A_UNDERLINE);
}

menu_t *
init_game_menu()
{
	menu_t *menu;
	item_t **items = (item_t **)ecalloc(gr_tab.ngames+1, sizeof(item_t *));

	int i;
	for (i = 0; i < gr_tab.ngames; i++) {
		items[i] = new_item(estrdup(game_entry(gr_tab.game_rec[i])),
							gr_tab.game_rec[i].id);
	}
	items[gr_tab.ngames] = (item_t *)NULL;

	/* creating menu */
	menu = new_menu(items);

	/* binding menu to windows */
	int w, h;
	getmaxyx(w_game_list, h, w);
	WINDOW *ms_win = derwin(w_game_list, h-1, w, 1, 0);
	bind_menu_win(menu, w_game_list);
	bind_menu_subwin(menu, ms_win);

	/* menu options */
	set_menu_format(menu, w, h-1);

	/* menu title */
	draw_menu_title(menu);

	activate_menu(menu);

	return menu;
}

void
destroy_game_menu(menu_t *menu)
{
	diactivate_menu(menu);

	for (unsigned int i = 0; i < menu->items_count; i++) {
		free(item_name(menu->items[i]));
		del_item(menu->items[i]);
	}
	free(menu->items);
	del_menu(menu);
}

void
show_header()
{
	mvwprintw(w_header, 0, 0, "%s", "menu1 | menu2 | menu3");
	
	wnoutrefresh(w_header);
}

void
show_status_bar()
{
	int lf, mf, rf;
	char bar[COLUMNS+1];
	char *left, *mid, *right;

	//TODO
	char msg[COLUMNS/3+1];
	esnprintf(msg, sizeof(msg), "Games: %d", gr_tab.ngames);
	add_str_status_buf(0, msg);

	lf = strlen(status_buf[0]);
	mf = strlen(status_buf[1]);
	rf = strlen(status_buf[2]);

	memset(bar, ' ', COLUMNS);

	int offset = COLUMNS/3;
	left  = bar;
	mid   = left + offset;
	right = mid  + offset;

	memmove(left, status_buf[0], lf);
	memmove(mid + CENTER(COLUMNS/3, mf), status_buf[1], mf);
	memmove(right + RIGHT(COLUMNS/3, rf), status_buf[2], rf);

	bar[COLUMNS] = '\0';

	wmove(w_status_bar, 0, 0);
	wclrtoeol(w_status_bar);
	wprintw(w_status_bar, "%s", bar);

	wnoutrefresh(w_status_bar);
}

void
add_str_status_buf(int pos, const char *str)
{
	esnprintf(status_buf[pos], sizeof(status_buf), "%s", str);
}

int
menu_move(menu_t *menu, aval_t *a)
{
	if (!menu->items_count) { //zero items
		return 1;
	}

	if (!a) {
		menu_driver(menu, MENU_REFRESH);
		return 0;
	}
	int id = -1;
	int counter = 0;
	switch (a->action) {
		/* Movements */
		/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
		case M_UP:
			do {
				menu_driver(menu, MENU_PREV_ITEM);
				counter++;
			} while (counter < a->val);
			break;
		case M_DOWN:
			do {
				menu_driver(menu, MENU_NEXT_ITEM);
				counter++;
			} while (counter < a->val);
			break;
		case M_DPAGE:
			menu_driver(menu, MENU_NEXTP_ITEM);
			break;
		case M_UPAGE:
			menu_driver(menu, MENU_PREVP_ITEM);
			break;
		case M_FIRST:
			menu_driver(menu, MENU_FIRST_ITEM);
			break;
		case M_LAST:
			menu_driver(menu, MENU_LAST_ITEM);
			break;
		/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

		/* Item action */
		/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
		case RUN_ITEM:
			id = gr_id(grt_find(item_val(cur_menu_item(menu))));
			run_game(id);
			break;
		case ITEM_INFO_TOGGLE:
			//1 check if already toggled
			//if not toggled
				//1 resize list window
				//2 create info window
				//3 bind info win rander to main loop
			//if toggled
				//1 delete inf win
				//2 unbild info rander
			wresize(w_game_list, 10, 10);
			break;

		case EDIT_ITEM:
			menu_driver(menu, MENU_ITEM_EDIT);
			break;

		case FIND_ITEM:
			break;

		//add new
		//scan

		case DELETE_ITEM:
			{
			if (menu->items_count == 0) {
				break;
			}
			break;
			}
		/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

		/* addition functionality */
		/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
		/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

		case ESC:
			break;

		/* IDLE */
		case NOTHING:
			break;
	}

	wnoutrefresh(w_game_list);
	return 0;
}
