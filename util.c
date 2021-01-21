#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>
#include <regex.h>
#include <sys/stat.h>

#include "eprintf.h"
#include "util.h"

char *
cat_fnames(const char *f, const char *s)
{
	if (!f && s) {
		return estrdup(s);
	} else if (!s && f) {
		return estrdup(f);
	} else if (!s && !f) {
		return NULL;
	}

	int len = strlen(f) + strlen(s) + 2;
	char path[len];

	esnprintf(path, len, "%s/%s", f, s);

	return estrdup(path);
}

void
pp_free(char **pa, int n)
{
	while (n--) {
		free(pa[n]);
	}
	free(pa);
}

int
pp_delete_dup(char **pa, int n)
{ 
    if (n == 0 || n == 1)
        return n;
 
    char *uniq[n];
	char *current;

	int k = 0;
	int i, j;
	for (i = 0; i < n; i++) {
	   if ( pa[i] == NULL ) {
			continue;
		}
		current = pa[i];
		uniq[k] = current;
		k++;
		for (j = i+1; j < n; j++) {
			if (current == pa[j]) {
				pa[j] = NULL;
			}
		}
	}

	for (j = i = 0; i < n; i++) {
		if (i > k-1) {
			pa[i] = NULL;
		} else {
			pa[i] = uniq[j++];
		}
	}
 
    return k-1;
}

int
isExist(const char *path)
{
	FILE *fd;
	if ((fd=fopen(path, "r"))) {
		fclose(fd);
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
	if (!isDirectory(path)
			|| isDotName(path)
			|| !strcmp(root, path)) {
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

	name = basename((char*)path);

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
isStartPoint(const char *filePath, const char *gameName)
{
	int i;
	char *fileName;
	char sh[FILENAME_MAX], x86_64[FILENAME_MAX], x64[FILENAME_MAX], x86[FILENAME_MAX];

	fileName = basename((char*)filePath);

	esnprintf(sh, sizeof(sh), "%s.sh", gameName);
	esnprintf(x86_64, sizeof(x86_64), "%s.x86_64", gameName);
	esnprintf(x64, sizeof(x64), "%s.x64", gameName);
	esnprintf(x86, sizeof(x86), "%s.x86", gameName);

	char *spPattern[21] = { "start.sh", "start",
		"run.sh", "run-game.sh", "rungame.sh",
		"run", "runit", "run-game", "rungame",
		"runme", "runme.sh",
		(char *)gameName, sh, x86_64, x64, x86,
		"launcher", "launcher.sh", "launcher.x64", "launcher.x86_64", "launcher.x86" };

	if (isExecuteble(filePath)) {
		for (i = 0; i < 21; i++) {
			if (strcasecmp(fileName, spPattern[i]) == 0) {
				return 1;
			}
		}
	}

	return 0;
}

int
pp_get_len(char **pp)
{
	int ret = 0;

	if (pp) {
		while (*pp++)
			ret++;
	}

	return ret;
}

