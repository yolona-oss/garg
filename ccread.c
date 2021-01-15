#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <limits.h>
#include <libconfig.h>

#include "global.h"
#include "ccread.h"
#include "util.h"

char *
findConfigFile(void)
{
	int len;
	char tmp[PATH_MAX];
	char *path = NULL;

	if (userConf) {
		len = strlen(userConf) + 1;
		path = (char *)malloc(len);
		memcpy(path, userConf, len);
	} else {
		len = sprintf(tmp, "%s/%s", getenv("HOME"), ".config/ga-org.conf") + 1;
		if (isExist(tmp)) {
			path = (char *)malloc(len);
			memcpy(path, tmp, len);
		}
	}

	return path;
}

char *
initCacheFile(void)
{
	char tmp[PATH_MAX];
	char *path;

	CAT_HOME(".cache/ga-org.conf", path, tmp);
	if (!isExist(path)) {
		FILE *fd;

		if (!(fd=fopen(path, "w"))) {
			warn("fopen:");
			return NULL;
		}
		fclose(fd);

		config_t cfg;
		config_setting_t *root, *app;

		config_init(&cfg);

		config_set_option(&cfg, CONFIG_OPTION_COLON_ASSIGNMENT_FOR_GROUPS|
				CONFIG_OPTION_OPEN_BRACE_ON_SEPARATE_LINE|
				CONFIG_OPTION_FSYNC, 0);

		if (!config_read_file(&cfg, path)) {
			warn("Reading cache file: \"%s\" error:%d - %s", config_error_file(&cfg), config_error_line(&cfg), config_error_text(&cfg));
			config_destroy(&cfg);
			return NULL;
		}

		root = config_root_setting(&cfg);

		if (!root) {
			warn("%s:%d - %s\n", config_error_file(&cfg), config_error_line(&cfg), config_error_text(&cfg));
			config_destroy(&cfg);
			return NULL;
		}

		app = config_setting_add(root, "all", CONFIG_TYPE_GROUP);
		config_setting_add(app, "games", CONFIG_TYPE_LIST);

		config_write_file(&cfg, path);

		config_destroy(&cfg);
	}

	return path;
}

struct game *
readGameEntryFromFile(const char *path, int id)
{
	struct game *GE;
	const char *location, *name, *sp;

	config_t cfg;
	config_setting_t *setting, *game_setting;

	config_init(&cfg);

	if (!config_read_file(&cfg, path)) {
		warn("Reading cache file: \"%s\" error:%d - %s", config_error_file(&cfg), config_error_line(&cfg), config_error_text(&cfg));
		config_destroy(&cfg);
		return NULL;
	}

	setting = config_lookup(&cfg, "games");

	if (!setting) {
		warn("Cant get setting of games list");
		return NULL;
	}

	game_setting = config_setting_get_elem(setting, id);

	if (!game_setting) {
		warn("Cant get setting of game entry");
		return NULL;
	}

	if(! (config_setting_lookup_string(game_setting, "location", &location) &&
			config_setting_lookup_string(game_setting, "name", &name) &&
			config_setting_lookup_string(game_setting, "SP", &sp) &&
			config_setting_lookup_int(game_setting, "ID", &id))) {
		return NULL;
	}

	GE->id = id;
	editGameEntry(id, name, location, sp);

	config_destroy(&cfg);

	return GE;
}

void
writeGameEntryToFile(const char *path, config_setting_t *stt, struct game GE)
{
	/* config_t cfg; */
	config_setting_t *sTmp;

	/* config_init(&cfg); */

	/* stt = config_lookup(&cfg, "games"); */

	/* config_set_option(&cfg, CONFIG_OPTION_COLON_ASSIGNMENT_FOR_GROUPS| */
	/* 		CONFIG_OPTION_OPEN_BRACE_ON_SEPARATE_LINE| */
	/* 		CONFIG_OPTION_FSYNC, 0); */
	/* config_set_tab_width(&cfg, 4); */

	if (stt) {
		if ((sTmp = config_setting_add(stt, "ID", CONFIG_TYPE_INT)))
			config_setting_set_int(sTmp, GE.id);

		if ((sTmp = config_setting_add(stt, "name", CONFIG_TYPE_STRING)))
			config_setting_set_string(sTmp, GE.name);

		if ((sTmp = config_setting_add(stt, "location", CONFIG_TYPE_STRING)))
			config_setting_set_string(sTmp, GE.location);

		if ((sTmp = config_setting_add(stt, "SP", CONFIG_TYPE_STRING)))
			config_setting_set_string(sTmp, GE.starPoint);
	}

	/* config_write_file(&cfg, path); */

	/* config_destroy(&cfg); */
}

