#include <stdio.h>
#include <string.h>
#include <ncurses.h>

#include "../utils/eprintf.h"
#include "tui.h"

#define PERC_OF(n, p) n*p/100

buf_t event_buf;

int g_scr_w;
int g_scr_h;

int g_max_gr;
int g_max_gen;

WINDOW *w_game_list;
WINDOW *w_status_bar;

/* status - [lsu] */
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
/* [NAME]           | [GENER] | [status] */
/* [NAME]           | [GENER] | [status] */
/* [NAME]           | [GENER] | [status] */
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
/* [NGAMES]                        [YXX] */
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

void
init_tui()
{
	getmaxyx(stdscr, g_scr_h, g_scr_w);

	w_game_list  = newwin(g_scr_h-1, g_scr_w, 0, 0);
	w_status_bar = newwin(1, g_scr_w, g_scr_h-1, 0);

	/* TODO */
	g_max_gr = g_scr_w - PERC_OF(g_scr_w, 30) - 3;
}

void
show_init_scr()
{
	
}

void
show_status_bar()
{

}

void
show_game_entry(game_t gr)
{
	int len;
	len = strlen(gr.name) + strlen(gr.gener) + 7;
}

void
show_game_list()
{

}

char *
input_field()
{

	return NULL;
}

void
add_buf(char *str)
{
	/* TODO */
	/* if (event_buf.status) { */
	/* } */
	esnprintf(event_buf.val, sizeof(event_buf.val),
			"%s", str);
}
