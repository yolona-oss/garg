#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

#include "run.h"
#include "../main.h"
#include "../games/gamerec.h"
#include "../scan/scan.h"
#include "../db/dbman.h"
#include "../utils/eprintf.h"
#include "../utils/util.h"
#include "../utils/gtk_widget_list.h"

static const int icon_size_w = 200;
static const int icon_size_h = 80;
static const char *game_icon_default = "assets/game-icon.png";
static const int g_dock_min_size_h = 300;
static const int g_dock_min_size_w = 120;

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
/* static GtkWidget *setup_menu_bar(); */
static void init_game_tab(void);
static GtkWidget *find_child(GtkWidget* parent, const gchar* name);
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

static GtkTreeModel *
make_model(GtkWidget *list)
{
	GtkCellRenderer *renderer, *renderer_pixbuf;
	GtkTreeViewColumn  *icon_col,
					   *name_col,
					   *gener_col,
					   *last_time_col,
					   *play_time_col,
					   *id_col;
	GtkListStore *store;

	renderer = gtk_cell_renderer_text_new();
	renderer_pixbuf = gtk_cell_renderer_pixbuf_new();

	icon_col = gtk_tree_view_column_new_with_attributes("",
			renderer_pixbuf, "pixbuf", ICON_C, NULL);
	gtk_tree_view_column_set_sizing(GTK_TREE_VIEW_COLUMN(icon_col),
			GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_fixed_width(GTK_TREE_VIEW_COLUMN(icon_col), icon_size_w);
	gtk_tree_view_append_column(GTK_TREE_VIEW(list), icon_col);

	name_col = gtk_tree_view_column_new_with_attributes("Name",
			renderer, "text", NAME_C, NULL);
	gtk_tree_view_column_set_sort_column_id(name_col, NAME_C);
	gtk_tree_view_append_column(GTK_TREE_VIEW(list), name_col);

	gener_col = gtk_tree_view_column_new_with_attributes("Gener",
			renderer, "text", GENER_C, NULL);
	gtk_tree_view_column_set_sort_column_id(gener_col, GENER_C);
	gtk_tree_view_append_column(GTK_TREE_VIEW(list), gener_col);

	last_time_col = gtk_tree_view_column_new_with_attributes("Last Played",
			renderer, "text", LAST_TIME_C, NULL);
	gtk_tree_view_column_set_sort_column_id(last_time_col, LAST_TIME_C);
	gtk_tree_view_append_column(GTK_TREE_VIEW(list), last_time_col);

	play_time_col = gtk_tree_view_column_new_with_attributes("Play Time",
			renderer, "text", PLAY_TIME_C, NULL);
	gtk_tree_view_column_set_sort_column_id(play_time_col, PLAY_TIME_C);
	gtk_tree_view_append_column(GTK_TREE_VIEW(list), play_time_col);

	id_col = gtk_tree_view_column_new_with_attributes("ID",
			renderer, "text", ID_C, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(list), id_col);
	gtk_tree_view_column_set_visible(id_col, FALSE);

	gtk_tree_view_set_tooltip_column(GTK_TREE_VIEW(list), NAME_C);
	store = gtk_list_store_new(N_COLUMNS,
			GDK_TYPE_PIXBUF,
			G_TYPE_STRING,
			G_TYPE_STRING,
			G_TYPE_STRING,
			G_TYPE_STRING,
			G_TYPE_UINT);

	return GTK_TREE_MODEL(store);
}

static int
setup_game_entries(GtkWidget *list)
{
	GtkListStore *store;
	GtkTreeIter iter;
	/* GtkTreeModelFilter *filtered; */
	/* GtkTreeModelSort* sorted; */

	init_game_tab();
	store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(list)));
	for (int i = 0; i < gr_tab.ngames; i++)
	{
		gtk_list_store_append(store, &iter);

		/* setuping last play time variable */
		char *last_time = gr_tab.game_rec[i].last_time ? ctime(&gr_tab.game_rec[i].last_time) : "";
		if (last_time[0] != '\0') last_time[strlen(last_time)-1] = '\0'; //ctime use \n simbol in end, deleting it

		gtk_list_store_set(store, &iter, ICON_C, load_icon(gr_tab.game_rec[i]),
										 NAME_C, gr_tab.game_rec[i].name,
										 GENER_C, gr_tab.game_rec[i].gener ? gr_tab.game_rec[i].gener : "",
										 LAST_TIME_C, last_time,
										 PLAY_TIME_C, play_time_human(&gr_tab.game_rec[i]),
										 ID_C, gr_tab.game_rec[i].id,
										 -1);
	}

	return 0;
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

		game_t *gr = grt_find(id);
		/* setuping last play time variable */
		char *last_time = gr->last_time ? ctime(&gr->last_time) : "";
		if (last_time[0] != '\0') str_del_last_sym(last_time);

		gtk_list_store_set(GTK_LIST_STORE(model), &iter, LAST_TIME_C, last_time, -1);
		gtk_list_store_set(GTK_LIST_STORE(model), &iter, PLAY_TIME_C, play_time_human(gr), -1);
		sqlite3 *db = db_init();
		db_upd_rec(db, gr);
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

		game_t *gr = grt_find(id);
		/* setuping last play time variable */
		char *last_time = gr->last_time ? ctime(&gr->last_time) : "";
		if (last_time[0] != '\0') str_del_last_sym(last_time);

		gtk_list_store_set(GTK_LIST_STORE(model), &iter, LAST_TIME_C, last_time, -1);
		gtk_list_store_set(GTK_LIST_STORE(model), &iter, PLAY_TIME_C, play_time_human(gr), -1);

		sqlite3 *db = db_init();
		db_upd_rec(db, gr);
		/* gtk_button_set_label(widget, "Stop"); */
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

	/* Setuping window props */
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), "GArg");
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_NONE);
	gtk_container_set_border_width(GTK_CONTAINER(window), 1);
	gtk_window_set_default_size(GTK_WINDOW(window), 900, 600);

	main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_container_add(GTK_CONTAINER(window), main_box);

	/* Setuping header */
	header = gtk_header_bar_new();
	main_search_entry = gtk_search_entry_new();
	gtk_entry_set_placeholder_text(GTK_ENTRY(main_search_entry), "Search games");
	gtk_entry_set_alignment(GTK_ENTRY(main_search_entry), 0);
	/* gint h = 10, */
	/* 	 ph = 10; */
	/* gtk_widget_get_preferred_height_and_baseline_for_width(main_search_entry, 100, &h, &ph, 0, 0); */
	/* gtk_widget_get_preferred_size(main_search_entry, sizereq, sizereq); */
	gtk_header_bar_set_custom_title(GTK_HEADER_BAR(header), main_search_entry);

	GtkWidget *add_new_game_button = gtk_button_new_from_icon_name("gtk-add", GTK_ICON_SIZE_BUTTON);
	GtkWidget *app_options_menu = gtk_button_new_from_icon_name("gtk-properties", GTK_ICON_SIZE_BUTTON);
	GtkWidget *list_type_button = gtk_button_new_with_label("change list orient");
	gtk_header_bar_pack_start(GTK_HEADER_BAR(header), add_new_game_button);
	gtk_header_bar_pack_end(GTK_HEADER_BAR(header), app_options_menu);
	gtk_header_bar_pack_end(GTK_HEADER_BAR(header), list_type_button);

	gtk_box_pack_start(GTK_BOX(main_box), header, FALSE, TRUE, 0);

	/* Setuping main part */
	wrapper = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_box_pack_start(GTK_BOX(main_box), wrapper, TRUE, TRUE, 0);

	/* dock */
	dock = gtk_box_new(GTK_ORIENTATION_VERTICAL, 1);
	gtk_widget_set_margin_start(dock, 4);
	gtk_widget_set_margin_end(dock, 4);
	gtk_box_set_homogeneous(GTK_BOX(dock), FALSE);
	gtk_widget_set_size_request(dock, g_dock_min_size_w, g_dock_min_size_h);

	GtkWidget *dock_library_label = gtk_label_new("Library");
	gtk_label_set_xalign(GTK_LABEL(dock_library_label), 0);
	PangoAttrList *attr_list = pango_attr_list_new();
	PangoFontDescription *font_desc = pango_font_description_new();
	pango_font_description_set_size(font_desc, 14 * PANGO_SCALE);
	PangoAttribute *attr = pango_attr_font_desc_new(font_desc);
	pango_attr_list_insert(attr_list, attr);
	gtk_label_set_attributes(GTK_LABEL(dock_library_label), attr_list);

	/* dock menu library */
	GtkWidget *dock_library_list_box = gtk_list_box_new();
	GtkWidget *dock_library_list_box_recent    = gtk_label_new("Recent");
	GtkWidget *dock_library_list_box_games     = gtk_label_new("Games");
	GtkWidget *dock_library_list_box_favorites = gtk_label_new("Favorites");

	gtk_label_set_attributes(GTK_LABEL(dock_library_list_box_games), attr_list);
	gtk_label_set_attributes(GTK_LABEL(dock_library_list_box_recent), attr_list);
	gtk_label_set_attributes(GTK_LABEL(dock_library_list_box_favorites), attr_list);
	gtk_label_set_xalign(GTK_LABEL(dock_library_list_box_recent), 0);
	gtk_label_set_xalign(GTK_LABEL(dock_library_list_box_games), 0);
	gtk_label_set_xalign(GTK_LABEL(dock_library_list_box_favorites), 0);

	gtk_list_box_prepend(GTK_LIST_BOX(dock_library_list_box), dock_library_list_box_favorites);
	gtk_list_box_prepend(GTK_LIST_BOX(dock_library_list_box), dock_library_list_box_recent);
	gtk_list_box_prepend(GTK_LIST_BOX(dock_library_list_box), dock_library_list_box_games);
	gtk_list_box_select_row(dock_library_list_box,
			gtk_list_box_get_row_at_index(dock_library_list_box, 0));

	gtk_box_pack_start(GTK_BOX(dock), dock_library_label, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(dock), dock_library_list_box, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(wrapper), dock, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(wrapper), gtk_separator_new(GTK_ORIENTATION_VERTICAL), FALSE, TRUE, 1);
	gtk_container_set_border_width(GTK_CONTAINER(dock), 0);

	//game list
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

	info_box_play_button = gtk_button_new_with_label("Play");
	/* info_box_play_button = gtk_combo_box_new(); */
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
	GtkTreeModel *store = make_model(game_list);
	gtk_tree_view_set_model(GTK_TREE_VIEW(game_list),
			GTK_TREE_MODEL(store));
	/* gtk_tree_view_set_column_drag_function(GTK_TREE_VIEW(game_list), GtkTreeViewColumnDropFunc, NULL, NULL); */
	gtk_tree_view_set_search_column(GTK_TREE_VIEW(game_list), NAME_C);
	g_object_unref(store);

	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(game_list), TRUE);
	gtk_container_add(GTK_CONTAINER(games_window), game_list);
	/* gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(games_window), game_list); */

	/* Showing game entries */
	setup_game_entries(game_list);

	g_signal_connect(G_OBJECT(window), "destroy",G_CALLBACK(quit), NULL);

	GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(game_list));
	g_signal_connect(selection, "changed", G_CALLBACK(show_sel_game_info), info_box); //showing info about game in bottom dock
	g_signal_connect(game_list, "row-activated", G_CALLBACK(view_onRowActivated), NULL);      //double-click on game list entry is running selected game
	g_signal_connect(info_box_play_button, "clicked", G_CALLBACK(play_button), game_list);    //click on "Play" button running selected game

	gtk_container_set_focus_child(GTK_CONTAINER(window), game_list);

	gtk_widget_show_all(window);
	gtk_widget_hide(info_box);
	gtk_main();

	return 0;
}
