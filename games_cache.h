#ifndef _CCREAD_H_
#define _CCREAD_H_

#define MAX_EXCEPTIONS  1000
#define MAX_INCLUSIONS  MAX_EXCEPTIONS

#include "gamerec.h"
#include <sqlite3.h>
#include <limits.h>

/* funcs */
int     db_rm_rec(sqlite3 *db, int n);
game_t *db_get_rec(const char *path, int n);
int     db_put_rec(sqlite3 *db, game_t *GE);

int db_cache_recs(void);
int db_read_cached_recs(void);

/* vars */
extern game_tab_t gr_tab;
extern char g_user_db[PATH_MAX];

#endif
