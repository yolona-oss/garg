#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

#include "run.h"
#include "../main.h"
#include "../games/gamerec.h"
#include "../scan/scan.h"
#include "../db/dbman.h"
#include "../utils/eprintf.h"
#include "../utils/gtk_widget_list.h"

enum {
	ICON_C = 0,
	NAME_C,
	ID_C,
	N_COLUMNS
};

/* funcs */
static void init_game_tab(void);
/* static gboolean on_key_press(GtkWidget *widget, GdkEventKey *event); */

/* static GtkWidget *setup_menu_bar(); */
static int add_game_entry(GtkWidget *w, game_t gr);
static int setup_game_entries(GtkWidget *w);

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
quit(GtkWidget *window, gpointer data)
{
	gtk_main_quit();
}

/* static gboolean */
/* on_key_press(GtkWidget *widget, GdkEventKey *event) */
/* { */
/* 	g_printerr("%s\n", gdk_keyval_name (event->keyval)); */
/* 	g_printerr("%d\n", event->keyval); */

/* 	if (event->type == GDK_KEY_PRESS) */
/* 	{ */  
/* 		switch (event->keyval) */  
/* 		{ */  
/* 			case GDK_KEY_q: */
/* 				g_signal_emit_by_name(widget, "destroy"); */
/* 				break; */
/* 		} */  
/* 	} */

/* 	return TRUE; */
/* } */

/* static GtkWidget * */
/* setup_menu_bar(void) */
/* { */
/* 	GtkWidget *menubar = gtk_menu_bar_new(); */
/* 	GtkWidget *fileMenu = gtk_menu_new(); */
/* 	GtkWidget *fileMi = gtk_menu_item_new_with_label("File"); */
/* 	GtkWidget *quitMi = gtk_menu_item_new_with_label("Quit"); */
/* 	gtk_menu_item_set_submenu(GTK_MENU_ITEM(fileMi), fileMenu); */
/* 	gtk_menu_shell_append(GTK_MENU_SHELL(fileMenu), quitMi); */
/* 	gtk_menu_shell_append(GTK_MENU_SHELL(menubar), fileMi); */
/* 	g_signal_connect(G_OBJECT(quitMi), "activate", */
/* 					 G_CALLBACK(gtk_main_quit), NULL); */

/* 	return menubar; */
/* } */

static int
add_game_entry(GtkWidget *w, game_t gr)
{
	GtkListStore *store;
	GtkTreeIter iter;

	store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(w)));

	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, ICON_C, "ICON", -1);
	gtk_list_store_set(store, &iter, NAME_C, gr.name, -1);
	gtk_list_store_set(store, &iter, ID_C, gr.id, -1);

	return 0;
}

static int
setup_game_entries(GtkWidget *w)
{
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *name_col, *id_col, *icon_col;
	GtkListStore *store;

	renderer = gtk_cell_renderer_text_new();

	icon_col = gtk_tree_view_column_new_with_attributes("",
			renderer, "text", ICON_C, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(w), icon_col);

	name_col = gtk_tree_view_column_new_with_attributes("Name",
			renderer, "text", NAME_C, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(w), name_col);

	id_col = gtk_tree_view_column_new_with_attributes("ID",
			renderer, "number", ID_C, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(w), id_col);
	gtk_tree_view_column_set_visible(id_col, FALSE);

	store = gtk_list_store_new(N_COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_UINT);

	gtk_tree_view_set_model(GTK_TREE_VIEW(w),
			GTK_TREE_MODEL(store));

	g_object_unref(store);

	init_game_tab();
	for (int i = 0; i < gr_tab.ngames; i++)
	{
		add_game_entry(w, gr_tab.game_rec[i]);
	}

	return 0;
}

void
show_sel_game_info(GtkWidget *widget, gpointer label) {
	GtkTreeIter iter;
	GtkTreeModel *model;
	gchar *value;

	if (gtk_tree_selection_get_selected(
				GTK_TREE_SELECTION(widget), &model, &iter)) {
		gtk_tree_model_get(model, &iter, NAME_C, &value,  -1);
		gtk_label_set_text(GTK_LABEL(label), value);
		g_free(value);
	}
}

void
view_onRowActivated(GtkTreeView *treeview,
					GtkTreePath *path,
					GtkTreeViewColumn  *col,
					gpointer userdata)
{
	GtkTreeModel *model;
	GtkTreeIter   iter;

	model = gtk_tree_view_get_model(treeview);
	if (gtk_tree_model_get_iter(model, &iter, path))
	{
		guint id;
		gtk_tree_model_get(model, &iter, ID_C, &id, -1);
		g_print ("Double-clicked row contains id %d\n", id);
		run_game(id);
	}
}

int
run()
{
	gtk_init(0, NULL);
	GtkWidget *window,
			  *main_box,
			  *header,
			  *wrapper,
			  *game_list_wrapper,
			  *info_box,
			  *info_box_label,
			  *games_window,
			  *game_list;

	/* Setuping window props */
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), "GArg");
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_NONE);
	gtk_container_set_border_width (GTK_CONTAINER(window), 1);
	gtk_window_set_default_size(GTK_WINDOW(window), 800, 400);

	main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_container_add(GTK_CONTAINER(window), main_box);

	header = gtk_header_bar_new();
	gtk_header_bar_set_custom_title(GTK_HEADER_BAR(header), gtk_label_new("GArg"));
	gtk_box_pack_start(GTK_BOX(main_box), header, FALSE, TRUE, 0);

	wrapper = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_box_pack_start(GTK_BOX(main_box), wrapper, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(wrapper), gtk_label_new("DOCK"), FALSE, TRUE, 8);

	game_list_wrapper = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_box_pack_start(GTK_BOX(wrapper), game_list_wrapper, TRUE, TRUE, 0);

	/* Setuping info box */
	info_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	info_box_label = gtk_label_new("info box");
	gtk_box_pack_start(GTK_BOX(info_box), info_box_label, FALSE, TRUE, 0);
	gtk_box_pack_end(GTK_BOX(game_list_wrapper), info_box, FALSE, TRUE, 10);

	/* Setuping scrolled window for games list */
	games_window = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(games_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_box_pack_start(GTK_BOX(game_list_wrapper), games_window, TRUE, TRUE, 0);

	/* Initializing game list */
	game_list = gtk_tree_view_new();
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(game_list), TRUE);
	gtk_container_add(GTK_CONTAINER(games_window), game_list);

	/* Showing game entries */
	setup_game_entries(game_list);

	g_signal_connect(G_OBJECT(window), "destroy",
					 G_CALLBACK(quit), NULL);
	/* g_signal_connect(G_OBJECT(window), "key-press-event", */
	/* 				 G_CALLBACK(on_key_press), NULL); */

	GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(game_list));
	g_signal_connect(selection, "changed", G_CALLBACK(show_sel_game_info), info_box_label);
	g_signal_connect(game_list, "row-activated", (GCallback) view_onRowActivated, NULL);

	gtk_widget_show_all(window);
	gtk_main();

	return 0;
}
