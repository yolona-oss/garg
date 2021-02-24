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
static char status_buf[4024] = {0};

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
			  *w_status_bar;

/* funcs */
static char *rec_status(game_t gr);
static char *game_entry(game_t gr);
static void add_str_status_buf(const char *);

static void
add_str_status_buf(const char *str)
{
	esnprintf(status_buf, sizeof(status_buf), "%s", str);
}

static char *
rec_status(game_t gr)
{
	char res[5] = {};

	if (gr.properties.icon) res[0] = 'i';
	else                    res[0] = 'i';

	if (gr.properties.location) res[1] = 'l';
	else                        res[1] = 'l';

	if (gr.properties.start_point) res[2] = 's';
	else                           res[2] = 's';

	if (gr.properties.uninstaller) res[3] = 'u';
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
	w_game_list  = newwin(ROWS-1, COLUMNS, 0, 0);
	w_status_bar = newwin(1, COLUMNS, ROWS-1, 0);

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
init_game_menu()
{
	g_entries = (ITEM **)calloc(gr_tab.ngames+1, sizeof(ITEM *));
	char *tmp = NULL;

	int i;
	for (i = 0; i < gr_tab.ngames; i++) {
		tmp = itoa(gr_tab.game_rec[i].id, 10);
		g_entries[i] = new_item(estrdup(game_entry(gr_tab.game_rec[i])), estrdup(tmp));
	}
	g_entries[gr_tab.ngames] = (ITEM *)NULL;

	/* creating menu */
	g_menu_list = new_menu((ITEM **)g_entries);

	/* binding menu to windows */
	WINDOW *ms_win = derwin(w_game_list, ROWS-1, COLUMNS, 0, 0);
	set_menu_win(g_menu_list, w_game_list);
	set_menu_sub(g_menu_list, ms_win);

	set_menu_format(g_menu_list, ROWS-1, 0);

	set_menu_mark(g_menu_list, "");

	post_menu(g_menu_list);
	wnoutrefresh(w_game_list);
}

void
destroy_game_menu()
{
	unpost_menu(g_menu_list);
	free_menu(g_menu_list);

	for (int i = 0; i < gr_tab.ngames; i++) {
		free_item(g_entries[i]);
	}
	free(g_entries);
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
menu_move(enum MENU_ACT a)
{
	if (a == M_UP) {
		menu_driver(g_menu_list, REQ_UP_ITEM);
	} else if (a == M_DOWN) {
		menu_driver(g_menu_list, REQ_DOWN_ITEM);
	} else if (a == M_DPAGE) {
		menu_driver(g_menu_list, REQ_SCR_DPAGE);
	} else if (a == M_UPAGE) {
		menu_driver(g_menu_list, REQ_SCR_UPAGE);
	} else if (a == SELECT) {
		menu_driver(g_menu_list, REQ_TOGGLE_ITEM);

		int id = grt_ind(atoi(item_description(current_item(g_menu_list))));
		run_game(id);
	}

	wnoutrefresh(w_game_list);
}
