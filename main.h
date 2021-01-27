#ifndef _MAIN_H_
#define _MAIN_H_

#include "list.h"

/* vars */
extern char *argv0;
extern int g_qflag, g_dflag;

extern char *g_user_db;

extern struct Gr_tab gr_tab;

extern node_t *g_exceptions;
extern node_t *g_exceptions_head;
extern node_t *g_inclusions;
extern node_t *g_inclusions_head;

/* functions */
int scan(const char *path);
void check_gr_tab(struct Gr_tab gr_tab);

#endif
