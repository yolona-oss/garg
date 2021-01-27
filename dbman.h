#ifndef _DBMAN_H_
#define _DBMAN_H_

#include <sqlite3.h>

sqlite3 *db_init(void);

int db_length(sqlite3 *db);


#endif
