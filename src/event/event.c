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
	event[eid].type = 1;
}

void
add_event_buf(enum EVENT_ID eid, const char *s)
{
	esnprintf(event[eid].buf, sizeof(event[eid].buf), "%s", s);
}

void
add_event_key(enum EVENT_ID eid, const char key)
{
	event[eid].key = key;
}

static void
close_event(enum EVENT_ID eid)
{
	event[eid].type = 0;
}

int
poll_event(event_t *e)
{
	for (int i = 0; i < N_EVENTS; i++) {
		event[i].func();
		if (event[i].type == 1) {
			e->type = event[i].type;
			esnprintf(e->buf, sizeof(e->buf), "%s", (event[i].buf[0] == '\0') ? NULL : estrdup(event[i].buf));
			e->key = event[i].key;
			e->func = event[i].func;
			close_event(i);
			return 0;
		}
	}
	e->type = -1;

	return -1;
}
