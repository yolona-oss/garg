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
#include "util.h"

struct game {
	int id;
	char *name;
	char *location;
	char *starPoint;
} Game[MAX_GAMES+1];

static char *confPathes[3];
static char *exceptionName[MAX_EXCEPTIONS];
static char *exceptionPath[MAX_EXCEPTIONS];
static char *inclusions[MAX_INCLUSIONS];

int
isExist(const char *path)
{
	if (access(path, F_OK ) == 0) {
		return 1;
	}

	return 0;
}

int
isDirectory(const char *path)
{
	struct stat fileStat;

	if (stat(path, &fileStat) == -1) {
		warn("Cant get stats of file: %s:", path);
		return 0;
	}

	if (S_ISDIR(fileStat.st_mode)) {
		return 1;
	}

	return 0;
}

int
isOtherDirectory(const char *root, const char *path)
{
	char *fileName;

	fileName = basename(strdup(path));

	if (!isDirectory(path)) {
		return 0;
	}

	if (isDotName(path) || strcmp(root, path) == 0) {
		return 0;
	}

	return 1;
}

int
isDotName(const char *name)
{
	if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
		return 1;
	}

	return 0;
}

int
isExecuteble(const char *path)
{
	if (access(path, X_OK) == 0) {
		return 1;
	}

	return 0;
}

int
isExcludeName(const char *path)
{
	int status;
	char *name;
	regex_t regex;

	name = basename(strdup(path));

	for (int i = 0; exceptionName[i]; i++) {
		status = regcomp(&regex, exceptionName[i], REG_EXTENDED|REG_NEWLINE|REG_NOSUB);

		if (status) {
			warn("regcomp:");
			regfree(&regex);
			continue;
		}

		status = regexec(&regex, name, 0, NULL, 0);
		
		if (!status) {
			regfree(&regex);
			return 1;
		}

		regfree(&regex);
	}

	return 0;
}

