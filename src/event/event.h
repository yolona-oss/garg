#ifndef _TUI_EVENT_H_
#define _TUI_EVENT_H_

enum events {
	N_EVENTS,
};

/* vars */
extern char event_list[N_EVENTS];

/* funcs */
void set_event(int event);
void clean_event(int event);
int pull_event();

#endif
