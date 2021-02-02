#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <limits.h>
#include <sqlite3.h>
#include <math.h>

#include "../main.h"
#include "dbman.h"
#include "games_cache.h"
#include "../utils/util.h"
#include "../utils/eprintf.h"

/* vars */
game_tab_t gr_tab;

int
db_put_rec(sqlite3 *db, game_t *grp)
{
	if (!grp) {
		warn("GR is not exist!");
		return -1;
	}

	//TODO Try trancactions
	sqlite3_stmt *res;
	int rc;
	const char *sql_req = "INSERT INTO \
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
		VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?) \
		ON CONFLICT (name) DO UPDATE SET id = id";

	rc = sqlite3_prepare_v2(db, sql_req, -1, &res, 0);

	if (rc == SQLITE_OK) {
		sqlite3_bind_int(res, 1, grp->id);
		sqlite3_bind_int(res, 2, grp->play_time);
		sqlite3_bind_text(res, 3, grp->name, strlen(grp->name), NULL);

		if (grp->icon)  sqlite3_bind_text(res, 4, grp->icon, strlen(grp->icon), NULL);
		if (grp->gener) sqlite3_bind_text(res, 5, grp->gener, strlen(grp->gener), NULL);

		sqlite3_bind_text(res, 6, grp->location, strlen(grp->location), NULL);
		sqlite3_bind_text(res, 7, grp->start_point, strlen(grp->start_point), NULL);

		if (grp->start_argv)  sqlite3_bind_text(res, 8, grp->start_argv, strlen(grp->start_argv), NULL);
		if (grp->uninstaller) sqlite3_bind_text(res, 9, grp->uninstaller, strlen(grp->uninstaller), NULL);
	} else {
		warn("Failed to execute statement: %s", sqlite3_errmsg(db));
	}

	sqlite3_step(res);
	sqlite3_finalize(res);

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

int
db_read_cached_recs()
{
	printf("Reading game records...\n");

	sqlite3 *db;
	if (!(db=db_init())) {
		warn("Cant write cache.");
		return -1;
	}

	sqlite3_stmt *res;
	char *sql_req = "SELECT * FROM Games";

	int rc = sqlite3_prepare_v2(db, sql_req, -1, &res, 0);

	if (rc == SQLITE_OK) {
		char *tmp;
		game_t *grp;
		grp = (game_t *)ecalloc(1, sizeof*grp);

		/*  0,  1,    2,    3,     4,   5,  6,    7,      8 */
		/* id, pt, name, icon, gener, loc, sp, sarg, uninst */
		while (sqlite3_step(res) == SQLITE_ROW) {
			grp->id        = sqlite3_column_int(res, 0);
			grp->play_time = sqlite3_column_int(res, 1);
			grp->name = estrdup((const char*)sqlite3_column_text(res, 2));

			tmp = (char *)sqlite3_column_text(res, 3);
			grp->icon = tmp ? estrdup(tmp) : NULL;

			tmp = (char *)sqlite3_column_text(res, 4);
			grp->gener = tmp ? estrdup(tmp) : NULL;

			grp->location    = estrdup((const char*)sqlite3_column_text(res, 5));
			grp->start_point = estrdup((const char*)sqlite3_column_text(res, 6));

			tmp = (char *)sqlite3_column_text(res, 7);
			grp->start_argv = tmp ? estrdup(tmp) : NULL;

			tmp = (char *)sqlite3_column_text(res, 8);
			grp->uninstaller = tmp ? estrdup(tmp) : NULL;

			gr_add(grp);
		}
		sqlite3_finalize(res);
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
		db_put_rec(db, &gr_tab.game_rec[i]);
	}

	sqlite3_close(db);

	return i;
}
