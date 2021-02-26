#include <stdio.h>

#include "garg_event.h"
#include "../utils/eprintf.h"

/* vars */
static event_t event[N_EVENTS];

/* funcs */
static void close_event(enum EVENT_ID eid);

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

void
event_action(enum EVENT_ID eid, int (*func)(void))
{
	event[eid].func = func;
}

void
up_event(enum EVENT_ID eid)
{
	event[eid].flag = 1;
}

void
add_event_buf(enum EVENT_ID eid, const char *s)
{
	esnprintf(event[eid].buf, sizeof(event[eid].buf), "%s", s);
}

static void
close_event(enum EVENT_ID eid)
{
	event[eid].flag = 0;
}

int
pull_event(void)
{
	int i;
	for (i = 0; i < N_EVENTS; i++) {
		event[i].func();
		if (event[i].flag == 1) {
			close_event(i);
			return i;
		}
	}

	return -1;
}
