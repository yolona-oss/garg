#include <glib/gi18n.h>

#include "run.h"
#include "../scan/scan.h"
#include "../games/gamerec.h"
#include "../utils/util.h"
#include "../utils/eprintf.h"

static GtkEntryBuffer *bgame_name,
			   *bgame_location,
			   *bgame_sp,
			   *bgame_start_arg,
			   *bgame_uninstaller;

enum ADD_NEW_GAME_TRY_FIND {
	ADD_NEW_GAME_TRY_FIND_SP,
	ADD_NEW_GAME_TRY_FIND_UNINSTALLER,
	ADD_NEW_GAME_TRY_FIND_ICON,
};

//TODO
void
quick_message(GtkWindow *parent, gchar *message)
{
	GtkWidget *dialog;

	dialog = gtk_message_dialog_new(GTK_WINDOW(parent),
							GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
							GTK_MESSAGE_INFO,
							GTK_BUTTONS_OK,
							"Info message");
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog), "%s", message);
	g_signal_connect (dialog, "response", G_CALLBACK (gtk_window_destroy), NULL);
	gtk_widget_show(dialog);
}

void
info_msg(const char *text)
{
	GtkWidget *message_label;
	GtkWidget *widget;
	GtkWidget *grid;
	GtkInfoBar *bar;

	// set up info bar
	widget = gtk_info_bar_new();
	bar = GTK_INFO_BAR(widget);
	gtk_box_append(game_list_wrapper, bar);
	grid = gtk_grid_new();
	message_label = gtk_label_new("");
	gtk_info_bar_add_child (bar, message_label);
	gtk_info_bar_add_button(bar,
						_("_OK"),
						GTK_RESPONSE_OK);
	g_signal_connect(bar,
					"response",
					G_CALLBACK(gtk_widget_hide),
					NULL);
	gtk_grid_attach(GTK_GRID(grid),
					widget,
					0, 2, 1, 1);

	gtk_label_set_text (GTK_LABEL (message_label), text);
	gtk_info_bar_set_message_type (bar, GTK_MESSAGE_ERROR);
	gtk_widget_show(GTK_WIDGET(bar));
}

void
add_new_game_try_find(GtkWindow *widget, enum ADD_NEW_GAME_TRY_FIND point)
{
	game_t *gr = NULL;
	const char *loc = gtk_entry_buffer_get_text(bgame_location);

	if (gtk_entry_buffer_get_length(bgame_location) > 1)
	{
		if (isExist(loc))
		{
			gr = scan_game_dir("", loc);
			if (gr) {
				if (point == ADD_NEW_GAME_TRY_FIND_SP) {
					gtk_entry_buffer_insert_text(bgame_sp, 0, gr->start_point, strlen(gr->start_point));
				} else if (point == ADD_NEW_GAME_TRY_FIND_UNINSTALLER) {
					gtk_entry_buffer_insert_text(bgame_uninstaller, 0, gr->uninstaller, strlen(gr->uninstaller));
				}
			} else {
				info_msg("Nothing found");
			}
		}
		else
		{
			char msg[1000];
			esnprintf(msg, sizeof(msg),
					"Directory \"%s\" does not exist", loc);
			quick_message(GTK_WINDOW(window), msg);
		}
	}
	else
	{
		quick_message(GTK_WINDOW(window), "First fill in field \"Game root directory\"");
	}
}

