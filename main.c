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
#include "ccread.h"

struct game Game[MAX_GAMES+1];

static char *confPathes[3];
static char *exceptionName[MAX_EXCEPTIONS];
/* static char *exceptionPath[MAX_EXCEPTIONS]; */
/* static char *inclusions[MAX_INCLUSIONS]; */

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
		/* exceptionName[ind] = (char *)malloc(100); */
		/* strcpy(exceptionName[ind++], "^Steam$"); */
		/* exceptionName[ind] = (char *)malloc(100); */
		/* strcpy(exceptionName[ind++], "^[.]wine\\w*"); */

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

char **
getFileList(const char *path, int *count)
{
	struct dirent **namelist;
	char **ret;

	char rpath[PATH_MAX];
	int n = 0, c;

	printf("      [II] getting file list\n"); fflush(stdout);

	n = scandir(path, &namelist, NULL, alphasort);
	if (n == -1) {
		warn("scandir:");
		return NULL;
	}

	printf("size - %d\n", n); fflush(stdout);
	ret = (char **)malloc(n+1);
	if (!ret) {
		warn("malloc:");
		return NULL;
	}

	int tmp = n;
	while (tmp--) {
		printf(" == %s\n", namelist[tmp]->d_name);
	}

	c = 0;
	while (n--)
	{
		if (!isDotName(namelist[n]->d_name) && getRPath(namelist[n]->d_name, path, rpath)) {
			printf("+"); fflush(stdout);
			if ((ret[c] = (char *)malloc(strlen(rpath)+1))) {
				strcpy(ret[c++], rpath);
			} else {
				warn("malloc:");
			}
		}

		free(namelist[n]);
	}
	free(namelist);
	printf("\n"); fflush(stdout);

	*count = c;

	printf("      [II] |%d| getted\n", c);

	return ret;
}

char *
searchSP(const char *location)
{
	int ecount, i, stat;
	char *sp = NULL;
	char **list;

	char **dirs = NULL;

	int i_d;
	i_d = 0;

	printf("    [II] Scanning %s\n", location);

	if (!(dirs = (char **)malloc(1000))) {
		warn("malloc:");
		return NULL;
	}
	
	list = getFileList(location, &ecount);

	for (i = 0; i < ecount; i++)
	{
		printf("      [II] Checking file %s\n", list[i]); fflush(stdout);
		if (!isExcludeName(list[i]))
		{
			/* printf("HEAR\n"); fflush(stdout); */
			if ((stat = isDirectory(list[i]))) {
				/* printf("      [II] Adding dir to array "); fflush(stdout); */
				/* if ((dirs[i_d] = (char *)malloc(strlen(list[i]+1)))) { */
				/* 	strcpy(dirs[i_d++], list[i]); */
				/* } else { */
				/* 	warn("malloc:"); */
				/* } */
				/* printf("%s\n", dirs[i_d-1]); fflush(stdout); */
			} else if (stat == 0) {
				printf("       [@@] Checking - \"%s\"\n", list[i]); fflush(stdout);

				if (isStartPoint(list[i])) {
					printf("     [SS] Congrat!!\n"); fflush(stdout);

					if ((sp = (char *)malloc(strlen(list[i]+1)))) {
						strcpy(sp, list[i]);
					} else {
						warn("malloc:");
					}
					break;
				}
			}
		}
	}

	freePP(list, ecount);
	freePP(dirs, i_d);

	/* if (!sp) { */
	/* 	printf("    [II] Start searching in Subdir\n"); */
	/* 	for (i = 0; i < i_d; i++) { */
	/* 		sp = searchSP(dirs[i]); */
	/* 	} */
	/* } */

	/* printf("   [II] End scanning\n"); */

	return sp;
}

//try use nftw instead loop
int
findGames(const char *path, int id)
{
	char rpath[PATH_MAX];
	char *startPoint;

	DIR *d;
	struct dirent *dir;

	printf("Finding games...\n");

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
			printf(":: Start finding in: %s\n", rpath); fflush(stdout);
			gameName = basename(rpath);
			startPoint = searchSP(rpath);
			if (startPoint) {
				/* printf("  -- Found sp: %s\n", startPoint); fflush(stdout); */
				editGameEntry(id, gameName, rpath, startPoint);
				printGameEntry(id);
				id++;
			}

			free(startPoint);
		}
	}

	if (closedir(d) == -1) {
		warn("Cant close dir: %s:", path);
		return -1;
	}

	return id - 1;
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
	/* readedGameEntries = readCache("/home/xewii/.cache/ga-org.conf"); */
	readedGameEntries = 0;
	gc = findGames(path, readedGameEntries);
	/* writeCache("/home/xewii/.cache/ga-org.conf"); */

	for (int i = 0; i < gc; i++) {
		printf("id          - %d\ngame        - %s\nlocation    - %s\nstart point - %s\n\n", i, Game[i].name, Game[i].location, Game[i].starPoint);
		fflush(stdout);
	}

	return gc;
}

void
usage(void)
{
	die("[-c|--config] <DIR>(<DIR>...)");
}

int main(int argc, char **argv)
{
	char *userConf = NULL;
	char tmp[PATH_MAX];

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

	sprintf(tmp, "%s/%s", getenv("HOME"), ".ga-org.conf");
	confPathes[0] = tmp;

	sprintf(tmp, "%s/%s", getenv("HOME"), ".config/ga-org.conf");
	confPathes[1] = tmp;

	confPathes[2] = userConf;

	//add multi path option
	scan((*argv) ? *argv : ".");

	return 0;
}
