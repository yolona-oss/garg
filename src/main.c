#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <signal.h>
#include <getopt.h>
#include <ctype.h>

#include <fcntl.h>
#include <unistd.h>

#include "main.h"
#include "run/run.h"
#include "utils/util.h"
#include "utils/eprintf.h"
#include "utils/list.h"
#include "games/gamerec.h"

/* vars */
int done = 0;
char g_buf[4096];

char g_user_db[PATH_MAX];   /* path to specific sqlite db */
char g_user_path[PATH_MAX]; /* scan path */

int g_scan_depth = 3; /* recursion level in scan */

extern node_t *g_exceptions;
extern node_t *g_inclusions;

static int g_logf = 0;

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
	close(g_logf);
}

static void
terminate()
{
	cleanup();
	done = 1;
}

static void
usage()
{
	/* TODO */
	die("--db=<PATH> -[qd] -s <PATH> --scan-depth=<depth>");
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

	char ch;

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
			case 'c': /* using db path */
				strcpy(g_user_db, optarg);
				break;
			case 'q': /* quiet mode */
				g_qflag = 1;
				break;
			case 'd': /* debug mode */
				g_dflag = 1;
				break;
			case 's': /* scan path */
				strcpy(g_user_path, optarg);
				break;
			case 'p': /* setting recursion level in scan */
				;
				int val;
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

			default:
				usage();
				break;
		}
	}

	/* redirect errors msgs to log file */
	char log_path[PATH_MAX];
	snprintf(log_path, sizeof(log_path),
			"%s/.garg.log", getenv("HOME"));
	g_logf = open(log_path, O_CREAT|O_APPEND|O_WRONLY, /* open or create and open with 600 mask */
			0664);
	if (g_logf) {
		if(dup2(g_logf, 2) < 0) {
			warn("Cant open log file: %s. dup2:", log_path);
		}
	} else {
		warn("Cant open log file: %s. open:", log_path);
	}

	GtkApplication *app;
	int status;

	app = gtk_application_new("org.gtk.garg", G_APPLICATION_FLAGS_NONE);
	g_signal_connect(app, "activate", G_CALLBACK(run), NULL);
	status = g_application_run(G_APPLICATION(app), 0, NULL);
	g_object_unref(app);

	cleanup();

	return status;
}