gboolean
end_add_new_game_dialog(GtkWidget *dialog,
						gint respons_id,
						gpointer data)
{
	if (respons_id == GTK_RESPONSE_OK)
	{
		if (!bgame_name) {
		} else {
		}
		if (!gtk_entry_buffer_get_length(bgame_sp)) {
		} else {
			if (!isExist(
		}
		if (!bgame_location) {
		} else {
		}
		if (!bgame_start_arg) {
		} else {
		}
		if (!bgame_uninstaller) {
		} else {
		}
	} else if (respons_id == GTK_RESPONSE_CANCEL)
	{

	} else {
		quick_message(GTK_WINDOW(dialog), "some error when adding new game!");
	}

	gtk_window_destroy(GTK_WINDOW(dialog));

	return TRUE;
}

static void
add_new_game_chooser_file_change(GtkWidget* self, int response)
{
	if (response == GTK_RESPONSE_ACCEPT)
	{
		GtkFileChooser *chooser = GTK_FILE_CHOOSER(self);
		GFile *file = gtk_file_chooser_get_file(chooser);
		char *loc = g_file_get_path(file);
		printf("%s\n", loc);
		gtk_entry_buffer_insert_text(bgame_location, 0, loc, strlen(loc));
		gtk_entry_buffer_insert_text(bgame_sp, 0, loc, strlen(loc));
		gtk_entry_buffer_insert_text(bgame_uninstaller, 0, loc, strlen(loc));
	}

	gtk_window_destroy (GTK_WINDOW (self));
}

void
new_filechooser_dialog(GtkWidget *w, GtkWindow *parent)
{
	GtkWidget *dialog;
	GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER;

	dialog = gtk_file_chooser_dialog_new("Open File",
										GTK_WINDOW(parent),
										action,
										"_Cancel",
										GTK_RESPONSE_CANCEL,
										"_Open",
										GTK_RESPONSE_ACCEPT,
										NULL);
	gtk_widget_show(dialog);

	g_signal_connect(GTK_FILE_CHOOSER(dialog), "response",
					G_CALLBACK(add_new_game_chooser_file_change),
					NULL);
}

void
add_new_game_dialog(GtkButton *button,
					gpointer   parent)
{
	GtkWidget *dialog,
			  *content_area,
			  *main_box,
			  *game_box;
	GtkWidget *egame_name,
			  *egame_location,
			  *egame_sp,
			  *egame_start_arg,
			  *egame_uninstaller;
	GtkWidget *lgame_name,
			  *lgame_location,
			  *lgame_sp,
			  *lgame_start_arg,
			  *lgame_uninstaller;
	GtkWidget *game_frame;
	GtkWidget *game_grid;
	GtkWidget *picker;
	GtkWidget *bgame_sp_finder,
			  *bgame_uninstaller_finder;

	dialog = gtk_dialog_new_with_buttons("Adding new game",
										GTK_WINDOW(parent),
										GTK_DIALOG_MODAL| GTK_DIALOG_DESTROY_WITH_PARENT|GTK_DIALOG_USE_HEADER_BAR,
										_("_Add"), GTK_RESPONSE_OK,
										_("_Close"), GTK_RESPONSE_CANCEL,
										NULL);
	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_CANCEL);

	content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

	main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	game_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

	gtk_window_set_child(GTK_WINDOW(dialog), content_area);
	gtk_box_append(GTK_BOX(content_area), main_box);
	gtk_box_append(GTK_BOX(main_box), game_box);

	bgame_name = gtk_entry_buffer_new("", 0);
	bgame_location = gtk_entry_buffer_new("", 0);
	bgame_sp = gtk_entry_buffer_new("", 0);
	bgame_uninstaller = gtk_entry_buffer_new("", 0);
	bgame_start_arg = gtk_entry_buffer_new("", 0);

	lgame_name = gtk_label_new_with_mnemonic("_Name");
	lgame_location = gtk_label_new_with_mnemonic("_Game root director");
	lgame_sp = gtk_label_new_with_mnemonic("_Start point");
	lgame_start_arg = gtk_label_new_with_mnemonic("_Start arguments");
	lgame_uninstaller = gtk_label_new_with_mnemonic("_Uninstaller");
	gtk_label_set_xalign(GTK_LABEL(lgame_name), 0);
	gtk_label_set_xalign(GTK_LABEL(lgame_sp), 0);
	gtk_label_set_xalign(GTK_LABEL(lgame_location), 0);
	gtk_label_set_xalign(GTK_LABEL(lgame_start_arg), 0);
	gtk_label_set_xalign(GTK_LABEL(lgame_uninstaller), 0);

	egame_name = gtk_entry_new_with_buffer(bgame_name);
	gtk_entry_set_placeholder_text(GTK_ENTRY(egame_name), "Name");
	gtk_label_set_mnemonic_widget(GTK_LABEL(lgame_name), egame_name);

	egame_location = gtk_entry_new_with_buffer(bgame_location);
	gtk_entry_set_placeholder_text(GTK_ENTRY(egame_location), "Location");
	gtk_label_set_mnemonic_widget(GTK_LABEL(lgame_location), egame_location);

	egame_sp = gtk_entry_new_with_buffer(bgame_sp);
	gtk_entry_set_placeholder_text(GTK_ENTRY(egame_sp), "Start point");
	gtk_label_set_mnemonic_widget(GTK_LABEL(lgame_sp), egame_sp);

	egame_uninstaller = gtk_entry_new_with_buffer(bgame_uninstaller);
	gtk_entry_set_placeholder_text(GTK_ENTRY(egame_uninstaller), "Uninstaller");
	gtk_label_set_mnemonic_widget(GTK_LABEL(lgame_uninstaller), egame_uninstaller);

	egame_start_arg = gtk_entry_new_with_buffer(bgame_start_arg);
	gtk_entry_set_placeholder_text(GTK_ENTRY(egame_start_arg), "Start arguments");
	gtk_label_set_mnemonic_widget(GTK_LABEL(lgame_start_arg), egame_start_arg);

	bgame_sp_finder = gtk_button_new_with_label("Try find");
	bgame_uninstaller_finder = gtk_button_new_with_label("Try find");
	g_signal_connect(G_OBJECT(bgame_sp_finder), "clicked",
			G_CALLBACK(add_new_game_try_find), ADD_NEW_GAME_TRY_FIND_SP);
	g_signal_connect(G_OBJECT(bgame_uninstaller_finder), "clicked",
			G_CALLBACK(add_new_game_try_find), ADD_NEW_GAME_TRY_FIND_UNINSTALLER);

	picker = gtk_button_new_with_label("Choose");
	g_signal_connect(G_OBJECT(picker), "clicked",
			G_CALLBACK(new_filechooser_dialog), dialog);

	game_frame = gtk_frame_new("Game");
	gtk_widget_set_margin_around(game_frame, 8);
	gtk_box_append(GTK_BOX(game_box), game_frame);

	game_grid = gtk_grid_new();
	gtk_widget_set_margin_around(game_grid, 8);
	gtk_frame_set_child(GTK_FRAME(game_frame), game_grid);

	gtk_grid_set_row_spacing(GTK_GRID(game_grid), 4);
	gtk_grid_set_column_spacing(GTK_GRID(game_grid), 10);

	gtk_grid_attach(GTK_GRID(game_grid), lgame_name,		0, 0, 1, 1);
	gtk_grid_attach(GTK_GRID(game_grid), egame_name,		1, 0, 2, 1);

	gtk_grid_attach(GTK_GRID(game_grid), lgame_location,	0, 1, 1, 1);
	gtk_grid_attach(GTK_GRID(game_grid), egame_location,	1, 1, 1, 1);
	gtk_grid_attach(GTK_GRID(game_grid), picker,			2, 1, 1, 1);

	gtk_grid_attach(GTK_GRID(game_grid), lgame_sp,			0, 2, 1, 1);
	gtk_grid_attach(GTK_GRID(game_grid), egame_sp,			1, 2, 1, 1);
	gtk_grid_attach(GTK_GRID(game_grid), bgame_sp_finder,	2, 2, 1, 1);

	gtk_grid_attach(GTK_GRID(game_grid), lgame_start_arg,	0, 3, 1, 1);
	gtk_grid_attach(GTK_GRID(game_grid), egame_start_arg,	1, 3, 2, 1);

	gtk_grid_attach(GTK_GRID(game_grid), lgame_uninstaller,	0, 4, 1, 1);
	gtk_grid_attach(GTK_GRID(game_grid), egame_uninstaller,	1, 4, 1, 1);
	gtk_grid_attach(GTK_GRID(game_grid), bgame_uninstaller_finder,	2, 4, 1, 1);

	g_signal_connect_swapped(dialog,
							"response",
							G_CALLBACK(end_add_new_game_dialog),
							dialog);

	/* gtk_widget_show_all(dialog); */
	gtk_widget_show(dialog);

}
