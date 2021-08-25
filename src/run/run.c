#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

#include "run.h"
#include "../main.h"
#include "../scan/scan.h"
#include "../utils/eprintf.h"

struct gapp_t gapp;

const int icon_size_w = 200;
const int icon_size_h = 80;
const int g_dock_min_size_h = 300;
const int g_dock_min_size_w = 120;

/* funcs */
static GtkTreeModel *make_model(GtkTreeView *list);
static void load_css(void);
static gboolean game_list_sort_func(GtkTreeModel* model, int column, const char* key, GtkTreeIter* iter, gpointer search_data);

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
game_list_sort_func(GtkTreeModel* model,
			   int column,
			   const char* key,
			   GtkTreeIter* iter,
			   gpointer search_data)
{
	
	return TRUE;
}

static GtkTreeModel *
make_model(GtkTreeView *list)
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
	gtk_init();
	load_css();

	/* -- # Shortcuts # -- */
	/* GtkShortcutTrigger *trigger = gtk_shortcut_trigger_parse_string("<Control>q"); */

	/* -- # Setuping window props # -- */
	gapp.window = GTK_WINDOW(gtk_application_window_new(app));
	gtk_window_set_default_size(gapp.window, 900, 600);

	gapp.main_box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 0));
	gtk_window_set_child(gapp.window, GTK_WIDGET(gapp.main_box));

	/* -- # Setuping header # -- */
	gapp.header.bar = GTK_HEADER_BAR(gtk_header_bar_new());
	gtk_window_set_titlebar(gapp.window, GTK_WIDGET(gapp.header.bar));

	/* -- # search bar # -- */
	gapp.header.search_bar = GTK_SEARCH_BAR(gtk_search_bar_new());
	gapp.header.search_entry = GTK_SEARCH_ENTRY(gtk_search_entry_new());
	gtk_search_bar_connect_entry(gapp.header.search_bar, GTK_EDITABLE(gapp.header.search_entry));
	gtk_search_bar_set_child(gapp.header.search_bar, GTK_WIDGET(gapp.header.search_entry));
	gtk_search_bar_set_show_close_button(gapp.header.search_bar, FALSE);

	gtk_box_append(gapp.main_box, GTK_WIDGET(gapp.header.search_bar));
	gtk_search_bar_set_key_capture_widget(gapp.header.search_bar, GTK_WIDGET(gapp.window));

	gapp.header.search_button = GTK_BUTTON(gtk_toggle_button_new());
	gtk_button_set_icon_name(GTK_BUTTON (gapp.header.search_button), "system-search-symbolic");
	g_object_bind_property(gapp.header.search_button, "active",
						   gapp.header.search_bar, "search-mode-enabled",
						   G_BINDING_BIDIRECTIONAL);
	gtk_header_bar_pack_start(gapp.header.bar, GTK_WIDGET(gapp.header.search_button));

		//add new game
	gapp.header.add_new_game_button	= GTK_BUTTON(gtk_button_new_from_icon_name("list-add-symbolic"));

		//props
	gapp.header.app_options_menu		= GTK_BUTTON(gtk_button_new_from_icon_name("open-menu-symbolic"));

		//change list orient
	gapp.header.list_type_button		= GTK_BUTTON(gtk_button_new_from_icon_name("view-list-ordered-symbolic")); //use flowbox or treeview

	gtk_header_bar_pack_start(gapp.header.bar, GTK_WIDGET(gapp.header.add_new_game_button));
	gtk_header_bar_pack_end(gapp.header.bar,   GTK_WIDGET(gapp.header.app_options_menu));
	gtk_header_bar_pack_end(gapp.header.bar,   GTK_WIDGET(gapp.header.list_type_button));

	/* -- # Setuping main box # -- */
	gapp.wrapper = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));
	gtk_widget_set_hexpand(GTK_WIDGET(gapp.wrapper), TRUE);
	gtk_widget_set_vexpand(GTK_WIDGET(gapp.wrapper), TRUE);
	gtk_box_append(gapp.main_box, GTK_WIDGET(gapp.wrapper));

	/* -- # dock # -- */
	gapp.dock.box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 1));
	gtk_box_set_homogeneous(gapp.dock.box, FALSE);
	gtk_widget_set_size_request(GTK_WIDGET(gapp.dock.box),
								g_dock_min_size_w, g_dock_min_size_h);

	gapp.dock.library_label = GTK_LABEL(gtk_label_new("Library"));
	gtk_widget_set_margin_around(GTK_WIDGET(gapp.dock.library_label), 4);
	gtk_label_set_xalign(gapp.dock.library_label, 0);

	PangoAttrList *attr_list = pango_attr_list_new();
	PangoFontDescription *font_desc = pango_font_description_new();
	pango_font_description_set_size(font_desc, 14 * PANGO_SCALE);
	PangoAttribute *attr = pango_attr_font_desc_new(font_desc);
	pango_attr_list_insert(attr_list, attr);
	gtk_label_set_attributes(GTK_LABEL(gapp.dock.library_label), attr_list);

	/* -- # dock menu library # -- */
	gapp.dock.library_list = GTK_LIST_BOX(gtk_list_box_new());
	gtk_widget_set_vexpand(GTK_WIDGET(gapp.dock.library_list), TRUE);

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

	gtk_list_box_insert(gapp.dock.library_list, dock_library_list_box_games, 0);
	gtk_list_box_insert(gapp.dock.library_list, dock_library_list_box_favorites, 1);
	gtk_list_box_insert(gapp.dock.library_list, dock_library_list_box_recent, 2);

	gtk_list_box_select_row(gapp.dock.library_list,
			gtk_list_box_get_row_at_index(gapp.dock.library_list, 0));

	gtk_box_append(gapp.dock.box, GTK_WIDGET(gapp.dock.library_label));
	gtk_box_append(gapp.dock.box, GTK_WIDGET(gapp.dock.library_list));
	gtk_box_append(gapp.wrapper, GTK_WIDGET(gapp.dock.box));
	gtk_box_append(gapp.wrapper, gtk_separator_new(GTK_ORIENTATION_VERTICAL));

	/* -- # game list wrapper # -- */
	gapp.game_list_wrapper = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 0));
	gtk_box_append(gapp.wrapper, GTK_WIDGET(gapp.game_list_wrapper));

	/* -- # Setuping info bar # -- */
	gapp.game_info_bar.box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 0));
	gtk_widget_set_margin_around(GTK_WIDGET(gapp.game_info_bar.box), 10);
	gtk_box_prepend(gapp.game_list_wrapper, GTK_WIDGET(gapp.game_info_bar.box));

	gapp.game_info_bar.game_name = GTK_LABEL(gtk_label_new(""));
	gtk_widget_add_css_class(GTK_WIDGET(gapp.game_info_bar.game_name), "gameName");
	gtk_widget_set_margin_bottom(GTK_WIDGET(gapp.game_info_bar.game_name), 10);
	gtk_label_set_xalign(gapp.game_info_bar.game_name, 0);
	gtk_box_append(gapp.game_info_bar.box, GTK_WIDGET(gapp.game_info_bar.game_name));

	gapp.game_info_bar.tool_box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));
	gtk_box_append(gapp.game_info_bar.box, GTK_WIDGET(gapp.game_info_bar.tool_box));
	gapp.game_info_bar.play = GTK_BUTTON(gtk_button_new_with_label("Play"));
	/* info_box_play_button = gtk_combo_box_new(); */
	gtk_box_append(gapp.game_info_bar.tool_box, GTK_WIDGET(gapp.game_info_bar.play));

	/* -- # Setuping scrolled window for games list # -- */
	gapp.games_window = GTK_SCROLLED_WINDOW(gtk_scrolled_window_new());
	gtk_scrolled_window_set_policy(gapp.games_window, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_widget_set_hexpand(GTK_WIDGET(gapp.games_window), TRUE);
	gtk_widget_set_vexpand(GTK_WIDGET(gapp.games_window), TRUE);
	gtk_box_prepend(gapp.game_list_wrapper, GTK_WIDGET(gapp.games_window));

	/* Initializing game list */
	gapp.game_list = GTK_TREE_VIEW(gtk_tree_view_new());
	gapp.game_list_store = make_model(gapp.game_list);
	gtk_tree_view_set_model(gapp.game_list, gapp.game_list_store);
	/* gtk_tree_view_set_column_drag_function(GTK_TREE_VIEW(game_list), GtkTreeViewColumnDropFunc, NULL, NULL); */
	gtk_tree_view_set_search_column(gapp.game_list, NAME_C);
	gtk_tree_view_set_search_entry(gapp.game_list, GTK_EDITABLE(gapp.header.search_entry));
	gtk_tree_view_set_enable_search(gapp.game_list, TRUE);
	gtk_tree_view_set_search_equal_func(gapp.game_list, game_list_sort_func, 0, 0);
	g_object_unref(gapp.game_list_store);

	gtk_tree_view_set_headers_visible(gapp.game_list, TRUE);
	gtk_scrolled_window_set_child(gapp.games_window, GTK_WIDGET(gapp.game_list));

	/* -- # Showing game entries # -- */
	setup_game_entries(gapp.game_list);

	GtkTreeSelection *selection = gtk_tree_view_get_selection(gapp.game_list);
	g_signal_connect(G_OBJECT(selection), "changed", G_CALLBACK(show_sel_game_info), gapp.game_info_bar.box);		//showing info about game in bottom dock
	g_signal_connect(G_OBJECT(gapp.game_list), "row-activated", G_CALLBACK(game_entry_clicked), NULL);		//double-click on game list entry is running selected game
	g_signal_connect(G_OBJECT(gapp.game_info_bar.play), "clicked", G_CALLBACK(play_button_clicked), gapp.game_list);	//click on "Play" button running selected game
	g_signal_connect(G_OBJECT(gapp.header.add_new_game_button), "clicked", G_CALLBACK(add_new_game_dialog), gapp.window);	//call dialog for adding new game

	g_signal_connect(G_OBJECT(gapp.window), "destroy",G_CALLBACK(quit), NULL);

	/* gtk_widget_set_focus_child(window, game_list); */

	if (!gtk_widget_get_visible(GTK_WIDGET(gapp.window)))
		gtk_widget_show(GTK_WIDGET(gapp.window));
	else
		gtk_window_destroy(gapp.window);
	gtk_widget_hide(GTK_WIDGET(gapp.game_info_bar.box));

	return 0;
}
