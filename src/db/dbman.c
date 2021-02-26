#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <limits.h>

#include "../main.h"
#include "../utils/util.h"
#include "../utils/eprintf.h"

sqlite3 *
db_init(void)
{
	char path[PATH_MAX];

	if (!g_user_db[0]) {
		esnprintf(path, sizeof(path), "%s/%s", getenv("HOME"), ".cache/garg.db");
		if (!path[0]) {
			return NULL;
		}
	} else if (!isExist(g_user_db)) {
		warn("Wrong user db path");
		return NULL;
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

	sqlite3_open(path, &db);

	char *crg = "CREATE TABLE IF NOT EXISTS Games( \
	 		id INT, \
			last_time, \
	 		play_time INT, \
	 		name TEXT, \
	 		icon TEXT, \
	 		gener TEXT, \
	 		location TEXT, \
	 		start_point TEXT, \
			start_argv TEXT, \
			uninstaller TEXT, \
			UNIQUE(name));";
	
	int rc = sqlite3_exec(db, crg, 0, 0, &err);

	if (rc != SQLITE_OK) {
		fprintf(stderr, "Failed to create Games table\n");
		fprintf(stderr, "SQL error: %s\n", err);
		sqlite3_free(err);
		return NULL;
	}

	char *crs = "CREATE TABLE IF NOT EXISTS Search(\
				 exceptions TEXT, \
				 inclusions TEXT, \
				 UNIQUE(exceptions), \
				 UNIQUE(inclusions));";

	rc = sqlite3_exec(db, crs, 0, 0, &err);

	if (rc != SQLITE_OK) {
		fprintf(stderr, "Failed to create Search table\n");
		fprintf(stderr, "SQL error: %s\n", err);
		sqlite3_free(err);
		return NULL;
	}

	return db;
}

int
db_length(sqlite3 *db)
{
	return sqlite3_last_insert_rowid(db);
}
