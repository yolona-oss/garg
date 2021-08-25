#include <gdk/gdk.h>

#include "run.h"
#include "../utils/eprintf.h"
#include "../games/gamerec.h"
#include "../scan/scan.h"
#include "../db/dbman.h"
#include "../utils/util.h"

static void init_game_tab(void);
static const char *game_icon_default = "assets/game-icon.png";

static void
init_game_tab(void)
{
	db_read_settings();
	db_read_cached_recs();
	/* scan_inclusions(); TODO */

	if (g_user_path[0]) {
		scan(g_user_path);
	}
}

int
tree_store_row_change_val(GtkTreeModel *model, GtkTreeIter iter, game_t *gr)
{
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, //TODO
					   /* ICON_C, gr->icon, */
					   NAME_C, gr->name,
					   LAST_TIME_C, gr_last_time_human(gr),
					   PLAY_TIME_C, gr_play_time_human(gr),
					   GENER_C, gr->gener ? gr->gener : "",
					   -1);
	return 0;
}

int
get_game_id_from_tree_model(GtkTreeModel *model, GtkTreeIter iter)
{
	guint id;
	gtk_tree_model_get(model, &iter, ID_C, &id,  -1);

	return id;
}

void
add_new_game(GtkListStore *store, game_t *gr)
{
	GtkTreeIter iter;

	GdkPixbuf *pixbuf = load_icon(gr->icon, game_icon_default, icon_size_w, icon_size_h);
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, ICON_C, pixbuf,
									 NAME_C, gr->name,
									 GENER_C, gr->gener ? gr->gener : "",
									 LAST_TIME_C, gr_last_time_human(gr),
									 PLAY_TIME_C, gr_play_time_human(gr),
									 ID_C, gr->id,
									 -1);
	g_object_unref(pixbuf);
}

void
add_new_game_short(game_t *gr)
{
	GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(gapp.game_list));
	add_new_game(store, gr);
}

int
setup_game_entries(GtkTreeView *list)
{
	GtkListStore *store;

	init_game_tab();
	store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(list)));
	for (int i = 0; i < gr_tab.ngames; i++)
	{
		add_new_game(store, &gr_tab.game_rec[i]);
	}

	return 0;
}

void
gtk_entry_buffer_insert_text_after_delete(GtkEntryBuffer *buffer, int pos, const char *text, int len)
{
	gtk_entry_buffer_delete_text(buffer, 0, gtk_entry_buffer_get_length(buffer));
	gtk_entry_buffer_insert_text(buffer, pos, text, len);
}

void
gtk_widget_set_margin_around(GtkWidget *widget, gint space)
{
	gtk_widget_set_margin_top(widget, space);
	gtk_widget_set_margin_bottom(widget, space);
	gtk_widget_set_margin_start(widget, space);
	gtk_widget_set_margin_end(widget, space);
}

GdkPixbuf *
load_icon(const char *icon_path, const char *fallback, int w, int h)
{
	GError *error = NULL;
	GdkPixbuf *pixbuf = NULL;
	char path[PATH_MAX];
	int ffallback = 0;

	if (icon_path)
		realpath(icon_path, path);
	else if (fallback) {
		realpath(fallback, path);
		ffallback = 1;
	} else {
		return NULL;
	}

	pixbuf = gdk_pixbuf_new_from_file_at_size(path, w, h, &error);

	if (!pixbuf && ffallback == 1) {
		g_assert(error != NULL);
		warn(G_STRLOC ": unable to open '%s': %s",
				path, error->message);
		g_error_free(error);
	} else if (fallback == 0 && !pixbuf) {
		pixbuf = load_icon(NULL, fallback, w, h);
	}

	return pixbuf;
}
