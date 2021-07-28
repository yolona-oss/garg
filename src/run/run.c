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

static const int icon_size_w = 200;
static const int icon_size_h = 80;
static const char *game_icon_default = "assets/game-icon.png";

enum {
	ICON_C = 0,
	NAME_C,
	ID_C,
	N_COLUMNS
};

/* funcs */
/* static GtkWidget *setup_menu_bar(); */
static void init_game_tab(void);
static GtkWidget *find_child(GtkWidget* parent, const gchar* name);
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

static GtkWidget *
find_child(GtkWidget* parent, const gchar* name)
{
	if (g_ascii_strcasecmp(gtk_widget_get_name(GTK_WIDGET(parent)), (gchar*)name) == 0) { 
		return parent;
	}
	if (GTK_IS_BIN(parent)) {
		GtkWidget *child = gtk_bin_get_child(GTK_BIN(parent));
		return find_child(child, name);
	}

	if (GTK_IS_CONTAINER(parent)) {
		GList *children = gtk_container_get_children(GTK_CONTAINER(parent));
		while (children != NULL) {
			GtkWidget* widget = find_child(children->data, name);
			children = g_list_next(children);
			if (widget != NULL) {
				return widget;
			}
		}
	}
	return NULL;
}

static GdkPixbuf *
load_icon(game_t gr)
{
	GError *error = NULL;
	GdkPixbuf *pixbuf = NULL;
	char path[PATH_MAX];

	realpath(gr.icon ? gr.icon : game_icon_default, path);
	pixbuf = gdk_pixbuf_new_from_file_at_size(path, icon_size_w, icon_size_h, &error);

	if (!pixbuf) {
		g_assert(error != NULL);
		warn(G_STRLOC ": unable to open '%s': %s",
				path, error->message);
		g_error_free (error);
		return NULL;
	}

	return pixbuf;
}

static int
add_game_entry(GtkWidget *treeview, game_t gr)
{
	GtkListStore *store;
	GtkTreeIter iter;

	store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(treeview)));

	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, ICON_C, load_icon(gr), -1);
	gtk_list_store_set(store, &iter, NAME_C, gr.name, -1);
	gtk_list_store_set(store, &iter, ID_C, gr.id, -1);

	return 0;
}

static GtkListStore *
make_model(GtkWidget *list)
{
	GtkCellRenderer *renderer_name, *renderer_id, *renderer_icon;
	GtkTreeViewColumn *name_col, *id_col, *icon_col;
	GtkListStore *store;

	renderer_icon = gtk_cell_renderer_pixbuf_new();
	renderer_name = gtk_cell_renderer_text_new();
	renderer_id = gtk_cell_renderer_text_new();

	icon_col = gtk_tree_view_column_new_with_attributes("",
			renderer_icon, "pixbuf", ICON_C, NULL);
	gtk_tree_view_column_set_sizing(GTK_TREE_VIEW_COLUMN (icon_col),
										GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_fixed_width(GTK_TREE_VIEW_COLUMN (icon_col), icon_size_w);
	gtk_tree_view_append_column(GTK_TREE_VIEW(list), icon_col);

	name_col = gtk_tree_view_column_new_with_attributes("Name",
			renderer_name, "text", NAME_C, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(list), name_col);

	id_col = gtk_tree_view_column_new_with_attributes("ID",
			renderer_id, "text", ID_C, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(list), id_col);
	gtk_tree_view_column_set_visible(id_col, FALSE);

	store = gtk_list_store_new(N_COLUMNS, GDK_TYPE_PIXBUF, G_TYPE_STRING, G_TYPE_UINT);

	return store;
}

static int
setup_game_entries(GtkWidget *list)
{
	GtkListStore *store = make_model(list);

	gtk_tree_view_set_model(GTK_TREE_VIEW(list),
			GTK_TREE_MODEL(store));

	g_object_unref(store);

	init_game_tab();
	for (int i = 0; i < gr_tab.ngames; i++)
	{
		add_game_entry(list, gr_tab.game_rec[i]);
	}

	return 0;
}

static void
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
		gchar *name;
		gtk_tree_model_get(model, &iter, NAME_C, &name, -1);
		g_print ("Starting %s\n", name);
		g_free(name);

		guint id;
		gtk_tree_model_get(model, &iter, ID_C, &id, -1);
		run_game(id);
	}
}

