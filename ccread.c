#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <libconfig.h>

#include "global.h"
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

int
getConfigIndex(void)
{
	int n = -1;
	int i;

	for (i = 2; i != 0; i--) {
		if (isExist(confPathes[i])) {
			n = i;
			break;
		}
	}

	return n;
}

int
readExceptions(const char *confPath)
{
	int i, c, len, ind = 0;
	config_t cfg;
	config_setting_t *setting, *setting_names;

	config_init(&cfg);

	exceptionName = (char **)malloc(MAX_EXCEPTIONS+1);

	if (!config_read_file(&cfg, confPath)) {
		warn("Cant read config file: %s", confPath);
		config_destroy(&cfg);
		return -1;
	}

	setting = config_lookup(&cfg, "exceptions");

	if (setting)
	{
		setting_names = config_setting_lookup(setting, "name");

		if (config_setting_type(setting_names) != CONFIG_TYPE_ARRAY) {
			config_destroy(&cfg);
			return -1;
		}

		c = config_setting_length(setting_names);

		ind = 0;
		/* default exceptions */
		/* exceptionName[ind++] = "^Steam$"; */
		/* exceptionName[ind++] = "^[.]wine\\w*"; */
		exceptionName[ind] = (char *)malloc(100);
		strcpy(exceptionName[ind++], "^Steam$");
		exceptionName[ind] = (char *)malloc(100);
		strcpy(exceptionName[ind++], "^[.]wine\\w*");

		for (i = 0; i < c; i++) {
			const char *exc = config_setting_get_string_elem(setting_names, i);
			len = strlen(exc) + 1;
			exceptionName[ind] = (char *)malloc(len);
			if (exceptionName[ind]) {
				memcpy(exceptionName[ind++], exc, len);
			}
		}
	}

	config_destroy(&cfg);

	rmDupInArrOfPointers(exceptionName, ind);

	return ind;
}

int
readConfig(void)
{
	int n;

	printf("Reading config file...\n");

	if ((n=getConfigIndex()) == -1) {
		warn("No config file found");
		return 0;
	}

	if (readExceptions(confPathes[n]) == -1) {
		warn("Cant read exceptions");
	}

	return 1;
}
