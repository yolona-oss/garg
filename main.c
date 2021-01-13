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
	/* char name[FILENAME_MAX]; */
	/* char location[PATH_MAX]; */
	/* char starPoint[PATH_MAX]; */
} Game[MAX_GAMES];

static char *confPathes[3];
static char *exceptionName[MAX_EXCEPTIONS]; //[FILENAME_MAX];
static char *exceptionPath[MAX_EXCEPTIONS];

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
		warn("Cant get stats of file: %s", path);
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

	fileName = basename((char *)path);

	if (!isDirectory(path)) {
		return 0;
	}

	if (strcmp(fileName, ".") == 0 || strcmp(fileName, "..") == 0 ||
			strcmp(root, path) == 0) {
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
isExcludeName(const char *name)
{
	int status;
	regex_t regex;

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

	if ((snprintf(path, sizeof(char) * PATH_MAX,
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

	for (i = 2; i > -1; i--) {
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
		exceptionName[ind++] = "^Steam$";

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
	char sh[FILENAME_MAX+3], x86_64[FILENAME_MAX+7], x64[FILENAME_MAX+4], x86[FILENAME_MAX+4];

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
				sprintf(Game[id].starPoint, "%s", filePath);
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
		if (isDotName(dir->d_name) || isExcludeName(dir->d_name) ||
				!getRPath(dir->d_name, location, rpath)) {
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
findGameLocations(const char *path)
{
	int gc;
	char rpath[PATH_MAX];

	DIR *d;
	struct dirent *dir;

	printf("Radding locations of games...\n");

	if (!(d = opendir(path))) {
		warn("FATAL. Cant open dir: %s", path);
		return -1;
	}

	gc = 0;
	while ((dir = readdir(d)))
	{	
		if (isDotName(dir->d_name) || isExcludeName(dir->d_name) ||
				!getRPath(dir->d_name, path, rpath)) {
			continue;
		}

		if (isDirectory(rpath)) {
			Game[gc].id = gc;
			sprintf(Game[gc].location, "%s", rpath);
			sprintf(Game[gc].name, "%s", basename(rpath));
			gc++;
		}
	}

	if (closedir(d) == -1) {
		warn("FATAL. Cant close dir: %s", path);
		return -1;
	}

	return gc - 1;
}

int
readGameEntryFromCache(const char *cachePath)
{

	return 1;
}

int
writeGameEntryToCache(const char *cachePath)
{

	return 1;
}

int
editGameEntry(int id, const char *name, const char *location, const char *startPoint)
{
	if (id < 0) {
		warn("ID of game entry is out of range");
		return -1;
	}

	if (name) {
		sprintf(Game[id].name, "%s", name);
	}

	if (location) {
		sprintf(Game[id].location, "%s", location);
	}

	if (startPoint) {
		sprintf(Game[id].starPoint, "%s", startPoint);
	}

	return 1;
}

int
scan(const char *path)
{
	int gc;
	char rpath[PATH_MAX];

	if (!getRPath("", path, rpath)) {
		return -1;
	}

	if(!isDirectory(rpath)) {
		warn("\"%s\": its not directory", rpath);
		return -1;
	}

	readConfig();
	if ((gc = findGameLocations(path)) <= 0) { 
		warn("No one dir were found in: \"%s\"", path);
	}
	findGameStartPoint(gc);

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
