#include "run.h"
#include "../db/dbman.h"
#include "../utils/util.h"

void
show_sel_game_info(GtkWidget *section, GtkWidget *info_box)
{
	GtkTreeIter iter;
	GtkTreeModel *model;
	gchar *value;

	if (gtk_tree_selection_get_selected(GTK_TREE_SELECTION(section), &model, &iter)) {
		gtk_widget_show(info_box);
		gtk_tree_model_get(model, &iter, NAME_C, &value,  -1);
		gtk_label_set_text(gapp.game_info_bar.game_name,
						   value);
		g_free(value);
	}
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
		id = get_game_id_from_tree_model(model, iter);
		run_game(id);
		game_t *gr = grt_find(id);
		gr_save(gr);
		tree_store_row_change_val(model, iter, gr);
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
		id = get_game_id_from_tree_model(model, iter);
		run_game(id);
		game_t *gr = grt_find(id); 	//TODO
		gr_save(gr); 				//TODO
		tree_store_row_change_val(model, iter, gr); //TODO
	}
}
