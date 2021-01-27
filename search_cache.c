#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "list.h"
#include "eprintf.h"
#include "dbman.h"
#include "search_cache.h"


extern node_t *g_exceptions;
extern node_t *g_exceptions_head;
extern node_t *g_inclusions;
extern node_t *g_inclusions_head;

void
add_exception(node_t *head, const char *name)
{
	node_t *exc  = list_new((char *)name);
	g_exceptions = list_addhead(&g_exceptions, exc);
}

int
sqlite_add_exception(void *NotUsed, int argc, char **argv, char **azColName)
{
	NotUsed = 0;

	add_exception(g_exceptions, argv[0]);

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
	printf("Reading settings...\n"); fflush(stdout);

	sqlite3 *db;
	if (!(db=db_init())) {
		warn("Cant write cache.");
		return -1;
	}

	db_get_exceptions(db);

	sqlite3_close(db);

	return 1;
}