static void
show_sel_game_info(GtkWidget *widget, GtkWidget *info_box)
{
	GtkTreeIter iter;
	GtkTreeModel *model;
	gchar *value;

	if (gtk_tree_selection_get_selected(GTK_TREE_SELECTION(widget), &model, &iter)) {
		gtk_widget_show_all(info_box);
		gtk_tree_model_get(model, &iter, NAME_C, &value,  -1);
		gtk_label_set_text(GTK_LABEL(find_child(info_box, "info_box_gamen")),
						   value);
		g_free(value);
	}
}

static void
play_button(GtkWidget *widget, gpointer *list)
{
	GtkTreeIter iter;
	GtkTreeModel *model;
	guint id;
	GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(list));

	if (gtk_tree_selection_get_selected(GTK_TREE_SELECTION(selection), &model, &iter)) {
		gtk_tree_model_get(model, &iter, ID_C, &id,  -1);
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
			  *main_search_entry,
			  *wrapper,
			  *dock,
			  *game_list_wrapper,
			  *info_box,
			  *games_window,
			  *game_list;

	/* GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file("/home/xewii/projects/garg/image.jpg", NULL); */
	/* GtkWidget *imgw = gtk_image_new_from_pixbuf(pixbuf); */

	/* Setuping window props */
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), "GArg");
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_NONE);
	gtk_container_set_border_width(GTK_CONTAINER(window), 1);
	gtk_window_set_default_size(GTK_WINDOW(window), 800, 400);

	main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_container_add(GTK_CONTAINER(window), main_box);

	/* Setuping header */
	header = gtk_header_bar_new();
	main_search_entry =  gtk_search_entry_new();
	gtk_header_bar_set_custom_title(GTK_HEADER_BAR(header), main_search_entry);
	GtkWidget *add_new_game_button = gtk_button_new_from_icon_name("add", GTK_ICON_SIZE_BUTTON);
	gtk_header_bar_pack_start(GTK_HEADER_BAR(header), add_new_game_button);
	gtk_box_pack_start(GTK_BOX(main_box), header, FALSE, TRUE, 0);

	/* Setuping main part */
	wrapper = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_box_pack_start(GTK_BOX(main_box), wrapper, TRUE, TRUE, 0);

	dock = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	GtkWidget *dock_lbl = gtk_label_new("Library");
	gtk_box_pack_start(GTK_BOX(dock), dock_lbl, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(wrapper), dock, FALSE, TRUE, 8);
	gtk_box_pack_start(GTK_BOX(wrapper), gtk_separator_new(GTK_ORIENTATION_VERTICAL), FALSE, TRUE, 1);
	gtk_container_set_border_width(GTK_CONTAINER(dock), 10);

	game_list_wrapper = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_box_pack_start(GTK_BOX(wrapper), game_list_wrapper, TRUE, TRUE, 0);

	/* Setuping info box */
	//info_box
		//info_box_wrapper
			//info_box_gamen_wrapper
				//info_box_gamen_label
			//info_box_tool_box
				//info_box_play_button
	GtkWidget *info_box_wrapper,
			  *info_box_tool_box,
			  *info_box_gamen_wrapper,
			  *info_box_gamen_label,
			  *info_box_play_button;

	info_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	info_box_wrapper = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_box_pack_start(GTK_BOX(info_box), info_box_wrapper, FALSE, TRUE, 0);
	gtk_box_pack_end(GTK_BOX(game_list_wrapper), info_box, FALSE, TRUE, 10);

	info_box_gamen_label = gtk_label_new("");
	gtk_widget_set_name(info_box_gamen_label, "info_box_gamen");
	info_box_gamen_wrapper = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_box_pack_start(GTK_BOX(info_box_wrapper), info_box_gamen_wrapper, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(info_box_gamen_wrapper), info_box_gamen_label, FALSE, TRUE, 0);

	/* info_box_play_button = gtk_button_new_with_label("Play"); */
	info_box_play_button = gtk_combo_box_new();
	gtk_widget_set_name(info_box_play_button, "info_box_play");
	info_box_tool_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_box_pack_start(GTK_BOX(info_box_wrapper), info_box_tool_box, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(info_box_tool_box), info_box_play_button, FALSE, TRUE, 0);

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

	g_signal_connect(G_OBJECT(window), "destroy",G_CALLBACK(quit), NULL);

	GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(game_list));
	g_signal_connect(selection, "changed", G_CALLBACK(show_sel_game_info), info_box); //showing info about game in bottom dock
	g_signal_connect(game_list, "row-activated", G_CALLBACK(view_onRowActivated), NULL);      //double-click on game list entry is running selected game
	g_signal_connect(info_box_play_button, "clicked", G_CALLBACK(play_button), game_list);    //click on "Play" button running selected game

	gtk_widget_show_all(window);
	gtk_widget_hide(info_box);
	gtk_main();

	return 0;
}
