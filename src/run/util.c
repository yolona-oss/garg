#include <gdk/gdk.h>
#include <libgen.h>
#include <time.h>

#include "run.h"
#include "../utils/eprintf.h"
#include "../games/gamerec.h"
#include "../scan/scan.h"
#include "../db/dbman.h"
#include "../utils/util.h"

struct run_game_info {
	game_t *gr;
	pid_t pid;
	time_t start_time;
	time_t end_time;
	unsigned int done: 1;
};

/* vars */
static struct run_game_info rgi;

/* funcs */
static void init_game_tab(void);
static const char *game_icon_default = "assets/game-icon.png";
static void shutup_game(int sig);
GtkTreeIter gtk_tree_model_search_by_name_col(GtkTreeModel *liststore, char *find);

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

static void
shutup_game(int sig)
{
	rgi.end_time = time(NULL);
	time_t diff = difftime(rgi.end_time, rgi.start_time);
	gr_play_time_append(rgi.gr, diff);

	gr_save(rgi.gr);

	GtkTreeIter iter = gtk_tree_model_search_by_name_col(gapp.game_list_store, rgi.gr->name);
	tree_store_row_change_val(gapp.game_list_store, iter, rgi.gr);

	rgi.gr = NULL;
	rgi.pid = -1;
	rgi.done = 1;
}

void
init_rgi(void)
{
	rgi.done = 1;
}

GtkTreeIter
gtk_tree_model_search_by_name_col(GtkTreeModel *liststore, char *find)
{
	GtkTreeIter iter;
	gboolean valid;
	gchar *name;

	valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(liststore), &iter);

	while (valid)
	{
		gtk_tree_model_get(liststore, &iter, NAME_C, &name,  -1);
		if (strcmp(name, find) == 0) {
			g_free(name);
			return iter;
		}

		g_free(name);

		valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(liststore), &iter);
	}

	return iter;
}

/* run some game with id 
 * returns:
 * 		1  - game is running
 * 		0  - child stoped
 * 		-1 - some error or another game already running */
int
run_game(int id)
{
	if (!rgi.done) {
		return -1;
	}

	rgi.done = 0;
	rgi.gr = grt_find(id);
	char *path = rgi.gr->start_point;
	char run[strlen(path) + 3];
	esnprintf(run, sizeof(run), "/.%s", path);

	/* last time setup */
	rgi.gr->last_time = time(NULL);
	/* start time */
	rgi.start_time = time(NULL);

	rgi.pid = fork();
	switch (rgi.pid) {
		case 0: //child
		{
			/* int devnull = open(, O_CREAT|O_WRONLY|O_TRUNC, /1* open or create and open with 600 mask *1/ */
			/* S_IRWXU); */
			/* if (!devnull) { */
			/* } */
			/* dup2(devnull, 1); */
			/* dup2(devnull, 2); */

			int argc = rgi.gr->start_argv ? 2 : 3;

			char *argv[argc];
			argv[0] = basename(run); //TODO this non freeding
			if (argc == 2) {
				argv[1] = NULL;
			} else if (argc == 3) {
				argv[1] = rgi.gr->start_argv;
				argv[2] = NULL;
			}

			if (execv(run, argv) < 0) {
				exit(1);
			}

			return 1;
		}

		default: //parent
		{
			signal(SIGCHLD, shutup_game);
		}
		break;

		case -1:
			//log error TODO
			warn("fork:");
			return -1;
	}

	return 0;
}

int
tree_store_row_change_val(GtkTreeModel *model, GtkTreeIter iter, game_t *gr)
{
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, //TODO
					   /* ICON_C, gr->icon, */
					   NAME_C, gr->name,
					   LAST_TIME_C, gr->last_time,
					   PLAY_TIME_C, play_time_human(gr->play_time),
					   GENER_C, gr->gener ? gr->gener : "",
					   -1);
	return 0;
}

int
get_game_id_from_tree_model(GtkTreeModel *model, GtkTreeIter iter)
{
	guint id;
	gtk_tree_model_get(model, &iter, ID_C, &id, -1);

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
									 LAST_TIME_C, gr->last_time,
									 PLAY_TIME_C, play_time_human(gr->play_time),
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