int
readGameEntriesCountFromFile(const char *path)
{
	int c = 0;
	config_t cfg;
	config_setting_t *games;

	config_init(&cfg);

	games = config_lookup(&cfg, "games");
	if (games) {
		c = config_setting_length(games);
	}

	config_destroy(&cfg);

	return c;
}

int
deleteGameEntryFromFile(const char *path, int n)
{
	config_t cfg;
	config_setting_t *stt;
	
	config_init(&cfg);

	config_set_option(&cfg, CONFIG_OPTION_COLON_ASSIGNMENT_FOR_GROUPS|
			CONFIG_OPTION_OPEN_BRACE_ON_SEPARATE_LINE|
			CONFIG_OPTION_FSYNC, 0);
	config_set_tab_width(&cfg, 4);

	stt = config_lookup(&cfg, "games");
	config_setting_remove_elem(stt, n);

	config_write_file(&cfg, path);

	config_destroy(&cfg);

	return 0;
}

int
readExceptions(const char *confPath)
{
	int i, c, len, ind;
	config_t cfg;
	config_setting_t *setting, *setting_names;

	exceptionName = (char **)malloc(MAX_EXCEPTIONS+1);

	ind = 0;
	/* default exceptions */
	exceptionName[ind] = (char *)malloc(strlen("^Steam$")+1);
	strcpy(exceptionName[ind++], "^Steam$");
	exceptionName[ind] = (char *)malloc(strlen("^[.]wine\\w*")+1);
	strcpy(exceptionName[ind++], "^[.]wine\\w*");

	config_init(&cfg);

	if (!config_read_file(&cfg, confPath)) {
		warn("Cant read config file: %s", confPath);
		config_destroy(&cfg);
		return -1;
	}

	setting = config_lookup(&cfg, "exceptions");

	if (setting)
	{
		setting_names = config_setting_lookup(setting, "name");

		if (config_setting_type(setting_names) != CONFIG_TYPE_LIST) {
			warn("config file format error");
			config_destroy(&cfg);
			return -1;
		}

		c = config_setting_length(setting_names);

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
	char *cfgPath;

	printf("Reading config file...\n");

	if (!(cfgPath = findConfigFile())) {
			return -1;
	}

	if (readExceptions(cfgPath) == -1) {
		warn("Cant read exceptions");
	}

	free(cfgPath);

	return 1;
}

int
readCache()
{
	int n, id, count;
	char *cchPath;
	struct game *GE;

	printf("Reading cache...\n");

	if (!(cchPath=initCacheFile())) {
		warn("Cant init cache file.");
		return -1;
	}

	count = readGameEntriesCountFromFile(cchPath);
	for (id = n = 0; n < count; n++)
	{
		GE = readGameEntryFromFile(cchPath, n);
		editGameEntry(id, GE->name, GE->location, GE->starPoint);

		if (checkGameEntry(id) == 0) {
			id++;
		} else {
			deleteGameEntryFromFile(cchPath, n);
			wipeGameEntry(id);
			if (!n)
				n--;
			count--;
		}

		freeSG(GE);
	}
	
	return id;
}

int
writeCache()
{
	int i = 0;
	char *cchPath;

	struct game *GE;

	printf("Writing cache...\n");

	if (!(cchPath=initCacheFile())) {
		warn("Cant init cache file.");
		return -1;
	}

	config_t cfg;
	config_setting_t *setting, *sEntry;

	config_init(&cfg);

	if (!config_read_file(&cfg, cchPath)) {
		warn("Reading cache file: \"%s\" error:%d - %s", config_error_file(&cfg), config_error_line(&cfg), config_error_text(&cfg));
		config_destroy(&cfg);
		return -1;
	}

	setting = config_lookup(&cfg, "games");

	if (!setting) {
		warn("Cant find games entries %s", cchPath);
		return -1;
	}

	int uniq;
	while (Game[i].starPoint)
	{
		uniq = 1;
		printf("cache read loop\n");

		//check unicality
		for (int f = 0; f < readGameEntriesCountFromFile(cchPath); f++) {
			GE = readGameEntryFromFile(cchPath, f);
			if (gecmp(*GE, Game[i]) == 0) {
				uniq = 0;
				freeSG(GE);
				break;
			}
			freeSG(GE);
		}

		if (!uniq) {
			continue;
		}

		printf("WRITING TO CACHE\n");
		writeGameEntryToFile(cchPath, setting, Game[i]);
		i++;
	}

	config_write_file(&cfg, cchPath);

	free(cchPath);
	config_destroy(&cfg);

	return i;
}
