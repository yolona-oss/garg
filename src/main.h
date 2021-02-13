#ifndef _MAIN_H_
#define _MAIN_H_

#include "utils/list.h"
#include "games/gamerec.h"
#include <limits.h>

/* global vars */
extern int done;

extern char *argv0;
extern int g_qflag, g_dflag;

extern char g_user_db[PATH_MAX];
extern char g_user_path[PATH_MAX];

extern int g_scan_depth; //TODO add start up key

extern game_tab_t gr_tab;

extern node_t *g_exceptions;
extern node_t *g_inclusions;

#endif
