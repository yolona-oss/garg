#include <stdio.h>

#include "garg_event.h"
#include "../utils/eprintf.h"

/* vars */
static event_t event[N_EVENTS];

/* funcs */
static void close_event(enum EVENT_ID eid);
static int is_event_up(event_t e);

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

void
event_action(enum EVENT_ID eid, int (*func)(void))
{
	event[eid].func = func;
	event[eid].type = eid;
	event[eid].up = 0;
}

void
up_event(enum EVENT_ID eid)
{
	event[eid].up = 1;
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
	event[eid].up = 0;
}

static int
is_event_up(event_t e)
{
	if (e.up ==1) {
		return 1;
	}
	return 0;
}

//check all event from first
//if event is triggered load data to event storage and return
//if no events return -1
int
poll_event(event_t *e)
{
	for (int i = 0; i < N_EVENTS; i++) {
		if (!event[i].func) {
			continue;
		}
		event[i].func();
		if (is_event_up(event[i]) == 1) {
			e->type = event[i].type;
			esnprintf(e->buf, sizeof(e->buf),
					"%s", (event[i].buf[0] == '\0') ? NULL : estrdup(event[i].buf));
			e->key = event[i].key;
			e->func = event[i].func;
			close_event(i);
			return 0;
		}
	}
	e->type = -1;

	return -1;
}
