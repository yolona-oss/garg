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

char **exceptionName;
/* char *exceptionPath[MAX_EXCEPTIONS]; */
/* char *inclusions[MAX_INCLUSIONS]; */

int
isCached(const char *path, struct Game_rec *Game)
{
	int n, total;
	struct Game_rec *bufGE;

	total = readGameEntriesCountFromFile(path);

	for (n = 0; n < total; n++) {
		bufGE = readGameEntryFromFile(path, n);
		if (bufGE && gecmp(Game, bufGE) == 0) {
			freePSG(bufGE);
			return 1;
		}
		free(bufGE->location);
		free(bufGE->name);
		free(bufGE->start_point);
	}

	return 0;
}

void
trimCache(const char *path)
{
	struct Game_rec *g_buf;
	int cn = 0;

	printf(" [!!] TRIMING\n"); fflush(stdout);

	while ((g_buf=readGameEntryFromFile(path, cn)))
	{
		printf("LOOP\n"); fflush(stdout);
		if (isCached(path, g_buf)) {
			printf("deleting\n"); fflush(stdout);
			deleteGameEntryFromFile(path, cn);
		}
		free(g_buf->location);
		free(g_buf->name);
		free(g_buf->start_point);
		cn++;
	}
}

char *
findConfigFile(void)
{
	int len;
	char tmp[PATH_MAX];
	char *path = NULL;

	if (userConf && isExist(userConf)) {
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

	/* CAT_WITH_HOME(".cache/ga-org.conf", path, tmp); */
	int len = sprintf(tmp, "%s/%s", getenv("HOME"), ".cache/ga-org.conf") + 1;
	path = (char *)malloc(len);
	memcpy(path, tmp, len);

	config_t cfg;
	config_setting_t *root, *games;

	config_init(&cfg);

	config_set_option(&cfg, CONFIG_OPTION_COLON_ASSIGNMENT_FOR_GROUPS|
			CONFIG_OPTION_OPEN_BRACE_ON_SEPARATE_LINE|
			CONFIG_OPTION_FSYNC, 0);

	if (!config_read_file(&cfg, path)) {
		warn("Reading cache file: \"%s\" error:%d - %s", config_error_file(&cfg), config_error_line(&cfg), config_error_text(&cfg));
		return NULL;
	}

	root = config_root_setting(&cfg);

	if (!root) {
		warn("%s:%d - %s\n", config_error_file(&cfg), config_error_line(&cfg), config_error_text(&cfg));
		return NULL;
	}

	if (config_setting_lookup(root, "games")) {
		return path;
	}

	games = config_setting_add(root, "cache", CONFIG_TYPE_GROUP);
	config_setting_add(games, "games", CONFIG_TYPE_LIST);
	config_write_file(&cfg, path);

	config_destroy(&cfg);

	return path;
}

struct Game_rec *
readGameEntryFromFile(const char *path, int n)
{
	printf("READING GE from file\n"); fflush(stdout);
	const char *location, *name, *sp;
	int id;

	config_t cfg;
	config_setting_t *root, *games, *elem;

	config_init(&cfg);

	if (!config_read_file(&cfg, path)) {
		_config_err(&cfg);
		return NULL;
	}

	root = config_lookup(&cfg, "cache");

	if (!root) {
		_config_err(&cfg);
		return NULL;
	}

	games = config_setting_lookup(root, "games");

	if (!games) {
		_config_err(&cfg);
		return NULL;
	}

	printf(" read cache entry №: %d\n", n); fflush(stdout);
	elem = config_setting_get_elem(games, n);

	if (!elem) {
		_config_err(&cfg);
		return NULL;
	}

	if (! (config_setting_lookup_string(elem, "location", &location) &&
			config_setting_lookup_string(elem, "name", &name) &&
			config_setting_lookup_string(elem, "SP", &sp) &&
			config_setting_lookup_int(elem, "ID", &id))) {
		return NULL;
	}

	struct Game_rec *GE;
	int len;

	GE = (struct Game_rec *)calloc(1, sizeof(struct Game_rec *));
	GE->id = id;

	len = strlen(location) + 1;
	GE->location = (char *)calloc(len, sizeof(char));
	if (GE->location) memcpy(GE->location, location, len);
	else warn("malloc:");

	len = strlen(name) + 1;
	GE->name = (char *)calloc(len, sizeof(char));
	if (GE->name) memcpy(GE->name, name, len);
	else warn("malloc:");

	len = strlen(sp) + 1;
	GE->start_point = (char *)calloc(len, sizeof(char));
	if (GE->start_point) memcpy(GE->start_point, sp, len);
	else warn("malloc:");

	config_destroy(&cfg);

	return GE;
}

int
readGameEntriesCountFromFile(const char *path)
{
	printf("Reading entries count\n"); fflush(stdout);
	int c = 0;

	config_t cfg;
	config_setting_t *root, *games;

	config_init(&cfg);

	if (!config_read_file(&cfg, path)) {
		_config_err(&cfg);
		config_destroy(&cfg);
		return -1;
	}

	root = config_lookup(&cfg, "cache");

	if (!root) {
		_config_err(&cfg);
		return -1;
	}

	games = config_setting_lookup(root, "games");

	if (games) {
		c = config_setting_length(games);
	} else {
		_config_err(&cfg);
		return -1;
	}

	config_destroy(&cfg);
	printf("END Reading entries count: %d.\n", c);

	return c;
}

int
writeGameEntryToFile(const char *path, struct Game_rec *GE)
{
	printf("WRITING TO CACHE\n"); fflush(stdout);

	if (!GE) {
		warn("GE is not exist");
		return -1;
	}

	config_t cfg;
	config_setting_t *root, *games, *group, *entry;

	config_init(&cfg);

	if (!config_read_file(&cfg, path)) {
		_config_err(&cfg);
		return -1;
	}

	root = config_lookup(&cfg, "cache");

	if (!root) {
		_config_err(&cfg);
		return -1;
	}

	games = config_setting_lookup(root, "games");

	if (!games) {
		_config_err(&cfg);
		return -1;
	}

	config_set_option(&cfg, CONFIG_OPTION_COLON_ASSIGNMENT_FOR_GROUPS|
			CONFIG_OPTION_OPEN_BRACE_ON_SEPARATE_LINE|
			CONFIG_OPTION_FSYNC, 0);
	config_set_tab_width(&cfg, 4);

	group = config_setting_add(games, NULL, CONFIG_TYPE_GROUP);

	if (!group) {
		_config_err(&cfg);
		return -1;
	}

	if ((entry = config_setting_add(group, "ID", CONFIG_TYPE_INT)))
		config_setting_set_int(entry, GE->id);

	if ((entry = config_setting_add(group, "name", CONFIG_TYPE_STRING)))
		config_setting_set_string(entry, GE->name);

	if ((entry = config_setting_add(group, "location", CONFIG_TYPE_STRING)))
		config_setting_set_string(entry, GE->location);

	if ((entry = config_setting_add(group, "SP", CONFIG_TYPE_STRING)))
		config_setting_set_string(entry, GE->start_point);

	config_write_file(&cfg, path);

	config_destroy(&cfg);
	
	printf(" [SS] WR SUCC\n");

	return 0;
}

int
deleteGameEntryFromFile(const char *path, int n)
{
	config_t cfg;
	config_setting_t *root, *games;
	
	config_init(&cfg);

	if (!config_read_file(&cfg, path)) {
		_config_err(&cfg);
		config_destroy(&cfg);
		return -1;
	}

	root = config_lookup(&cfg, "cache");

	if (!root) {
		_config_err(&cfg);
		return -1;
	}

	games = config_setting_lookup(root, "games");

	if (!games) {
		_config_err(&cfg);
		return -1;
	}

	config_set_option(&cfg, CONFIG_OPTION_COLON_ASSIGNMENT_FOR_GROUPS|
			CONFIG_OPTION_OPEN_BRACE_ON_SEPARATE_LINE|
			CONFIG_OPTION_FSYNC, 0);
	config_set_tab_width(&cfg, 4);

	config_setting_remove_elem(games, n);

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

	exceptionName = (char **)calloc(MAX_EXCEPTIONS+1, sizeof(char));

	ind = 0;
	/* default exceptions */
	exceptionName[ind] = (char *)calloc(9, sizeof(char));
	strcpy(exceptionName[ind++], "^Steam$");
	exceptionName[ind] = (char *)calloc(13, sizeof(char));
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

	rmDupInPP(exceptionName, ind);

	return ind;
}

int
readConfig(void)
{
	char *cfgPath;

	printf("Reading config file...\n"); fflush(stdout);

	if (!(cfgPath = findConfigFile())) {
			return -1;
	}

	if (readExceptions(cfgPath) == -1) {
		warn("Cant read exceptions");
	}

	free(cfgPath);

	return 1;
}

struct Game_rec **
readCache(struct Game_rec **Games)
{
	int n, id, count;
	char *cchPath;
	struct Game_rec *GE;

	printf("Reading cache...\n"); fflush(stdout);

	if (!(cchPath=initCacheFile())) {
		warn("Cant init cache file.");
		return NULL;
	}

	count = readGameEntriesCountFromFile(cchPath);
	for (id = n = 0; n < count; n++)
	{
		GE = readGameEntryFromFile(cchPath, n);

		printf("Adding\n"); fflush(stdout);

		if (!GE) {
			printf(" [EE] NULL GE\n");
			continue;
		}

		if (!isGameEntryUniq(Games, GE)) {
			printf(" skipping exist entry");
			continue;
		}

		if (isBrokenGameEntry(GE) == 0) {
			printf(" [SS] True\n");
			Games[id] = (struct Game_rec *)calloc(1, sizeof(struct Game_rec));
			if (addGameEntry(Games[id], GE->name, GE->location, GE->start_point) == 3)
				id++;
		} else {
			printf(" Deleting game entry form cache\n"); fflush(stdout);
			deleteGameEntryFromFile(cchPath, n);
			if (!n)
				n--;
			count--;
		}

		freePSG(GE);
	}

	printf("End reading cache.\n");

	printf("FOUNd in CACHE:");
	for (int i = id-1; i >= 0; i--)
		printGameEntry(Games[i]);
	printf("END FOUNT IN CACHE.\n\n");

	free(cchPath);
	
	return Games;
}

int
writeCache(struct Game_rec **Games)
{
	int i, count, uniq;
	char *cchPath;

	printf("Writing cache...\n"); fflush(stdout);

	if (!(cchPath=initCacheFile())) {
		warn("Cant init cache file.");
		return -1;
	}

	i = 0;
	while (Games[i]->start_point)
	{
		uniq = 1;
		printf("cache read loop\n");

		if (isCached(cchPath, Games[i])) {
			continue;
		}

		writeGameEntryToFile(cchPath, Games[i++]);
	}

	free(cchPath);

	return i;
}
