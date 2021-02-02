#ifndef _DBMAN_H_
#define _DBMAN_H_

#include "games_cache.h"
#include "search_cache.h"

sqlite3 *db_init(void);

int db_length(sqlite3 *db);

#endif
