#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <limits.h>
#include <sqlite3.h>
#include <math.h>

#include "main.h"
#include "dbman.h"
#include "games_cache.h"
#include "util.h"
#include "eprintf.h"
#include "gamerec.h"

/* func */
static int sql_gr_init(void *NotUsed, int argc, char **argv, char **azColName);

/* vars */
struct Gr_tab gr_tab;

/* Check db file(~/cache/garg.db) for existence */
/* Create if dont and add Games, Search tables */
/* Get number of Table rows */
int
db_put_rec(sqlite3 *db, game_t *grp)
{
	if (!grp) {
		warn("GR is not exist!");
		return -1;
	}

	char *err_msg = 0;
	int rc;
	int len = PATH_MAX*2 +
			FILENAME_MAX +
			UINT_MAX_DIG*2 +
			91;
	char sql_req[len];

	/* TODO try to update */
	esnprintf(sql_req, len,
		"INSERT INTO \
		Games(\
		id, \
		play_time, \
		name, \
		icon, \
		gener, \
		location, \
		start_point, \
		start_argv, \
		uninstaller) \
		VALUES(%d, %d, '%s', '%s', '%s', '%s', '%s', '%s', '%s')", 
		grp->id,
		grp->play_time,
		grp->name,
		grp->icon ? grp->icon : "",
		grp->gener ? grp->gener : "",
		grp->location,
		grp->start_point,
		grp->start_argv ? grp->start_argv : "",
		grp->uninstaller ? grp->uninstaller : "");

	rc = sqlite3_exec(db, sql_req, 0, 0, &err_msg);

	if (rc != SQLITE_OK) {
		warn("SQL error: %s", err_msg);
		sqlite3_free(err_msg);
		return -1;
	}

	return 0;
}

/* Delete game record with pushed id */
int
db_rm_rec(sqlite3 *db, int id)
{
	char *err_msg = 0;
	int len = 29 + UINT_MAX_DIG;
	char sql_req[10];

	esnprintf(sql_req, len, "DELETE FROM Games WHERE id = %d", id);

	int rc = sqlite3_exec(db, sql_req, 0, 0, &err_msg);
	
	if (rc != SQLITE_OK) {
		warn("SQL error: %s", err_msg);
		sqlite3_free(err_msg);
		return -1;
	}

	return rc;
}


static int
sql_gr_init(void *NotUsed, int argc, char **argv, char **azColName)
{
	game_t *grp;
	char *nm, *val;
	int i;

	NotUsed = 0;

	grp = (game_t *)ecalloc(1, sizeof*grp);

	/* TODO */
	for (i = 0; i < argc; i++) {
		nm  = azColName[i];
		val = argv[i];

		if (!strcmp(nm, "id")) {
			grp->id = atoi(val);
		} else if (!strcmp(nm, "play_time")) {
			grp->play_time = atoi(val);
		} else if (!strcmp(nm, "name")) {
			grp->name = val ? estrdup(val) : NULL;
		} else if (!strcmp(nm, "ganer")) {
			grp->gener = val ? estrdup(val) : NULL;
		} else if (!strcmp(nm, "location")) {
			grp->location = val ? estrdup(val) : NULL;
		} else if (!strcmp(nm, "start_point")) {
			grp->start_point = val ? estrdup(val) : NULL;
		} else if (!strcmp(nm, "start_argv")) {
			grp->start_argv = val ? estrdup(val) : NULL;
		} else if (!strcmp(nm, "uninstaller")) {
			grp->uninstaller = val ? estrdup(val) : NULL;
		}
	}

	gr_add(grp);
	free(grp);

	return 0;
}

int
db_read_cached_recs()
{
	printf("Reading game records...\n");

	sqlite3 *db;
	if (!(db=db_init())) {
		warn("Cant write cache.");
		return -1;
	}

	char *err_msg;
	char *sql_req = "SELECT * FROM Games;";

	int rc =sqlite3_exec(db, sql_req, sql_gr_init, 0, &err_msg);

	if (rc != SQLITE_OK) {
		warn("SQL error: %s", err_msg);
		sqlite3_free(err_msg);
		return -1;
	}

	sqlite3_close(db);
	
	return 0;
}

int
db_cache_recs(void)
{
	int i;
	sqlite3 *db;

	printf("Writing game records...\n"); fflush(stdout);

	if (!(db=db_init())) {
		warn("Cant write cache.");
		return -1;
	}

	for (i = 0; i < gr_tab.ngames; i++)
	{
		db_put_rec(db, &gr_tab.game_rec[i++]);
	}

	sqlite3_close(db);

	return i;
}
