#include <time.h>
#include <unistd.h>
#include <errno.h>

#include "garg_time.h"
#include "eprintf.h"

struct timespec
now()
{
	struct timespec now;
	if (clock_gettime(CLOCK_MONOTONIC, &now) < 0) {
		die("clock_gettime: ");
	}
	return now;
}

void
difftimespec(struct timespec *res, struct timespec *a, struct timespec *b)
{
	res->tv_sec = a->tv_sec - b->tv_sec - (a->tv_nsec < b->tv_nsec);
	res->tv_nsec = a->tv_nsec - b->tv_nsec +
	               (a->tv_nsec < b->tv_nsec) * 1E9;
}

struct timespec
time_remain(struct timespec *start, int interval)
{
	struct timespec current, diff, wait, intspec;

	current = now();
	difftimespec(&diff, &current, start);

	intspec.tv_sec = interval / 1000;
	intspec.tv_nsec = (interval % 1000) * 1E6;
	difftimespec(&wait, &intspec, &diff);

	return wait;
}

void
wait_left(struct timespec wait)
{
	if (wait.tv_sec >= 0) {
		if (nanosleep(&wait, NULL) < 0 &&
			errno != EINTR) {
			die("nanosleep:");
		}
	}
}
