#include <stdio.h>
#include <errno.h>
#include <time.h>

#include "../main.h"
#include "../games/gamerec.h"
#include "../scan/scan.h"
#include "../db/dbman.h"
#include "../utils/eprintf.h"

#define M_SEC(n) 1000*n

static const unsigned int interval = M_SEC(0.1);

extern char g_user_path[PATH_MAX];
extern int done;

static void
difftimespec(struct timespec *res, struct timespec *a, struct timespec *b)
{
	res->tv_sec = a->tv_sec - b->tv_sec - (a->tv_nsec < b->tv_nsec);
	res->tv_nsec = a->tv_nsec - b->tv_nsec +
	               (a->tv_nsec < b->tv_nsec) * 1E9;
}

static int
pre(void)
{
	db_read_settings();
	db_read_cached_recs();

	if (g_user_path[0]) {
		scan(g_user_path);
	}

	return 0;
}

int
run()
{
	if ( pre() ) {
		warn("Cant pre initiate run");
		return -1;
	}
	
	struct timespec start, current, diff, intspec, wait;

	/* for (int i = 0; i < gr_tab.ngames; i++) { */
	/* 	gr_print(&gr_tab.game_rec[i]); */
	/* } */

	while (!done) {
		if (clock_gettime(CLOCK_MONOTONIC, &start) < 0) {
			die("clock_gettime:");
		}

		/* scan_inclusions(NULL); /1* TODO *1/ */
		check_gr_tab(gr_tab);

		if (!done) {
			if (clock_gettime(CLOCK_MONOTONIC, &current) < 0) {
				die("clock_gettime:");
			}
			difftimespec(&diff, &current, &start);

			intspec.tv_sec = interval / 1000;
			intspec.tv_nsec = (interval % 1000) * 1E6;
			difftimespec(&wait, &intspec, &diff);

			if (wait.tv_sec >= 0) {
				if (nanosleep(&wait, NULL) < 0 &&
				    errno != EINTR) {
					die("nanosleep:");
				}
			}
		}
	}

	return 0;
}
