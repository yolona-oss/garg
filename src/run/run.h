#ifndef _RUN_H_
#define _RUN_H_

#include <limits.h>
#include "menu.h"

/* vars */
extern int done;
extern char g_user_path[PATH_MAX];

/* funcs */
void *resizeHandler(int);
int run(void);

#endif
