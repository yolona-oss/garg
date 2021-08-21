#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

#include "run.h"
#include "../main.h"
#include "../scan/scan.h"
#include "../utils/eprintf.h"

/* struct gapp_t *gapp; */

GtkWidget *info_box_game_name_label;
GtkWidget *window = NULL;
GtkWidget *game_list_wrapper;
GtkWidget *game_list;

const int icon_size_w = 200;
const int icon_size_h = 80;
const int g_dock_min_size_h = 300;
const int g_dock_min_size_w = 120;

/* funcs */
static GtkTreeModel *make_model(GtkWidget *list);
static void load_css(void);
static gboolean game_list_sort(GtkTreeModel* model, int column, const char* key, GtkTreeIter* iter, gpointer search_data);

void
quit(GtkWidget *window, gpointer data)
{
	done = 1;
}

static void
load_css(void)
{
	GtkCssProvider *provider;
	GdkDisplay *dy = gdk_display_get_default();
	const char *css_settings = ".gameName { font-size: 120%; font-weight: bold; }";
	int css_settings_len = strlen(css_settings);

	provider = gtk_css_provider_new();
	gtk_css_provider_load_from_data(provider, css_settings, css_settings_len);
	gtk_style_context_add_provider_for_display(dy, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_USER);
}

static gboolean
game_list_sort(GtkTreeModel* model,
			   int column,
			   const char* key,
			   GtkTreeIter* iter,
			   gpointer search_data)
{
	
	return TRUE;
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

int
run(GtkApplication* app,
	gpointer        user_data)
{
	GtkWidget *main_box,
			  *header,
			  *main_search_bar,
			  *main_search_entry,
			  *wrapper,
			  *dock,
			  *info_box,
			  *games_window;

	gtk_init();
	load_css();

	/* -- # Shortcuts # -- */
	/* GtkShortcutTrigger *trigger = gtk_shortcut_trigger_parse_string("<Control>q"); */

	/* -- # Setuping window props # -- */
	window = gtk_application_window_new(app);
	gtk_window_set_default_size(GTK_WINDOW(window), 900, 600);

	main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_window_set_child(GTK_WINDOW(window), main_box);

	/* -- # Setuping header # -- */
	header = gtk_header_bar_new();
	gtk_window_set_titlebar(GTK_WINDOW(window), header);

	/* -- # search bar # -- */
	main_search_bar = gtk_search_bar_new();
	main_search_entry = gtk_search_entry_new();
	gtk_search_bar_connect_entry(GTK_SEARCH_BAR(main_search_bar), GTK_EDITABLE(main_search_entry));
	gtk_search_bar_set_child(GTK_SEARCH_BAR(main_search_bar), main_search_entry);
	gtk_search_bar_set_show_close_button(GTK_SEARCH_BAR(main_search_bar), FALSE);

	gtk_box_append(GTK_BOX(main_box), main_search_bar);
	gtk_search_bar_set_key_capture_widget(GTK_SEARCH_BAR(main_search_bar), window);

	GtkWidget *search_button = gtk_toggle_button_new ();
	gtk_button_set_icon_name(GTK_BUTTON (search_button), "system-search-symbolic");
	g_object_bind_property(search_button, "active",
						   main_search_bar, "search-mode-enabled",
						   G_BINDING_BIDIRECTIONAL);
	gtk_header_bar_pack_start(GTK_HEADER_BAR (header), search_button);

		//add new game
	GtkWidget *add_new_game_button = gtk_button_new_from_icon_name("list-add-symbolic");

		//props
	GtkWidget *app_options_menu = gtk_button_new_from_icon_name("open-menu-symbolic");

		//change list orient
	GtkWidget *list_type_button = gtk_button_new_from_icon_name("view-list-ordered-symbolic"); //use flowbox or treeview

	gtk_header_bar_pack_start(GTK_HEADER_BAR(header), add_new_game_button);
	gtk_header_bar_pack_end(GTK_HEADER_BAR(header), app_options_menu);
	gtk_header_bar_pack_end(GTK_HEADER_BAR(header), list_type_button);

	/* -- # Setuping main box # -- */
	wrapper = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_widget_set_hexpand(wrapper, TRUE);
	gtk_widget_set_vexpand(wrapper, TRUE);
	gtk_box_append(GTK_BOX(main_box), wrapper);

	/* -- # dock # -- */
	dock = gtk_box_new(GTK_ORIENTATION_VERTICAL, 1);
	gtk_box_set_homogeneous(GTK_BOX(dock), FALSE);
	gtk_widget_set_size_request(dock, g_dock_min_size_w, g_dock_min_size_h);

	GtkWidget *dock_library_label = gtk_label_new("Library");
	gtk_widget_set_margin_around(dock_library_label, 4);
	gtk_label_set_xalign(GTK_LABEL(dock_library_label), 0);

	PangoAttrList *attr_list = pango_attr_list_new();
	PangoFontDescription *font_desc = pango_font_description_new();
	pango_font_description_set_size(font_desc, 14 * PANGO_SCALE);
	PangoAttribute *attr = pango_attr_font_desc_new(font_desc);
	pango_attr_list_insert(attr_list, attr);
	gtk_label_set_attributes(GTK_LABEL(dock_library_label), attr_list);

	/* -- # dock menu library # -- */
	GtkWidget *dock_library_list_box = gtk_list_box_new();
	gtk_widget_set_vexpand(dock_library_list_box, TRUE);

	int icon_dock_menu_h = 50, icon_dock_menu_w = 50;
	GdkPixbuf *pixbuf = NULL;

	GtkWidget *dock_library_list_box_recent = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
	pixbuf = load_icon("assets/clock-50x50.png", NULL, icon_dock_menu_w, icon_dock_menu_h);
	gtk_box_append(GTK_BOX(dock_library_list_box_recent), gtk_image_new_from_pixbuf(pixbuf));
	GtkWidget *recent_label = gtk_label_new("Recent");
	gtk_box_append(GTK_BOX(dock_library_list_box_recent), recent_label);

	GtkWidget *dock_library_list_box_games = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
	pixbuf = load_icon("assets/game-64x64.png", NULL, icon_dock_menu_w, icon_dock_menu_h);
	gtk_box_append(GTK_BOX(dock_library_list_box_games), gtk_image_new_from_pixbuf(pixbuf));
	GtkWidget *games_label = gtk_label_new("Games");
	gtk_box_append(GTK_BOX(dock_library_list_box_games), games_label);

	GtkWidget *dock_library_list_box_favorites = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
	pixbuf = load_icon("assets/favorite-50x50.png", NULL, icon_dock_menu_w, icon_dock_menu_h);
	gtk_box_append(GTK_BOX(dock_library_list_box_favorites), gtk_image_new_from_pixbuf(pixbuf));
	GtkWidget *favorites_label = gtk_label_new("Favorites");
	gtk_box_append(GTK_BOX(dock_library_list_box_favorites), favorites_label);

	gtk_list_box_insert(GTK_LIST_BOX(dock_library_list_box), dock_library_list_box_games, 0);
	gtk_list_box_insert(GTK_LIST_BOX(dock_library_list_box), dock_library_list_box_favorites, 1);
	gtk_list_box_insert(GTK_LIST_BOX(dock_library_list_box), dock_library_list_box_recent, 2);

	gtk_list_box_select_row(GTK_LIST_BOX(dock_library_list_box),
			gtk_list_box_get_row_at_index(GTK_LIST_BOX(dock_library_list_box), 0));

	gtk_box_append(GTK_BOX(dock), dock_library_label);
	gtk_box_append(GTK_BOX(dock), dock_library_list_box);
	gtk_box_append(GTK_BOX(wrapper), dock);
	gtk_box_append(GTK_BOX(wrapper), gtk_separator_new(GTK_ORIENTATION_VERTICAL));

	/* -- # game list wrapper # -- */
	game_list_wrapper = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_box_append(GTK_BOX(wrapper), game_list_wrapper);

	/* -- # Setuping info bar # -- */
	GtkWidget *info_box_tool_box,
			  *info_box_play_button;

	info_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_widget_set_margin_around(info_box, 10);
	gtk_box_append(GTK_BOX(game_list_wrapper), info_box);

	info_box_game_name_label = gtk_label_new("");
	gtk_widget_add_css_class(info_box_game_name_label, "gameName");
	gtk_widget_set_margin_bottom(info_box_game_name_label, 10);
	gtk_label_set_xalign(GTK_LABEL(info_box_game_name_label), 0);
	gtk_box_append(GTK_BOX(info_box), info_box_game_name_label);

	info_box_tool_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_box_append(GTK_BOX(info_box), info_box_tool_box);
	info_box_play_button = gtk_button_new_with_label("Play");
	/* info_box_play_button = gtk_combo_box_new(); */
	gtk_box_append(GTK_BOX(info_box_tool_box), info_box_play_button);

	/* -- # Setuping scrolled window for games list # -- */
	games_window = gtk_scrolled_window_new();
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(games_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_widget_set_hexpand(games_window, TRUE);
	gtk_widget_set_vexpand(games_window, TRUE);
	gtk_box_append(GTK_BOX(game_list_wrapper), games_window);

	/* Initializing game list */
	game_list = gtk_tree_view_new();
	GtkTreeModel *store = make_model(game_list);
	gtk_tree_view_set_model(GTK_TREE_VIEW(game_list), GTK_TREE_MODEL(store));
	/* gtk_tree_view_set_column_drag_function(GTK_TREE_VIEW(game_list), GtkTreeViewColumnDropFunc, NULL, NULL); */
	gtk_tree_view_set_search_column(GTK_TREE_VIEW(game_list), NAME_C);
	gtk_tree_view_set_search_entry(GTK_TREE_VIEW(game_list), GTK_EDITABLE(main_search_entry));
	gtk_tree_view_set_enable_search(GTK_TREE_VIEW(game_list), TRUE);
	gtk_tree_view_set_search_equal_func(GTK_TREE_VIEW(game_list), game_list_sort, 0, 0);
	g_object_unref(store);

	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(game_list), TRUE);
	gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(games_window), game_list);

	/* -- # Showing game entries # -- */
	setup_game_entries(game_list);

	GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(game_list));
	g_signal_connect(selection, "changed", G_CALLBACK(show_sel_game_info), info_box);		//showing info about game in bottom dock
	g_signal_connect(game_list, "row-activated", G_CALLBACK(game_entry_clicked), NULL);		//double-click on game list entry is running selected game
	g_signal_connect(info_box_play_button, "clicked", G_CALLBACK(play_button_clicked), game_list);	//click on "Play" button running selected game
	g_signal_connect(add_new_game_button, "clicked", G_CALLBACK(add_new_game_dialog), window);	//call dialog for adding new game

	g_signal_connect(G_OBJECT(window), "destroy",G_CALLBACK(quit), NULL);

	/* gtk_widget_set_focus_child(window, game_list); */

	if (!gtk_widget_get_visible(window))
		gtk_widget_show(window);
	else
		gtk_window_destroy(GTK_WINDOW(window));
	gtk_widget_hide(info_box);

	return 0;
}
