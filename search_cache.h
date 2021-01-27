#ifndef _SEARCH_CACHE_H_
#define _SEARCH_CACHE_H_

#include "list.h"

extern node_t *g_exceptions;
extern node_t *g_exceptions_head;
extern node_t *g_inclusions;
extern node_t *g_inclusions_head;

int db_read_settings(void);

#endif
