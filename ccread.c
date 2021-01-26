#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <limits.h>
#include <math.h>
#include <sqlite3.h>

#include "ccread.h"
#include "util.h"
#include "eprintf.h"
#include "gamerec.h"

/* func */
static sqlite3 *db_init(void);
/* static int isCached(const char *path, Game_rec gr); */
/* static void cache_trim(const char *path); */
static int db_read_exeptions(sqlite3 *db);

/* vars */
struct Gr_tab gr_tab;

/* If gr aready exists in file, function
 * returns position of elem in file. */
/* static int */
/* isCached(const char *path, Game_rec gr) */
/* { */
/* 	return 0; */
/* } */

/* static void */
/* cache_trim(const char *path) */
/* { */
/* } */

/* Check cache file(~/cache/garg.db) for existence */
/* Create if dont and add libconfig constructions */
static sqlite3 *
db_init(void)
{
	char path[PATH_MAX];

	if (!g_user_db) {
		esnprintf(path, sizeof(path), "%s/%s", getenv("HOME"), ".cache/garg.db");
		if (!path[0]) {
			return NULL;
		}
	}

	if (!isExist(path)) {
		FILE *fd;
		if (!(fd=fopen(path, "w"))) {
			warn("fopen:");
			return NULL;
		}
		fclose(fd);
	}
	
	char *err = 0;
	sqlite3 *db;
	/* db = (sqlite3 *)calloc(1, sizeof(struct sqlite3 *)); */

	sqlite3_open(path, &db);

	char *cr = "CREATE TABLE IF NOT EXISTS Games( \
	 		id INT, \
	 		'play_time' INT, \
	 		name TEXT, \
	 		icon TEXT, \
	 		gener TEXT, \
	 		location TEXT, \
	 		start_point TEXT, \
			start_argv TEXT, \
			uninstaller TEXT, \
			UNIQUE(Name));";
	
	int rc = sqlite3_exec(db, cr, 0, 0, &err);

	if (rc != SQLITE_OK) {
		fprintf(stderr, "Failed to create table\n");
		fprintf(stderr, "SQL error: %s\n", err);
		sqlite3_free(err);
		return NULL;
	}

	return db;
}

/* Return record number "n" */
/* from cache->games group */
/* in file "path". */
Game_rec *
db_get_rec(const char *path, int n)
{
	/* const char *location, *name, *sp; */
	/* int id; */

	Game_rec *grp;
	grp = (Game_rec *)ecalloc(1, sizeof *grp + 1);
	if (!grp) {
		return NULL;
	}

	/* grp->id = id; */
	/* grp->location = estrdup(location); */
	/* grp->name = estrdup(name); */
	/* grp->start_point = estrdup(sp); */

	return grp;
}

/* Get number of Games Table rows */
int
cache_get_length(sqlite3 *db)
{
	return sqlite3_last_insert_rowid(db);
}

int
db_put_rec(sqlite3 *db, struct Game_rec *GE)
{
	if (!GE) {
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
		GE->id,
		GE->play_time,
		GE->name,
		GE->icon ? GE->icon : "",
		GE->gener ? GE->gener : "",
		GE->location,
		GE->start_point,
		GE->start_argv ? GE->start_argv : "",
		GE->uninstaller ? GE->uninstaller : "");

	rc = sqlite3_exec(db, sql_req, 0, 0, &err_msg);

	if (rc != SQLITE_OK) {
		warn("SQL error: %s", err_msg);
		sqlite3_free(err_msg);
		return -1;
	}

	return 0;
}

/* Delete game record with pushed id */
/* in file "path". */
int
cache_delete_record(sqlite3 *db, int id)
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

/* TODO */
static int
db_read_exeptions(sqlite3 *db)
{
	/* int i, len, ind; */
	int ind;

	exceptionName = (char **)ecalloc(MAX_EXCEPTIONS+1, sizeof(char));
	if (!exceptionName) {
		return -1;
	}

	ind = 0;
	/* default exceptions */
	exceptionName[ind] = (char *)ecalloc(9, sizeof(char));
	strcpy(exceptionName[ind++], "^Steam$");
	exceptionName[ind] = (char *)ecalloc(13, sizeof(char));
	strcpy(exceptionName[ind++], "^[.]wine\\w*");

	/* if (exceptionName[ind]) { */
	/* 	memcpy(exceptionName[ind++], exc, len); */

	/* pp_delete_dup(exceptionName, ind); */

	return ind;
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

	db_read_exeptions(db);

	sqlite3_close(db);

	return 1;
}

int
sql_gr_init(void *NotUsed, int argc, char **argv, char **azColName)
{
	Game_rec *grp;
	char *nm, *val;
	int i;

	NotUsed = 0;

	grp = (Game_rec *)ecalloc(1, sizeof*grp);

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
