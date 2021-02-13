#ifndef _TUI_H_
#define _TUI_H_

#include "../games/gamerec.h"

typedef struct buf_t buf_t;
struct buf_t {
	char val[4096];
	short status; /* busyness */
};

/* vars */
extern buf_t event_buf;

extern int g_scr_w;
extern int g_scr_h;

/* funcs */
void show_init_scr();
void show_status_bar();
void show_game_entry(game_t gr);
char *input_field();

void add_buf(char *str);

/* util */
void cut(char *str, char *res, int width);

#endif
