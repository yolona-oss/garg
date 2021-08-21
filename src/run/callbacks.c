#include "run.h"
#include "../db/dbman.h"
#include "../utils/util.h"

void
show_sel_game_info(GtkWidget *widget, GtkWidget *info_box)
{
	GtkTreeIter iter;
	GtkTreeModel *model;
	gchar *value;

	if (gtk_tree_selection_get_selected(GTK_TREE_SELECTION(widget), &model, &iter)) {
		gtk_widget_show(info_box);
		gtk_tree_model_get(model, &iter, NAME_C, &value,  -1);
		gtk_label_set_text(GTK_LABEL(info_box_game_name_label),
						   value);
		g_free(value);
	}
}

static int
get_game_id(GtkTreeModel *model, GtkTreeIter iter)
{
	guint id;
	gtk_tree_model_get(model, &iter, ID_C, &id,  -1);

	return id;
}

static void
post_run_game(GtkTreeModel *model, GtkTreeIter iter, int id)
{
	game_t *gr = grt_find(id);
	/* setuping last play time variable */
	char *last_time = gr->last_time ? ctime(&gr->last_time) : "";
	if (last_time[0] != '\0') str_del_last_sym(last_time);

	gtk_list_store_set(GTK_LIST_STORE(model), &iter, LAST_TIME_C, last_time, -1);
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, PLAY_TIME_C, play_time_human(gr), -1);

	sqlite3 *db = db_init();
	db_upd_rec(db, gr);
	db_close(db);
}

void
game_entry_clicked(GtkTreeView *treeview,
					GtkTreePath *path,
					GtkTreeViewColumn  *col,
					gpointer userdata)
{
	GtkTreeModel *model;
	GtkTreeIter   iter;
	guint id;

	model = gtk_tree_view_get_model(treeview);
	if (gtk_tree_model_get_iter(model, &iter, path))
	{
		gchar *name;
		gtk_tree_model_get(model, &iter, NAME_C, &name, -1);
		g_print("Starting %s\n", name);
		g_free(name);

		id = get_game_id(model, iter);
		run_game(id);
		post_run_game(model, iter, id);
	}
}

void
play_button_clicked(GtkWidget *widget, gpointer *list)
{
	GtkTreeModel *model;
	guint id;
	GtkTreeIter iter;
	GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(list));

	if (gtk_tree_selection_get_selected(GTK_TREE_SELECTION(selection), &model, &iter)) {
		id = get_game_id(model, iter);
		run_game(id);
		post_run_game(model, iter, id);
	}
}
