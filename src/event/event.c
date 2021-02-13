#include <stdio.h>

#include "event.h"

char event_list[N_EVENTS];

void
set_event(int en)
{
	event_list[en] = 1;
}

void
clean_event(int en)
{
	event_list[en] = 0;
}

int
pull_event()
{
	for (int i = 0; i < N_EVENTS; i++) {
		if (event_list[i] == 1) {
			clean_event(i);
			return i;
		}
	}

	return -1;
}
