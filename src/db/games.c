#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <limits.h>
#include <sqlite3.h>
#include <math.h>

#include "dbman.h"
#include "../utils/util.h"
#include "../utils/eprintf.h"

/* vars */
game_tab_t gr_tab;

int
db_upd_rec(sqlite3 *db, game_t *grp)
{
	if (!grp) {
		warn("GR is not exist!");
		return -1;
	}

	sqlite3_stmt *res;
	int rc;
	const char *sql_req = "UPDATE Games \
						   SET (id, last_time, play_time, name, icon, gener, location, start_point, start_argv, uninstaller) = \
						       (?,  ?,         ?,         ?,    ?,    ?,     ?,        ?,           ?,          ?) \
						   WHERE id = ?";

	rc = sqlite3_prepare_v2(db, sql_req, -1, &res, 0);

	if (rc == SQLITE_OK)
	{
		sqlite3_bind_int(res, 1, grp->id);
		sqlite3_bind_int(res, 2, grp->last_time);
		sqlite3_bind_int(res, 3, grp->play_time);
		sqlite3_bind_text(res, 4, grp->name, strlen(grp->name), NULL);
		if (grp->icon)  sqlite3_bind_text(res, 5, grp->icon, strlen(grp->icon), NULL);
		if (grp->gener) sqlite3_bind_text(res, 6, grp->gener, strlen(grp->gener), NULL);
		sqlite3_bind_text(res, 7, grp->location, strlen(grp->location), NULL);
		sqlite3_bind_text(res, 8, grp->start_point, strlen(grp->start_point), NULL);
		if (grp->start_argv)  sqlite3_bind_text(res, 9, grp->start_argv, strlen(grp->start_argv), NULL);
		if (grp->uninstaller) sqlite3_bind_text(res, 10, grp->uninstaller, strlen(grp->uninstaller), NULL);
		sqlite3_bind_int(res, 11, grp->id);

		sqlite3_step(res);
		sqlite3_finalize(res);
	} else {
		warn("Failed to execute statement: %s", sqlite3_errmsg(db));
	}

	return rc;
}

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
		last_time, \
		play_time, \
		name, \
		icon, \
		gener, \
		location, \
		start_point, \
		start_argv, \
		uninstaller) \
		VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?) \
		ON CONFLICT (name) DO UPDATE SET id = id";

	rc = sqlite3_prepare_v2(db, sql_req, -1, &res, 0);

	if (rc == SQLITE_OK)
	{
		sqlite3_bind_int(res, 1, grp->id);
		sqlite3_bind_int(res, 2, grp->last_time); //TODO
		sqlite3_bind_int(res, 3, grp->play_time); //TODO
		sqlite3_bind_text(res, 4, grp->name, strlen(grp->name), NULL);

		if (grp->icon)  sqlite3_bind_text(res, 5, grp->icon, strlen(grp->icon), NULL);
		if (grp->gener) sqlite3_bind_text(res, 6, grp->gener, strlen(grp->gener), NULL);

		sqlite3_bind_text(res, 7, grp->location, strlen(grp->location), NULL);
		sqlite3_bind_text(res, 8, grp->start_point, strlen(grp->start_point), NULL);

		if (grp->start_argv)  sqlite3_bind_text(res, 9, grp->start_argv, strlen(grp->start_argv), NULL);
		if (grp->uninstaller) sqlite3_bind_text(res, 10, grp->uninstaller, strlen(grp->uninstaller), NULL);


		sqlite3_step(res);
		sqlite3_finalize(res);
	} else {
		warn("Failed to execute statement: %s", sqlite3_errmsg(db));
	}

	return rc;
}

/* Delete game record with pushed id */
int
db_rm_game(int id)
{
	sqlite3 *db = db_init();
	sqlite3_stmt *statement;
	char *sql_req = "DELETE FROM Games WHERE id = ?";

	int rc = sqlite3_prepare_v2(db, sql_req, -1, &statement, 0);

	if (rc == SQLITE_OK) {
		sqlite3_bind_int(statement, 1, id);
		sqlite3_step(statement);
		sqlite3_finalize(statement);
	} else {
		warn("Failed to execute statement: %s", sqlite3_errmsg(db));
	}

	sqlite3_close(db);

	return rc;
}

int
db_read_cached_recs()
{
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
		game_t *grp = (game_t *)ecalloc(1, sizeof*grp);

		/*  0,  1,  2,    3,    4,    5,    6,  7,   8        9 */
		/* id, lt, pt, name, icon, gener, loc, sp, sarg, uninst */
		while (sqlite3_step(res) == SQLITE_ROW) {
			grp->id        = sqlite3_column_int(res, 0);
			grp->last_time = sqlite3_column_int(res, 1);
			grp->play_time = sqlite3_column_int(res, 2);
			grp->name = estrdup((const char*)sqlite3_column_text(res, 3));

			tmp = (char *)sqlite3_column_text(res, 4);
			grp->icon = tmp ? estrdup(tmp) : NULL;

			tmp = (char *)sqlite3_column_text(res, 5);
			grp->gener = tmp ? estrdup(tmp) : NULL;

			grp->location    = estrdup((const char*)sqlite3_column_text(res, 6));
			grp->start_point = estrdup((const char*)sqlite3_column_text(res, 7));

			tmp = (char *)sqlite3_column_text(res, 8);
			grp->start_argv = tmp ? estrdup(tmp) : NULL;

			tmp = (char *)sqlite3_column_text(res, 9);
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
