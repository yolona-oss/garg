#ifndef _RUN_H_
#define _RUN_H_

#include <limits.h>
#include <gtk/gtk.h>

/* vars */
extern char g_user_path[PATH_MAX];

/* funcs */
void quit(GtkWidget *window, gpointer data);
int run(void);

#endif
