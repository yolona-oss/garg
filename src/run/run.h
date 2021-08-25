#ifndef _RUN_H_
#define _RUN_H_

#include <limits.h>
#include <gtk/gtk.h>
#include "../games/gamerec.h"

struct game_info_bar_t {
	GtkBox    *box;
	GtkLabel  *game_name;
	GtkBox    *tool_box;
	GtkButton *play;
};

struct pages {
	GtkLabel   *label;
	GtkListBox *list;
	GtkWidget **items;
};

struct dock_t {
	GtkBox *box;
	/* struct pages *library; */
	GtkLabel *library_label;
	GtkListBox *library_list;
};

struct header_t {
	GtkHeaderBar *bar;

	GtkButton *add_new_game_button;
	GtkButton *app_options_menu;
	GtkButton *list_type_button;

	GtkButton      *search_button;
	GtkSearchBar   *search_bar;
	GtkSearchEntry *search_entry;
};

struct gapp_t {
	GtkWindow *window;

	GtkBox *main_box;
	GtkBox *wrapper;
	GtkBox *game_list_wrapper;

	GtkScrolledWindow *games_window;
	GtkTreeView 	  *game_list;
	GtkTreeModel      *game_list_store;

	struct header_t			header;
	struct dock_t			dock;
	struct game_info_bar_t  game_info_bar;
};

/* vars */
extern char g_user_path[PATH_MAX];
extern struct gapp_t gapp;
extern const int icon_size_w;
extern const int icon_size_h;
extern const int g_dock_min_size_h;
extern const int g_dock_min_size_w;

enum {
	ICON_C = 0,
	NAME_C,
	GENER_C,
	LAST_TIME_C,
	PLAY_TIME_C,
	ID_C,
	N_COLUMNS
};


/* funcs */
void quit(GtkWidget *window, gpointer data);
void add_new_game(GtkListStore *store, game_t *gr);
void add_new_game_short(game_t *gr);

int run(GtkApplication* app, gpointer user_data);

//callbacks
void show_sel_game_info(GtkWidget *widget, GtkWidget *info_box);
void game_entry_clicked(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn  *col, gpointer data);
void play_button_clicked(GtkWidget *widget, gpointer *list);


//dialogs
void quick_message(GtkWindow *parent, gchar *message);
void add_new_game_dialog(GtkButton *button, gpointer window);
void info_msg(const char *text);

//utils
void gtk_entry_buffer_insert_text_after_delete(GtkEntryBuffer *buffer, int pos, const char *text, int len);
void gtk_widget_set_margin_around(GtkWidget *widget, gint space);
GdkPixbuf *load_icon(const char *icon_path, const char *fallback, int w, int h);

int get_game_id_from_tree_model(GtkTreeModel *model, GtkTreeIter iter);
int tree_store_row_change_val(GtkTreeModel *model, GtkTreeIter iter, game_t *gr);

int setup_game_entries(GtkTreeView *list);

#endif
