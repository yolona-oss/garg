#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "../utils/eprintf.h"
#include "../event/event.h"
#include "dbman.h"

node_t *g_exceptions;
node_t *g_inclusions;

node_t *
add_exception(node_t *head, const char *name)
{
	node_t *exc  = list_new((char *)name);
	g_exceptions = list_addhead(&g_exceptions, exc);

	return g_exceptions;
}

int
sqlite_add_exception(void *NotUsed, int argc, char **argv, char **azColName)
{
	NotUsed = 0;

	g_exceptions = add_exception(g_exceptions, argv[0]);

	return 0;
}

void
db_put_exception(sqlite3 *db, const char *name)
{
	char *err_msg;
	int len = PATH_MAX + 45;
	char sql_req[len];

	esnprintf(sql_req, len,
			"INSERT INTO Search(exceptions) VALUES('%s');", name);

	int rc = sqlite3_exec(db, sql_req, 0, 0, &err_msg);

	if (rc != SQLITE_OK) {
		warn("SQL error: %s", err_msg);
		sqlite3_free(err_msg);
	}
}

void
db_rm_exception(sqlite3 *db, const char *name)
{
	;
}

void
db_get_exceptions(sqlite3 *db)
{
	char *err_msg;
	char *sql_req = "SELECT exceptions FROM Search;";

	sqlite3_exec(db, sql_req, sqlite_add_exception, 0, &err_msg);
}

int
db_read_settings(void)
{
	set_event(SCAN_START);

	sqlite3 *db;
	if (!(db=db_init())) {
		warn("Cant write cache.");
		return -1;
	}

	db_get_exceptions(db);

	sqlite3_close(db);

	set_event(SCAN_END);
	return 1;
}

