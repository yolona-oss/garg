#include "run.h"
#include "../utils/eprintf.h"

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
