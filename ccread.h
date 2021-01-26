#ifndef _CCREAD_H_
#define _CCREAD_H_

#define MAX_EXCEPTIONS  1000
#define MAX_INCLUSIONS  MAX_EXCEPTIONS

#include <sqlite3.h>

/* funcs */
int cache_delete_record(sqlite3 *db, int n);

struct Game_rec *db_get_rec(const char *path, int n);
int db_put_rec(sqlite3 *db, struct Game_rec *GE);
int db_length(sqlite3 *db);

int db_read_settings(void);
int db_cache_recs(void);
int db_read_cached_recs(void);

/* vars */
extern struct Gr_tab gr_tab;
extern char *g_user_db;

#endif