int
getRPath(const char *fileName, const char *root, char *rpath)
{
	char path[PATH_MAX];

	if ((snprintf(path, sizeof(path),
			"%s/%s", root, fileName)) < 1) {
		warn("Cant fill path:");
		return 0;
	}

	if (!realpath(path, rpath)) {
		warn("realpath: \"%s\":", path);
		return 0;
	}

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
	int i, c, ind = 0;
	config_t cfg;
	config_setting_t *setting, *setting_names;

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

		if (config_setting_type(setting_names) != CONFIG_TYPE_ARRAY) {
			config_destroy(&cfg);
			return -1;
		}

		c = config_setting_length(setting_names);

		ind = 0;
		/* default exceptions */
		exceptionName[ind++] = "^Steam$";
		exceptionName[ind++] = "^[.]wine\\w*";

		for (i = 0; i < c; i++) {
			exceptionName[ind++] = strdup(config_setting_get_string_elem(setting_names, i));
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

int
checkStartPoint(int id, const char *filePath)
{
	int i;
	char *fileName;
	char sh[FILENAME_MAX], x86_64[FILENAME_MAX], x64[FILENAME_MAX], x86[FILENAME_MAX];

	fileName = basename(strdup(filePath));

	if (sprintf(sh, "%s.sh", Game[id].name) < 0)         { warn("sprintf:"); }
	if (sprintf(x86_64, "%s.x86_64", Game[id].name) < 0) { warn("sprintf:"); }
	if (sprintf(x64, "%s.x64", Game[id].name) < 0)       { warn("sprintf:"); }
	if (sprintf(x86, "%s.x86", Game[id].name) < 0)       { warn("sprintf:"); }

	char *spPattern[21] = { "start.sh", "start",
		"run.sh", "run-game.sh", "rungame.sh",
		"run", "runit", "run-game", "rungame",
		"runme", "runme.sh",
		Game[id].name, sh, x86_64, x64, x86,
		"launcher", "launcher.sh", "launcher.x64", "launcher.x86_64", "launcher.x86" };

	if (isExecuteble(filePath)) {
		for (i = 0; i < 14; i++) {
			if (strcasecmp(fileName, spPattern[i]) == 0) {
				editGameEntry(id, NULL, NULL, strdup(filePath));
				return 1;
			}
		}
	}

	return 0;
}

//try use nftw instead loop
int
searchGameStartPoint(int id, const char *location)
{
	char rpath[PATH_MAX];

	DIR *d;
	struct dirent *dir;

	if (!(d = opendir(location))) {
		warn("FATAL. Cant open dir: %s", location);
		return -1;
	}

	while ((dir = readdir(d)))
	{	
		if (isDotName(dir->d_name) || !getRPath(dir->d_name, location, rpath)
				|| isExcludeName(rpath)) {
			continue;
		}

		if (!isDirectory(rpath)) {
			if (checkStartPoint(id, rpath)) {
				return 1;
			}
		}
		else if (isOtherDirectory(location, rpath)) {
			searchGameStartPoint(id, rpath);
		}
	}

	if (closedir(d) == -1) {
		warn("FATAL. Cant close dir: %s", location);
		return -1;
	}
	
	return 0;
}

int
findGameStartPoint(int gc)
{
	int i;

	printf("Finding start point...\n");

	for (i = 0; i <= gc; i++)
	{
		if (searchGameStartPoint(i, Game[i].location) != 1) {
			/* printf("No start point found in \"%s\"\n", Game[i].location); */
			;
		}

		printf("\r%d/%d", i, gc);
		fflush(stdout);
	}

	return i;
}

int
findGameLocations(const char *path, int id)
{
	char rpath[PATH_MAX];

	DIR *d;
	struct dirent *dir;

	printf("Radding locations of games...\n");

	if (!(d = opendir(path))) {
		warn("Cant open dir: %s:", path);
		return -1;
	}

	while ((dir = readdir(d)))
	{
		if (isDotName(dir->d_name) || !getRPath(dir->d_name, path, rpath)
				|| isExcludeName(rpath)) {
			continue;
		}

		if (isDirectory(rpath)) {
			/* if (searchGameStartPoint(id, rpath) == 1) { */
				editGameEntry(id, basename(rpath), rpath, NULL);
				id++;
			/* } */
		}
	}

	if (closedir(d) == -1) {
		warn("Cant close dir: %s:", path);
		return -1;
	}

	return id - 1;
}

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
		
	if (!checkStartPoint(id, Game[id].starPoint)) {
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

	printf("HEAR\n"); fflush(stdout);

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
	if (!initCacheFile(cachePath)) {
		warn("Cant init cache file: \"%s\"", cachePath);
		return -1;
	}

	config_t cfg;
	config_setting_t *setting;

	config_init(&cfg);

	if (!config_write_file(&cfg, cachePath)) {
		warn("Cant write cache:");
		config_destroy(&cfg);
		return -1;
	}

	config_destroy(&cfg);

	return 1;
}

int
editGameEntry(int id, const char *name, const char *location, const char *startPoint)
{
	if (id < 0 || id > MAX_GAMES) {
		warn("ID of game entry is out of range");
		return -1;
	}

	if (name) {
		Game[id].name = strdup(name);
	}

	if (location) {
		Game[id].location = strdup(location);
	}

	if (startPoint) {
		Game[id].starPoint = strdup(startPoint);
	}

	return 1;
}

int
scan(const char *path)
{
	int gc, readedGameEntries;
	char rpath[PATH_MAX];

	if (!getRPath("", path, rpath)) {
		return -1;
	}

	if(!isDirectory(rpath)) {
		warn("\"%s\": its not directory", rpath);
		return -1;
	}

	readConfig();
	readedGameEntries = readCache("/home/xewii/.cache/ga-org.conf");
	if ((gc = findGameLocations(path, readedGameEntries)) <= 0) { 
		warn("No one dir were found in: \"%s\"", path);
	}
	findGameStartPoint(gc);
	/* writeCache("/home/xewii/.cache/ga-org.conf"); */

	for (int i = 0; i < gc; i++) {
		printf("id          - %d\ngame        - %s\nlocation    - %s\nstart point - %s\n\n", i, Game[i].name, Game[i].location, Game[i].starPoint);
	}

	return gc;
}

static void
usage(void)
{
	die("[-c|--config] <DIR>(<DIR>...)");
}

int
main(int argc, char **argv)
{
	char *userConf = NULL;

	argv0 = *argv;
	argv++;

	for (int i = 0; *argv && (*argv)[0] == '-' && (*argv)[1]; i++, argc--, argv++)
	{
		if ((*argv)[1] == '-') {
			*argv += 2;
		} else {
			(*argv)++;
		}
			if (strcmp(*argv, "c") == 0) {
				userConf = *(++argv);
			} else if (strcmp(*argv, "config") == 0) {
				userConf = *(++argv);
			} else if (strcmp(*argv, "h") == 0) {
				usage();
			} else if (strcmp(*argv, "help") == 0){
				usage();
			} else {
				warn("Unknown argument: \"%s\"", *argv);
				usage();
			}
	}

	if (! *argv) {
		warn("No directory path specified");
		usage();
	}

	char tmp[PATH_MAX];

	sprintf(tmp, "%s/%s", getenv("HOME"), ".ga-org.conf");
	confPathes[0] = tmp;

	sprintf(tmp, "%s/%s", getenv("HOME"), ".config/ga-org.conf");
	confPathes[1] = tmp;

	confPathes[2] = userConf;

	//add multi path option
	scan(*argv);

	return 0;
}
