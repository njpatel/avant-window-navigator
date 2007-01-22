

#include <gtk/gtk.h>

#define WNCK_I_KNOW_THIS_IS_UNSTABLE 1
#include <libwnck/libwnck.h>

static GList *windows = NULL;	
static GtkWidget *window = NULL;

static void
window_paint( WnckWindow *win, cairo_t *cr)
{
	if (wnck_window_is_skip_tasklist(win) )
		return;
	static double w = 0;
	cairo_move_to(cr, w, 0);
	
	
	GdkPixbuf *pixbuf = gdk_pixbuf_copy(wnck_window_get_icon(win));
	gdk_cairo_set_source_pixbuf(cr, pixbuf, w, 0);
	cairo_paint(cr);
	
	g_print("%s\n", wnck_window_get_name(win));
	w+=60.0;
}

static void 
render (cairo_t *cr, gint width, gint height)
{
	g_print("render\n");
	cairo_set_source_rgba (cr, 1.0f, 1.0f, 1.0f, 0.0f);
	cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
	cairo_paint (cr);
	
	cairo_set_operator (cr, CAIRO_OPERATOR_OVER);
	
	cairo_set_line_width(cr, 1.0);
	
	WnckScreen *screen = wnck_screen_get_default();
	
	g_list_foreach(wnck_screen_get_windows(screen), (GFunc)window_paint, (gpointer)cr);
	
	
}

static gboolean
win_expose(GtkWidget *widget, GdkEventExpose *expose)
{
	gint width;
	gint height;
	cairo_t *cr = NULL;

	cr = gdk_cairo_create (widget->window);
	if (!cr) {
		g_print("failed\n");
		return FALSE;
	}
	gtk_window_get_size (GTK_WINDOW (widget), &width, &height);
	render (cr, width, height);
	cairo_destroy (cr);
	

	return FALSE;
}

static void 
_on_alpha_screen_changed (GtkWidget* pWidget, GdkScreen* pOldScreen, GtkWidget* pLabel)
{                       
	GdkScreen* pScreen = gtk_widget_get_screen (pWidget);
	GdkColormap* pColormap = gdk_screen_get_rgba_colormap (pScreen);
      
	if (!pColormap)
		pColormap = gdk_screen_get_rgb_colormap (pScreen);

	gtk_widget_set_colormap (pWidget, pColormap);
}

int 
main (int argc, char** argv)
{
	gtk_init (&argc, &argv);
	
	/* wnck */
	WnckScreen *screen = wnck_screen_get_default();
	windows = wnck_screen_get_windows(screen);
	
	/* window */
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect (G_OBJECT(window), "expose-event",
			  G_CALLBACK(win_expose), (gpointer)windows);
	
	_on_alpha_screen_changed (GTK_WIDGET(window), NULL, NULL);
	gtk_widget_set_app_paintable (GTK_WIDGET(window), TRUE);
	gtk_window_set_type_hint(GTK_WINDOW(window), GDK_WINDOW_TYPE_HINT_DOCK);
	gtk_window_resize(GTK_WINDOW(window), 1280, 50);
	gtk_window_move(GTK_WINDOW(window), 0, 974);
	
	gtk_widget_show_all(window);
	
	gtk_main();
}
