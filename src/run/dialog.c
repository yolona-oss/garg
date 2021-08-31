#include <glib/gi18n.h>

#include "run.h"
#include "../scan/scan.h"
#include "../games/gamerec.h"
#include "../utils/util.h"
#include "../utils/eprintf.h"

struct new_game_t {
	GtkEntryBuffer *bgame_name,
				   *bgame_location,
				   *bgame_sp,
				   *bgame_start_arg,
				   *bgame_uninstaller,
				   *bgame_icon;
	GtkWidget *egame_name,
			  *egame_location,
			  *egame_sp,
			  *egame_start_arg,
			  *egame_uninstaller,
			  *egame_icon;
	GtkWidget *lgame_name,
			  *lgame_location,
			  *lgame_sp,
			  *lgame_start_arg,
			  *lgame_uninstaller,
			  *lgame_icon;
} static new_game;

struct error_state_t {
	unsigned name: 1;
	unsigned sp: 1;
	unsigned loc: 1;
	unsigned uninstaller: 1;
};

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
	gtk_box_prepend(gapp.game_list_wrapper, GTK_WIDGET(bar));
	grid = gtk_grid_new();
	message_label = gtk_label_new("");
	gtk_info_bar_add_child(bar, message_label);
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
add_new_game_try_find(GtkWidget *widget, enum ADD_NEW_GAME_TRY_FIND point)
{
	game_t *gr = NULL;
	const char *loc = gtk_entry_buffer_get_text(new_game.bgame_location);

	gtk_button_set_child(GTK_BUTTON(widget), gtk_spinner_new());

	if (gtk_entry_buffer_get_length(new_game.bgame_location) > 1)
	{
		if (isExist(loc))
		{
			gr = scan_game_dir("", loc);
			if (gr) {
				if (point == ADD_NEW_GAME_TRY_FIND_SP) {
					if (gr->start_point) {
						gtk_entry_buffer_insert_text_after_delete(new_game.bgame_sp, 0, gr->start_point, strlen(gr->start_point));
					}
				} else if (point == ADD_NEW_GAME_TRY_FIND_UNINSTALLER) {
					if (gr->uninstaller) {
						gtk_entry_buffer_insert_text_after_delete(new_game.bgame_uninstaller, 0, gr->uninstaller, strlen(gr->uninstaller));
					}
				} else if (point == ADD_NEW_GAME_TRY_FIND_ICON) {
					if (gr->icon) {
						gtk_entry_buffer_insert_text_after_delete(new_game.bgame_icon, 0, gr->icon, strlen(gr->icon));
					}
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
			quick_message(gapp.window, msg);
		}
	}
	else
	{
		quick_message(gapp.window, "First fill in field \"Game root directory\"");
	}

	gtk_button_set_label(GTK_BUTTON(widget), "Try find"); //TODO
}

/* if do_set eq 1 set as error, if eq 0 reset */
static void
set_error_state(GtkWidget *widget, int do_set)
{
	char *error_class = "error";
	if (do_set == 1) {
		if (!gtk_widget_has_css_class(widget, error_class)) {
			gtk_widget_add_css_class(widget, error_class);
		}
	} else if (do_set == 0) {
		if (gtk_widget_has_css_class(widget, error_class)) {
			gtk_widget_remove_css_class(widget, error_class);
		}
	}
}

static gboolean
error_state_ok(struct error_state_t es)
{
	if (es.name == 1) {
		return 0;
	}
	if (es.loc == 1) {
		return 0;
	}
	if (es.sp == 1) {
		return 0;
	}
	if (es.uninstaller == 1) {
		return 0;
	}

	return 1;
}

gboolean
end_add_new_game_dialog(GtkWidget *dialog,
						gint respons_id,
						gpointer data)
{
	/* if var eq one - error, else - no */
	struct error_state_t error_state;
	game_t *gr;

	if (respons_id == GTK_RESPONSE_OK)
	{
		if (gtk_entry_buffer_get_length(new_game.bgame_name) < 1) {
			error_state.name = 1;
			set_error_state(new_game.egame_name, 1);
		} else {
			error_state.name = 0;
			set_error_state(new_game.egame_name, 0);
		}

		if (gtk_entry_buffer_get_length(new_game.bgame_sp) < 1) {
			error_state.sp = 1;
			set_error_state(new_game.egame_sp, 1);
		} else {
			if (!isExist(gtk_entry_buffer_get_text(new_game.bgame_sp))) {
				error_state.sp = 1;
				set_error_state(new_game.egame_sp, 1);
			} else {
				error_state.sp = 0;
				set_error_state(new_game.egame_sp, 0);
			}
		}

		if (gtk_entry_buffer_get_length(new_game.bgame_location) < 1) {
			error_state.loc = 1;
			set_error_state(new_game.egame_location, 1);
		} else {
			if (!isExist(gtk_entry_buffer_get_text(new_game.bgame_location))) {
				error_state.loc = 1;
				set_error_state(new_game.egame_location, 1);
			} else {
				error_state.loc = 0;
				set_error_state(new_game.egame_location, 0);
			}
		}

		if (gtk_entry_buffer_get_length(new_game.bgame_uninstaller) < 1) {
			error_state.uninstaller = 1;
			set_error_state(new_game.egame_uninstaller, 1);
		} else {
			if (!isExist(gtk_entry_buffer_get_text(new_game.bgame_uninstaller))) {
				error_state.uninstaller = 1;
				set_error_state(new_game.egame_uninstaller, 1);
			} else {
				error_state.uninstaller = 0;
				set_error_state(new_game.egame_uninstaller, 0);
			}
		}

		if (gtk_entry_buffer_get_length(new_game.bgame_icon) > 0) {
			if (!isExist(gtk_entry_buffer_get_text(new_game.bgame_icon))) {
				set_error_state(new_game.egame_icon, 1);
			} else {
				set_error_state(new_game.egame_icon, 0);
			}
		}

		if (error_state_ok(error_state) == 1) {
			gr = gr_init(gtk_entry_buffer_get_text(new_game.bgame_name),
				gtk_entry_buffer_get_text(new_game.bgame_location),
				gtk_entry_buffer_get_text(new_game.bgame_sp),
				gtk_entry_buffer_get_text(new_game.bgame_uninstaller),
				gtk_entry_buffer_get_text(new_game.bgame_icon));

			if (gr) {
				gr->start_argv = (char*)gtk_entry_buffer_get_text(new_game.bgame_start_arg);
				add_new_game_short(gr);
				gr_save(gr);
				gtk_window_destroy(GTK_WINDOW(dialog));
			} else {
				quick_message(GTK_WINDOW(dialog), "Cant initialize game!");
			}
		}
	} else if (respons_id == GTK_RESPONSE_CANCEL) {
		gtk_window_destroy(GTK_WINDOW(dialog));
	} else {
		quick_message(GTK_WINDOW(dialog), "Some error when adding new game!");
		gtk_window_destroy(GTK_WINDOW(dialog));
	}

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
		gtk_entry_buffer_insert_text_after_delete(new_game.bgame_location, 0, loc, strlen(loc));
		gtk_entry_buffer_insert_text_after_delete(new_game.bgame_sp, 0, loc, strlen(loc));
		gtk_entry_buffer_insert_text_after_delete(new_game.bgame_uninstaller, 0, loc, strlen(loc));
		gtk_entry_buffer_insert_text_after_delete(new_game.bgame_icon, 0, loc, strlen(loc));
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
			  *game_box,
			  *game_frame,
			  *game_grid,
			  *picker,
			  *bgame_sp_finder,
			  *bgame_uninstaller_finder,
			  *bgame_icon_finder;

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

	new_game.bgame_name			= gtk_entry_buffer_new("", 0);
	new_game.bgame_location		= gtk_entry_buffer_new("", 0);
	new_game.bgame_sp			= gtk_entry_buffer_new("", 0);
	new_game.bgame_uninstaller	= gtk_entry_buffer_new("", 0);
	new_game.bgame_start_arg	= gtk_entry_buffer_new("", 0);
	new_game.bgame_icon			= gtk_entry_buffer_new("", 0);

	new_game.lgame_name			= gtk_label_new_with_mnemonic("_Name");
	new_game.lgame_location		= gtk_label_new_with_mnemonic("_Game root director");
	new_game.lgame_sp			= gtk_label_new_with_mnemonic("_Start point");
	new_game.lgame_start_arg	= gtk_label_new_with_mnemonic("_Start arguments");
	new_game.lgame_uninstaller	= gtk_label_new_with_mnemonic("_Uninstaller");
	new_game.lgame_icon 		= gtk_label_new_with_mnemonic("_Icon");
	gtk_label_set_xalign(GTK_LABEL(new_game.lgame_name), 0);
	gtk_label_set_xalign(GTK_LABEL(new_game.lgame_sp), 0);
	gtk_label_set_xalign(GTK_LABEL(new_game.lgame_location), 0);
	gtk_label_set_xalign(GTK_LABEL(new_game.lgame_start_arg), 0);
	gtk_label_set_xalign(GTK_LABEL(new_game.lgame_uninstaller), 0);
	gtk_label_set_xalign(GTK_LABEL(new_game.lgame_icon), 0);

	new_game.egame_name = gtk_entry_new_with_buffer(new_game.bgame_name);
	gtk_entry_set_placeholder_text(GTK_ENTRY(new_game.egame_name), "Name");
	gtk_label_set_mnemonic_widget(GTK_LABEL(new_game.lgame_name), new_game.egame_name);

	new_game.egame_location = gtk_entry_new_with_buffer(new_game.bgame_location);
	gtk_entry_set_placeholder_text(GTK_ENTRY(new_game.egame_location), "Location");
	gtk_label_set_mnemonic_widget(GTK_LABEL(new_game.lgame_location), new_game.egame_location);

	new_game.egame_sp = gtk_entry_new_with_buffer(new_game.bgame_sp);
	gtk_entry_set_placeholder_text(GTK_ENTRY(new_game.egame_sp), "Start point");
	gtk_label_set_mnemonic_widget(GTK_LABEL(new_game.lgame_sp), new_game.egame_sp);

	new_game.egame_uninstaller = gtk_entry_new_with_buffer(new_game.bgame_uninstaller);
	gtk_entry_set_placeholder_text(GTK_ENTRY(new_game.egame_uninstaller), "Uninstaller");
	gtk_label_set_mnemonic_widget(GTK_LABEL(new_game.lgame_uninstaller), new_game.egame_uninstaller);

	new_game.egame_start_arg = gtk_entry_new_with_buffer(new_game.bgame_start_arg);
	gtk_entry_set_placeholder_text(GTK_ENTRY(new_game.egame_start_arg), "Start arguments");
	gtk_label_set_mnemonic_widget(GTK_LABEL(new_game.lgame_start_arg), new_game.egame_start_arg);

	new_game.egame_icon = gtk_entry_new_with_buffer(new_game.bgame_icon);
	gtk_entry_set_placeholder_text(GTK_ENTRY(new_game.egame_icon), "Icon");
	gtk_label_set_mnemonic_widget(GTK_LABEL(new_game.lgame_icon), new_game.egame_icon);

	g_signal_connect(G_OBJECT(new_game.egame_name), "changed", G_CALLBACK(set_error_state), 0);
	g_signal_connect(G_OBJECT(new_game.egame_location), "changed", G_CALLBACK(set_error_state), 0);
	g_signal_connect(G_OBJECT(new_game.egame_sp), "changed", G_CALLBACK(set_error_state), 0);
	g_signal_connect(G_OBJECT(new_game.egame_start_arg), "changed", G_CALLBACK(set_error_state), 0);
	g_signal_connect(G_OBJECT(new_game.egame_uninstaller), "changed", G_CALLBACK(set_error_state), 0);
	g_signal_connect(G_OBJECT(new_game.egame_icon), "changed", G_CALLBACK(set_error_state), 0);

	bgame_sp_finder				= gtk_button_new_with_label("Try find");
	bgame_uninstaller_finder	= gtk_button_new_with_label("Try find");
	bgame_icon_finder			= gtk_button_new_with_label("Try find");
	g_signal_connect(G_OBJECT(bgame_sp_finder), "clicked",
			G_CALLBACK(add_new_game_try_find), (gpointer*)ADD_NEW_GAME_TRY_FIND_SP);
	g_signal_connect(G_OBJECT(bgame_uninstaller_finder), "clicked",
			G_CALLBACK(add_new_game_try_find), (gpointer*)ADD_NEW_GAME_TRY_FIND_UNINSTALLER);
	g_signal_connect(G_OBJECT(bgame_icon_finder), "clicked",
			G_CALLBACK(add_new_game_try_find), (gpointer*)ADD_NEW_GAME_TRY_FIND_ICON);

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

	gtk_grid_attach(GTK_GRID(game_grid), new_game.lgame_name,		0, 0, 1, 1);
	gtk_grid_attach(GTK_GRID(game_grid), new_game.egame_name,		1, 0, 2, 1);

	gtk_grid_attach(GTK_GRID(game_grid), new_game.lgame_location,	0, 1, 1, 1);
	gtk_grid_attach(GTK_GRID(game_grid), new_game.egame_location,	1, 1, 1, 1);
	gtk_grid_attach(GTK_GRID(game_grid), picker,					2, 1, 1, 1);

	gtk_grid_attach(GTK_GRID(game_grid), new_game.lgame_sp,			0, 2, 1, 1);
	gtk_grid_attach(GTK_GRID(game_grid), new_game.egame_sp,			1, 2, 1, 1);
	gtk_grid_attach(GTK_GRID(game_grid), bgame_sp_finder,			2, 2, 1, 1);

	gtk_grid_attach(GTK_GRID(game_grid), new_game.lgame_start_arg,	0, 3, 1, 1);
	gtk_grid_attach(GTK_GRID(game_grid), new_game.egame_start_arg,	1, 3, 2, 1);

	gtk_grid_attach(GTK_GRID(game_grid), new_game.lgame_uninstaller,	0, 4, 1, 1);
	gtk_grid_attach(GTK_GRID(game_grid), new_game.egame_uninstaller,	1, 4, 1, 1);
	gtk_grid_attach(GTK_GRID(game_grid), bgame_uninstaller_finder,		2, 4, 1, 1);

	gtk_grid_attach(GTK_GRID(game_grid), new_game.lgame_icon,			0, 5, 1, 1);
	gtk_grid_attach(GTK_GRID(game_grid), new_game.egame_icon,			1, 5, 1, 1);
	gtk_grid_attach(GTK_GRID(game_grid), bgame_icon_finder,				2, 5, 1, 1);

	g_signal_connect_swapped(dialog,
							"response",
							G_CALLBACK(end_add_new_game_dialog),
							dialog);

	gtk_widget_show(dialog);

}
