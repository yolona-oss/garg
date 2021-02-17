#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <signal.h>
#include <getopt.h>
#include <ctype.h>

#include "main.h"
#include "run/run.h"
#include "utils/util.h"
#include "utils/eprintf.h"
#include "utils/list.h"
#include "games/gamerec.h"

/* vars */
char buf[4096];

int done = 0;

char g_user_db[PATH_MAX];   /* path to specific sqlite db */
char g_user_path[PATH_MAX]; /* scan path */

int g_scan_depth = 1; /* recursion level in scan */

extern node_t *g_exceptions;
extern node_t *g_inclusions;

/* funcs */
static void cleanup();
static void terminate();
static void usage();

static void
cleanup()
{
	list_freeall(g_exceptions);
	list_freeall(g_inclusions);
	grt_free(gr_tab);
}

static void
terminate()
{
	done = 1;
}

static void
usage()
{
	/* TODO */
	die("--db=<PATH> -[qd] -s <PATH>");
}

int
main(int argc, char **argv)
{
	/* signals hendling */
	struct sigaction action;
	memset(&action, 0, sizeof(struct sigaction));
	action.sa_handler = terminate;
	sigaction(SIGTERM, &action, NULL);
	sigaction(SIGINT, &action, NULL);

	argv0 = *argv;

	g_qflag = 0;
	g_dflag = 0;

	char ch = 0;
	int val = 0;

	struct option long_options[] = {
		{ "db",     required_argument, NULL, 'c' },
		{ "quiet",  no_argument,       NULL, 'q' },
		{ "debug",  no_argument,       NULL, 'd' },
		{ "help",   no_argument,       NULL, 'h' },
		{ "scan",   required_argument, NULL, 's' },
		{ "scan-depth", required_argument, NULL, 'p' },
	};

	while ((ch = getopt_long(argc, argv, "s:c:qdh", long_options, NULL)) != -1) {
		switch (ch) {
			case 'c':
				strcpy(g_user_db, optarg);
				break;
			case 'q':
				g_qflag = 1;
				break;
			case 'd':
				g_dflag = 1;
				break;
			case 's':
				strcpy(g_user_path, optarg);
				break;
			case 'p':
				if ((val=atoi(optarg))) {
					if (val > 5) {
						printf("Scanning can take a long time\n");
					}
					g_scan_depth = val;
				} else {
					warn("Dissatisfy value: %s", optarg);
				}
				break;
			case 'h':
				usage();
				break;
			case '?':
				usage();
			default:
				usage();
				/* TODO */
				break;
		}
	}

	int rc = run();

	cleanup();

	return rc;
}
