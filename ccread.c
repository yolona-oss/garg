#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <limits.h>
#include <libconfig.h>

#include "main.h"
#include "ccread.h"
#include "util.h"
#include "eprintf.h"
#include "gamerec.h"

/* func */
static char *findConfigFile(void);
static char *initCacheFile(void);
static int isCached(const char *path, Game_rec gr);
static void cache_trim(const char *path);
static int config_read_exeptions(const char *confPath);

/* vars */
struct Gr_tab gr_tab;

/* If gr aready exists in file, function
 * returns position of elem in file. */
static int
isCached(const char *path, Game_rec gr)
{
	int n, total;
	struct Game_rec *buf;

	total = cache_get_length(path);

	for (n = 0; n < total; n++) {
		buf = cache_get_record(path, n);
		if (buf && grcmp(gr, *buf) == 0) {
			grp_free(buf);
			return n+1;
		}

		grp_free(buf);
	}

	return 0;
}

static void
cache_trim(const char *path)
{
	struct Game_rec *buf;
	int n, crec;

	crec = cache_get_length(path);
	for (n = 0; n < crec; n++)
	{
		buf = cache_get_record(path, n);
		if (!buf) {
			continue;
		}

		if (isCached(path, *buf) != (n+1)) {
			cache_delete_record(path, n);

			if (!n) n--;
			crec--;
		}

		grp_free(buf);
	}
}

//Returns ~/.config/ga-org.conf if the file exists or
//of path specified by command line argument if the file exists.
static char *
findConfigFile(void)
{
	int len;
	char tmp[PATH_MAX];
	char *path = NULL;

	if (userConf) {
		if (!isExist(userConf)) {
			warn("Wrong config path: \"%s\". No such file", userConf);
		} else {
			len = strlen(userConf) + 1;
			path = (char *)emalloc(len);
			if (path) {
				memcpy(path, userConf, len);
			}

			return path;
		}
	}

	len = esnprintf(tmp, sizeof(tmp), "%s/%s", getenv("HOME"), ".config/ga-org.conf") + 1;
	if (isExist(tmp)) {
		path = (char *)emalloc(len);
		if (path) {
			memcpy(path, tmp, len);
		}
	}

	return path;
}

//Check cache file(~/cache/ga-org.conf) for existence
//Create if dont and add libconfig constructions
static char *
initCacheFile(void)
{
	char tmp[PATH_MAX];
	char *path;

	int len = esnprintf(tmp, sizeof(tmp), "%s/%s", getenv("HOME"), ".cache/ga-org.conf") + 1;
	path = (char *)emalloc(len);
	if (!path) {
		return NULL;
	}
	memcpy(path, tmp, len);

	if (!isExist(path)) {
		FILE *fd;
		if (!(fd=fopen(path, "w"))) {
			warn("fopen:");
			return NULL;
		}
		fclose(fd);
	}
	
	config_t cfg;
	config_setting_t *root, *games;

	config_init(&cfg);

	config_set_option(&cfg, _config_write_options, 0);

	if (!config_read_file(&cfg, path)) {
		_config_err(&cfg);
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

//Return record number "n"
//from cache->games group
//in file "path".
Game_rec *
cache_get_record(const char *path, int n)
{
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

	Game_rec *grp;
	grp = (Game_rec *)ecalloc(1, sizeof *grp + 1);
	if (!grp) {
		return NULL;
	}

	grp->id = id;
	grp->location = estrdup(location);
	grp->name = estrdup(name);
	grp->start_point = estrdup(sp);

	config_destroy(&cfg);

	return grp;
}

//Get number of elements
//cache->games group in file "path".
int
cache_get_length(const char *path)
{
	int c;

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

	c = 0;
	if (games) {
		c = config_setting_length(games);
	} else {
		_config_err(&cfg);
		return -1;
	}

	config_destroy(&cfg);

	return c;
}

int
cache_put_record(const char *path, struct Game_rec *GE)
{
	if (!GE) {
		warn("GR is not exist!");
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

	config_set_option(&cfg, _config_write_options, 0);
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
	
	return 0;
}

//Delete game record number "n"
//in file "path".
int
cache_delete_record(const char *path, int n)
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

	config_set_option(&cfg, _config_write_options, 0);
	config_set_tab_width(&cfg, 4);

	config_setting_remove_elem(games, n);

	config_write_file(&cfg, path);

	config_destroy(&cfg);

	return 0;
}

static int
config_read_exeptions(const char *confPath)
{
	int i, c, len, ind;
	config_t cfg;
	config_setting_t *setting, *setting_names;

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
			exceptionName[ind] = (char *)emalloc(len);
			if (exceptionName[ind]) {
				memcpy(exceptionName[ind++], exc, len);
			}
		}
	}

	config_destroy(&cfg);

	pp_delete_dup(exceptionName, ind);

	return ind;
}

int
readConfig(void)
{
	char *cfgPath;

	printf("Reading config file...\n"); fflush(stdout);

	if (!(cfgPath = findConfigFile())) {
		warn("No config file!");
		return -1;
	}

	if (config_read_exeptions(cfgPath) == -1) {
		warn("Cant read exceptions!");
	}

	free(cfgPath);

	return 1;
}

int
readCache()
{
	int n, c_rec;
	char *cchPath;
	Game_rec *GR;

	printf("Reading cache...\n"); fflush(stdout);

	if (!(cchPath=initCacheFile())) {
		warn("Cant read cache!");
		return -1;
	}

	cache_trim(cchPath);

	c_rec = cache_get_length(cchPath);
	for (n = 0; n < c_rec; n++)
	{
		GR = cache_get_record(cchPath, n);

		if (!GR) {
			continue;
		} else if (gr_is_dup(*GR)) {
			grp_free(GR);
			continue;
		}

		if (gr_is_broken(GR) == 0) {
			gr_add(*GR);
		} else {
			cache_delete_record(cchPath, n);
			grp_free(GR);

			if (!n) n--;
			c_rec--;

			//skiping free(GR)
			continue;
		}

		free(GR);
	}
	
	free(cchPath);

	return 1;
}

int
writeCache(void)
{
	int i;
	char *cchPath;

	printf("Writing cache...\n"); fflush(stdout);

	if (!(cchPath=initCacheFile())) {
		warn("Cant write cache.");
		return -1;
	}

	for (i = 0; i < gr_tab.ngames; i++)
	{
		if (isCached(cchPath, gr_tab.game_rec[i])) {
			continue;
		}

		cache_put_record(cchPath, &gr_tab.game_rec[i++]);
	}

	free(cchPath);

	return i;
}
