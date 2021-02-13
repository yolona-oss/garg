#ifndef _DBMAN_H_
#define _DBMAN_H_

#include <limits.h>
#include <sqlite3.h>
#include "../games/gamerec.h"

sqlite3 *db_init(void);

int db_length(sqlite3 *db);

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
/* games */
int     db_rm_rec(sqlite3 *db, int n);
game_t *db_get_rec(const char *path, int n);
int     db_put_rec(sqlite3 *db, game_t *GE);

int db_cache_recs(void);
int db_read_cached_recs(void);

/* vars */
extern game_tab_t gr_tab;
extern char g_user_db[PATH_MAX];

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
/* settings */
#include "../utils/list.h"

#define MAX_EXCEPTIONS  1000
#define MAX_INCLUSIONS  MAX_EXCEPTIONS

/* vars */
extern node_t *g_exceptions;
extern node_t *g_exceptions_head;

/* funcs */
int db_read_settings(void);

#endif
