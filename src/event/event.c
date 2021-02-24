#include <stdio.h>

#include "garg_event.h"

static event_t event_list[N_EVENTS];

void
setup_event(enum EVENT e, int (*func)(void))
{
	event_list[e].func = func;
}

void
do_event_func(enum EVENT e)
{
	event_list[e].func();
}

void
up_event(int en)
{
	event_list[en].flag = 1;
}

void
close_event(int en)
{
	event_list[en].flag = 0;
}

int
pull_event()
{
	int i;
	check_timer();
	for (i = 0; i < N_EVENTS; i++) {
		event_list[i].func();

		if (event_list[i].flag == 1) {
			close_event(i);
			return i;
		}
	}

	return -1;
}
