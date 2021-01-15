#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <libgen.h>
#include <regex.h>
#include <limits.h>
#include <sys/stat.h>
#include <libconfig.h>

#include "main.h"
#include "ccread.h"
#include "util.h"

int
initCacheFile(const char *cachePath)
{
	FILE *fd = NULL;

	if (!(fd = fopen(cachePath, "rw"))) {
		warn("fopen:");
		return 0;
	}

	fclose(fd);

	return 1;
}

int
checkGameEntry(int id)
{
	int eflag = 0;

	if (!isExist(Game[id].location)) {
		warn("Game location dont exist");
		eflag |= 1;
	}
		
	if (!isStartPoint(Game[id].starPoint)) {
		warn("Game start point dont exist");
		eflag |= 2;
	}

	if (Game[id].name) {
		warn("Game dont hame name");
		eflag |= 4;
	}

	return eflag;
}

int
readCache(const char *cachePath)
{
	int local_id = 0;

	printf("Reading cache...\n");

	if (!initCacheFile(cachePath)) {
		warn("Cant init cache file: \"%s\"", cachePath);
		return -1;
	}

	config_t cfg;
	config_setting_t *setting, *game_setting;

	config_init(&cfg);

	if (!config_read_file(&cfg, cachePath)) {
		warn("Reading cache file: \"%s\" error:%d - %s", config_error_file(&cfg), config_error_line(&cfg), config_error_text(&cfg));
		config_destroy(&cfg);
		return -1;
	}

	setting = config_lookup(&cfg, "games");

	if (setting) {
		int count = config_setting_length(setting);
		int i;
		printf("\n");

		for (i = 0; i < count; i++) {
			game_setting = config_setting_get_elem(setting, i);

			const char *location, *name, *sp;
			int id;

			if(! (config_setting_lookup_string(game_setting, "location", &location) &&
					config_setting_lookup_string(game_setting, "name", &name) &&
					config_setting_lookup_string(game_setting, "startPoint", &sp) &&
					config_setting_lookup_int(game_setting, "id", &id))) {
				continue;
			}

			editGameEntry(local_id, name, location, sp);
			if (checkGameEntry(local_id) == 0) {
				local_id++;
			}
			/* printf("name - %s\nlocation - %s\n sp - %s\nid - %d\n\n", name, location, sp, id); */
		}
	}

	config_destroy(&cfg);

	return local_id;
}

int
writeCache(const char *cachePath)
{
	/* if (!initCacheFile(cachePath)) { */
	/* 	warn("Cant init cache file: \"%s\"", cachePath); */
	/* 	return -1; */
	/* } */

	/* config_t cfg; */
	/* config_setting_t *setting; */

	/* config_init(&cfg); */

	/* if (!config_write_file(&cfg, cachePath)) { */
	/* 	warn("Cant write cache:"); */
	/* 	config_destroy(&cfg); */
	/* 	return -1; */
	/* } */

	/* config_destroy(&cfg); */

	return 1;
}

