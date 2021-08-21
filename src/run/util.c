#include <gdk/gdk.h>

#include "run.h"
#include "../utils/eprintf.h"
#include "../games/gamerec.h"
#include "../scan/scan.h"
#include "../db/dbman.h"

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

void
add_new_game(GtkListStore *store, game_t *gr)
{
	GtkTreeIter iter;
	/* setuping last play time variable */
	char *last_time = gr->last_time ? ctime(&gr->last_time) : "";
	if (last_time[0] != '\0') last_time[strlen(last_time)-1] = '\0'; //ctime use \n simbol in end, deleting it

	GdkPixbuf *pixbuf = load_icon(gr->icon, game_icon_default, icon_size_w, icon_size_h);

	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, ICON_C, pixbuf,
									 NAME_C, gr->name,
									 GENER_C, gr->gener ? gr->gener : "",
									 LAST_TIME_C, last_time,
									 PLAY_TIME_C, play_time_human(gr),
									 ID_C, gr->id,
									 -1);
	g_object_unref(pixbuf);
}

void
add_new_game_short(game_t *gr)
{
	GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(game_list)));
	add_new_game(store, gr);
}

int
setup_game_entries(GtkWidget *list)
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
