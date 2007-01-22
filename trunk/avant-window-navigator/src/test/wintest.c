#include <gtk/gtk.h>

gboolean
timeout (GtkWidget *win ) 
{
	gtk_window_set_urgency_hint(GTK_WINDOW(win), TRUE);
}

int
main (int argc, char** argv)
{
	gtk_init(&argc, &argv);
	
	GtkWidget *win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	GtkWidget *lab = gtk_label_new("Hello");
	
	gtk_container_add(GTK_CONTAINER(win), lab);
	
	gtk_widget_show_all(win);

	g_timeout_add(2000, timeout, (gpointer)win);
	
	gtk_main();
}
