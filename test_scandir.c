#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <stdarg.h>

void
verr(const char *fmt, va_list ap)
{
	if (strncmp(fmt, "usage", sizeof("usage") - 1)) {
		fprintf(stderr, "%s: ", "prog");
	}

	vfprintf(stderr, fmt, ap);

	if (fmt[0] && fmt[strlen(fmt) - 1] == ':') {
		fputc(' ', stderr);
		perror(NULL);
	} else {
		fputc('\n', stderr);
	}

	fflush(stdout);
}

void
warn(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	verr(fmt, ap);
	va_end(ap);
}

int
isDirectory(const char *path)
{
	struct stat fileStat;

	if (stat(path, &fileStat) == -1) {
		warn("stat: \"%s\":", path);
		return -1;
	}

	if (S_ISDIR(fileStat.st_mode)) {
		return 1;
	}

	return 0;
}

int
getRPath(const char *fileName, const char *root, char *rpath)
{
	char path[PATH_MAX];

	if ((snprintf(path, sizeof(path),
			"%s/%s", root, fileName)) < 1) {
		warn("realpath: \"%s\":", path);
		return 0;
	}

	if (!realpath(path, rpath)) {
		warn("realpath: \"%s\":", path);
		return 0;
	}

	return 1;
}

int
main(int argc, char *argv[])
{
    struct dirent **namelist;
	char tmp[PATH_MAX];
    int n;

    n = scandir((argc < 2) ? "." : argv[1], &namelist, NULL, alphasort);
    if (n == -1) {
        perror("scandir");
        exit(EXIT_FAILURE);
    }

    while (n--) {
		getRPath(namelist[n]->d_name, (argc < 2) ? "." : argv[1], tmp);
		if (isDirectory(tmp) == 1) {
			free(namelist[n]);
			continue;
		}

		printf("%s\n", namelist[n]->d_name);
        free(namelist[n]);
    }
    free(namelist);

    exit(EXIT_SUCCESS);
}
