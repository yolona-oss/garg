#include <stdio.h>
#include <stdlib.h> //tmp
#include <string.h>
#include <ncurses.h>

#include "../main.h"
#include "../utils/eprintf.h"
#include "../utils/util.h"
#include "tui.h"

#define PERC_OF(n, p) n*p/100

static int g_scr_w,
		   g_scr_h;

static int g_max_gn,
		   g_max_gen,
		   g_max_stat;

static int g_perc_field_gn,
		   g_perc_field_gen,
		   g_perc_field_stat;

static int g_list_i = 0;

static WINDOW *w_game_list,
			  *w_status_bar;

/* status - [lsu] */
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
/* [NAME]           | [GENER] | [status] */
/* [NAME]           | [GENER] | [status] */
/* [NAME]           | [GENER] | [status] */
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
/* [NGAMES]                        [YXX] */
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

void
resizeHandler(int sig)
{
	getmaxyx(stdscr, g_scr_h, g_scr_w);
}

void
init_tui()
{
	initscr();
	nodelay(stdscr, true);
	curs_set(0); /* invisible cursor */
	noecho();    /* no echo input */

	getmaxyx(stdscr, g_scr_h, g_scr_w);

	/* creating windows */
	w_game_list  = newwin(g_scr_h-1, g_scr_w, 0, 0);
	w_status_bar = newwin(1, g_scr_w, g_scr_h-1, 0);

	/* fields percentages */
	g_perc_field_gn = 60;
	g_perc_field_gen = 30;
	g_perc_field_stat = 10;

	g_max_gn   = g_scr_w - PERC_OF(g_scr_w, (100-g_perc_field_gn));
	g_max_gen  = g_scr_w - PERC_OF(g_scr_w, (100-g_perc_field_gen));
	g_max_stat = g_scr_w - PERC_OF(g_scr_w, (100-g_perc_field_stat));
}

void
show_init_scr()
{
	
}

void
show_status_bar()
{
	wmove(w_status_bar, 0, 0);
	wprintw(w_status_bar, "%d", g_list_i);
	update_status_bar();
}

static char *
game_status(game_t gr)
{
	char res[5] = {};
	memset(res, 0, 5);

	if (gr.properties.icon) strncat(res, "i", 4);
	else                    strncat(res, "i", 4); 

	if (gr.properties.location) strncat(res, "l", 4);
	else strncat(res, "l", 4);

	if (gr.properties.start_point) strncat(res, "s", 4);
	else strncat(res, "s", 4);

	if (gr.properties.uninstaller) strncat(res, "u", 4);
	else strncat(res, "u", 4);

	return bprintf("%s", res);
}

static void
show_game_entry(game_t gr)
{
	char *gana = cut(gr.name, g_max_gn);
	gana = cut(gr.name, g_max_gn);

	char tmp[g_max_gn+1];
	memset(tmp, ' ', g_max_gn);
	memmove(tmp, gana, strlen(gana));

	wprintw(w_game_list, "%s ", tmp);
	wprintw(w_game_list, "%s", game_status(gr));
	wprintw(w_game_list, "\n");
}

void
show_game_list(enum MOVEMENT d)
{
	if (d != IDLE) {
		if (d == UP && g_list_i > 0)
			g_list_i--;
		else if (d == DOWN && g_list_i < gr_tab.ngames-(g_scr_h-1))
			g_list_i++;
	}

	wmove(w_game_list, 0, 0);
	for (int i = g_list_i; i < g_list_i+g_scr_h-1; i++) {
		show_game_entry(gr_tab.game_rec[i]);
	}
	update_game_list();
}

char *
input_field()
{

	return NULL;
}

void update_game_list()  { wrefresh(w_game_list); }
void update_status_bar() { wrefresh(w_status_bar); }
