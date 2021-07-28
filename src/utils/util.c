#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>
#include <regex.h>
#include <sys/stat.h>

#include "eprintf.h"
#include "util.h"
#include "list.h"

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

char *
cut(char *str, int width)
{
	char res[width+1];
	int len = strlen(str);

	if (len > 1 && len > width) {
		esnprintf(res, sizeof(char) * (width - 2),
				"%s", str);
		strcat(res, "...");
	} else {
		esnprintf(res, sizeof(res),
				"%s", str);
	}

	return bprintf("%s", res);
}

char **
pp_sort(char **pp, int check(const char *))
{
	int i, j;
	char **res = (char **)ecalloc(pp_length(pp)+1, sizeof(char **));
	for (i = j = 0; pp[i]; i++) {
		if (check(pp[i])) {
			res[j++] = pp[i];
		}
	}
	
	return res;
}

int
pp_length(char **pp)
{
	int ret = 0;

	if (pp) {
		while (*pp++)
			ret++;
	}

	return ret;
}

void
pp_nfree(char **pp, int n)
{
	while (n--) {
		free(pp[n]);
	}
	free(pp);
}

void
pp_free(char **pp)
{
	int n = pp_length(pp);
	while (n--) {
		free(pp[n]);
	}
	free(pp);
}

int
pp_delete_dup(char **pp, int n)
{ 
    if (n == 0 || n == 1)
        return n;
 
    char *uniq[n];
	char *current;

	int k = 0;
	int i, j;
	for (i = 0; i < n; i++) {
	   if ( pp[i] == NULL ) {
			continue;
		}
		current = pp[i];
		uniq[k] = current;
		k++;
		for (j = i+1; j < n; j++) {
			if (current == pp[j]) {
				pp[j] = NULL;
			}
		}
	}

	for (j = i = 0; i < n; i++) {
		if (i > k-1) {
			pp[i] = NULL;
		} else {
			pp[i] = uniq[j++];
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

	if (!g_exceptions) {
		return 0;
	}

	for ( ; g_exceptions; g_exceptions=g_exceptions->next) {
		status = regcomp(&regex, g_exceptions->value,
					REG_EXTENDED|REG_NEWLINE|REG_NOSUB);
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
isStartPoint(const char *file_path, const char *gameName)
{
	char *file_name;
	char sh[FILENAME_MAX], x86_64[FILENAME_MAX], x64[FILENAME_MAX], x86[FILENAME_MAX];

	file_name = basename((char*)file_path);

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

	if (isExecuteble(file_path)) {
		int i;
		for (i = 0; i < 21; i++) {
			if (strcasecmp(file_name, spPattern[i]) == 0) {
				return 1;
			}
		}
	}

	return 0;
}

int
isUninstaller(const char *file_path, const char *null)
{
	char *name;
	regex_t regex;
	name = basename((char *)file_path);

	int rc = regcomp(&regex, "uninsta", REG_EXTENDED|REG_NEWLINE|REG_NOSUB);

	if (rc) {
		warn("regcomp:");
		regfree(&regex);
		return -1;
	}

	rc = regexec(&regex, name, 0, NULL, 0);
	
	if (!rc) {
		regfree(&regex);
		return 1;
	}

	regfree(&regex);

	return 0;
}

int
isIcon(const char *file_path, const char *null)
{
	char *name;
	regex_t regex;
	name = basename((char *)file_path);

	int rc = regcomp(&regex, "icon", REG_ICASE|REG_EXTENDED|REG_NEWLINE|REG_NOSUB);

	if (rc) {
		warn("regcomp:");
		regfree(&regex);
		return -1;
	}

	rc = regexec(&regex, name, 0, NULL, 0);
	
	if (!rc) {
		regfree(&regex);
		return 1;
	}

	regfree(&regex);

	return 0;
}

char *
bprintf(const char *fmt, ...)
{
	va_list ap;
	int ret;

	va_start(ap, fmt);
	ret = evsnprintf(g_buf, sizeof(g_buf), fmt, ap);
	va_end(ap);

	return (ret < 0) ? NULL : g_buf;
}

char *
itoa(int val, int base)
{
	char buf[32] = {0};
	int i = 30;
	for(; val && i ; --i, val /= base)
		buf[i] = "0123456789abcdef"[val % base];
	return bprintf("%s", &buf[i+1]);
}

int
ctocp(char c, char *ret)
{
	if (sizeof(ret) < 2) {
		return -1;
	}

	ret[0] = c;
	ret[1] = '\0';

	return 0;
}

int
getCountsOfDigits(int n) {
	if (n < 100000) {
		if (n < 100) {
			if (n < 10) {
				return 1;
			} else {
				return 2;
			}
		} else {
			if (n < 1000) {
				return 3;
			} else {
				if (n < 10000) {
					return 4;
				} else {
					return 5;
				}
			}
		}
	} else {
		if (n < 10000000) {
			if (n < 1000000) {
				return 6;
			} else {
				return 7;
			}
		} else {
			if (n < 100000000) {
				return 8;
			} else {
				if (n < 1000000000) {
					return 9;
				} else {
					return 10;
				}
			}
		}
	}
}

void
swap(void *s, void *d, size_t size)
{
	unsigned char *p = s, *q = d, tmp;
	for (size_t i = 0; i != size; i++)
	{
		tmp  = p[i];
		p[i] = q[i];
		q[i] = tmp;
	}
}

void
shift_pp(void **pp, int i, int j, size_t size)
{
	while (i < j)
	{
		swap(pp+(i), pp+((i+1)), size);
		i++;
	}
}
