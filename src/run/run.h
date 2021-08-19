#ifndef _RUN_H_
#define _RUN_H_

#include <limits.h>
#include <gtk/gtk.h>

/* vars */
extern char g_user_path[PATH_MAX];
extern GtkWidget *window;
extern GtkWidget *game_list_wrapper;

/* funcs */
void quit(GtkWidget *window, gpointer data);

//dialogs
void quick_message(GtkWindow *parent, gchar *message);
void add_new_game_dialog(GtkButton *button, gpointer window);
void info_msg(const char *text);

//utils
void gtk_widget_set_margin_around(GtkWidget *widget, gint space);
GdkPixbuf *load_icon(const char *icon_path, const char *fallback, int w, int h);

int run(GtkApplication* app, gpointer user_data);

#endif
