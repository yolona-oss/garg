#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <menu.h>

#include <unistd.h>

#include "../main.h"
#include "../utils/eprintf.h"
#include "../utils/util.h"
#include "tui.h"

/* status - [lsu] */
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
/* [NAME]           | [GENER] | [status] */
/* [NAME]           | [GENER] | [status] */
/* [NAME]           | [GENER] | [status] */
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
/* [NGAMES]                        [YXX] */
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

/* vars */
char status_buf[MAX_STATUS_BUF] = {0};

static int COLUMNS, ROWS;

static int g_max_gn,
		   g_max_gen,
		   g_max_stat;

static int g_perc_field_gn   = 60,
		   g_perc_field_gen  = 30,
		   g_perc_field_stat = 10;

static MENU *g_menu_list;
static ITEM **g_entries;

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
	else                    res[0] = 'i';

	if (gr.properties.location == 1) res[1] = '-';
	else                        res[1] = 'l';

	if (gr.properties.start_point == 1) res[2] = '-';
	else                           res[2] = 's';

	if (gr.properties.uninstaller == 1) res[3] = '-';
	else                           res[3] = 'u';

	res[4] = '\0';

	return bprintf("[%s]", res);
}

static char *
game_entry(game_t gr)
{
	int len;
	char *tmp;

	char gn[g_max_gn+1];
	char gener[g_max_gen+1];
	char stat[g_max_stat+1];

	/* filling areas with spaces */
	memset(gn,    ' ', g_max_gn);
	memset(gener, ' ', g_max_gen);
	memset(stat,  ' ', g_max_stat);
	
	/* adding terminating char */
	gn[g_max_gn] = '\0';
	gener[g_max_gen] = '\0';
	stat[g_max_stat] = '\0';

	/* game name */
	tmp = cut(gr.name, g_max_gn);
	memmove(gn, tmp, strlen(tmp));

	/* gener */
	memmove(&gener[CENTER(g_max_gen, 5)], "GENER", 5);

	/* game entry stats */
	tmp = rec_status(gr);
	len = strlen(tmp);
	memmove(&stat[CENTER(g_max_stat, len)], tmp, len);

	return bprintf("%s %s %s", gn, gener, stat);
}

void
resizeHandler(int sig)
{
	getmaxyx(stdscr, ROWS, COLUMNS);
}

void
init_tui()
{
	initscr();
	timeout(0);  /* input delay */
	curs_set(0); /* invisible cursor */
	noecho();    /* no echo input */

	getmaxyx(stdscr, ROWS, COLUMNS);

	/* creating windows */
	w_game_list  = newwin(ROWS-2, COLUMNS, 1, 0);
	w_status_bar = newwin(1, COLUMNS, ROWS-1, 0);
	w_header     = newwin(1, COLUMNS, 0, 0);

	g_max_gn   = COLUMNS - PERC_OF(COLUMNS, (100-g_perc_field_gn)) - 1;
	g_max_gen  = COLUMNS - PERC_OF(COLUMNS, (100-g_perc_field_gen)) - 1;
	g_max_stat = COLUMNS - PERC_OF(COLUMNS, (100-g_perc_field_stat)) - 1;

	if (g_max_stat < 7) {
		int need = 7-g_max_stat;

		if (IS_ODD(need)) {
			g_max_gn -= need/2+1;
		} else {
			g_max_gn -= need/2;
		}

		g_max_gen -= need/2;
		g_max_stat = 7;
	}
}

void
destroy_tui()
{
	curs_set(1);
	echo();

	endwin();
}

void
init_game_menu()
{
	g_entries = (ITEM **)calloc(gr_tab.ngames+1, sizeof(ITEM *));

	int i;
	for (i = 0; i < gr_tab.ngames; i++) {
		g_entries[i] = new_item(estrdup(game_entry(gr_tab.game_rec[i])),
								estrdup(itoa(gr_tab.game_rec[i].id, 10)));
	}
	g_entries[gr_tab.ngames] = (ITEM *)NULL;

	/* creating menu */
	g_menu_list = new_menu((ITEM **)g_entries);

	/* binding menu to windows */
	int w, h;
	getmaxyx(w_game_list, h, w);
	WINDOW *ms_win = derwin(w_game_list, h-1, w, 1, 0);
	set_menu_win(g_menu_list, w_game_list);
	set_menu_sub(g_menu_list, ms_win);

	/* menu options */
	set_menu_format(g_menu_list, h-1, 0);
	set_menu_mark(g_menu_list, "");

	mvwprintw(w_game_list, 0, 0, "TITLE MENU");
	refresh();

	post_menu(g_menu_list);

	wnoutrefresh(w_game_list);
}

void
destroy_game_menu()
{
	unpost_menu(g_menu_list);
	free_menu(g_menu_list);

	for (int i = 0; i < gr_tab.ngames; i++) {
		free((char*)item_name(g_entries[i]));
		free((char*)item_description(g_entries[i]));
		free_item(g_entries[i]);
	}
	free(g_entries);
}

void
show_header()
{
	mvwprintw(w_header, 0, 0, "%s", "HEADER");
	
	wnoutrefresh(w_header);
}

void
show_status_bar()
{
	wmove(w_status_bar, 0, 0);
	wclrtobot(w_status_bar);

	wprintw(w_status_bar, "gamesc: %d; cur: %d, cur_gid: %s, h:%d, w:%d, buf: %s",
			gr_tab.ngames, item_index(current_item(g_menu_list))+1, item_description(current_item(g_menu_list)), ROWS, COLUMNS,
			status_buf);

	wnoutrefresh(w_status_bar);
}

void
add_str_status_buf(const char *str)
{
	esnprintf(status_buf, sizeof(status_buf), "%s", str);
}

void
menu_move(enum MENU_ACT a)
{
	switch (a) {
		/* movements */
		case M_UP:
			menu_driver(g_menu_list, REQ_UP_ITEM);
			break;
		case M_DOWN:
			menu_driver(g_menu_list, REQ_DOWN_ITEM);
			break;
		case M_DPAGE:
			menu_driver(g_menu_list, REQ_SCR_DPAGE);
			break;
		case M_UPAGE:
			menu_driver(g_menu_list, REQ_SCR_UPAGE);
			break;
		case M_FIRST:
			menu_driver(g_menu_list, REQ_FIRST_ITEM);
			break;
		case M_LAST:
			menu_driver(g_menu_list, REQ_LAST_ITEM);
			break;

		/* item action */
		case SELECT:
			menu_driver(g_menu_list, REQ_TOGGLE_ITEM);

			int id = grt_ind(atoi(item_description(current_item(g_menu_list))));
			run_game(id);
			break;

		/* addition functionality */
		case BACK:
			break;

		case NOTHING:
			break;
	}

	wnoutrefresh(w_game_list);
}
