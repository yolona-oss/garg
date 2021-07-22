#include <gtk-3.0/gtk/gtk.h>
#include <unistd.h>
#include <stdlib.h>

int
init_gui(void)
{
	GtkWidget *window;

	int argc = 1;
	gtk_init(&argc, NULL);
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_widget_show(window);
	gtk_main();

	return 0;
}
