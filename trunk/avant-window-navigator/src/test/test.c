/*
 *  Copyright (C) 2007 Neil Jagdish Patel <njpatel@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA.
 *
 *  Author : Neil Jagdish Patel <njpatel@gmail.com>
 *
 *  Notes : Thanks to MacSlow (macslow.thepimp.net) for the transparent & shaped
 *	    window code.
*/

#include "awn-bar.h"

G_DEFINE_TYPE (AwnBar, awn_bar, GTK_TYPE_WINDOW)

#define AWN_BAR_DEFAULT_WIDTH		300
#define AWN_BAR_DEFAULT_HEIGHT		100


static void awn_bar_destroy (GtkObject *object);
static void _on_alpha_screen_changed (GtkWidget* pWidget, GdkScreen* pOldScreen, GtkWidget* pLabel);
static gboolean _on_expose (GtkWidget *widget, GdkEventExpose *expose);
static void _update_input_shape (GtkWidget* window, int width, int height);
static gboolean  _on_configure (GtkWidget* pWidget, GdkEventConfigure* pEvent, gpointer userData);
static void _position_window (GtkWidget *window);


static void
awn_bar_class_init( AwnBarClass *this_class )
{
        GObjectClass *g_obj_class;
        GtkObjectClass *object_class;
        GtkWidgetClass *widget_class;
        
        g_obj_class = G_OBJECT_CLASS( this_class );
        object_class = (GtkObjectClass*) this_class;
        widget_class = GTK_WIDGET_CLASS( this_class );
        
        object_class->destroy =awn_bar_destroy;
        parent_class = gtk_type_class (gtk_widget_get_type ());
        
}

static void
awn_bar_init( AwnBar *bar )
{

}

GtkWidget *
awn_bar_new( void )
{
        AwnBar *this = g_object_new(AWN_BAR_TYPE, 
        			    "type", GTK_WINDOW_TOPLEVEL,
        			    "type-hint", GDK_WINDOW_TYPE_HINT_DOCK,
        			    NULL);
        _on_alpha_screen_changed (GTK_WIDGET(this), NULL, NULL);
        gtk_widget_set_app_paintable (GTK_WIDGET(this), TRUE);
        
        gtk_window_resize (GTK_WINDOW(this), AWN_BAR_DEFAULT_WIDTH, AWN_BAR_DEFAULT_HEIGHT);
        
        g_signal_connect (G_OBJECT (this),"destroy",
			  G_CALLBACK (gtk_main_quit), NULL);
	
	g_signal_connect (G_OBJECT (this), "expose-event",
			  G_CALLBACK (_on_expose), NULL);
	
	
	g_signal_connect (G_OBJECT (this), "configure-event",
			  G_CALLBACK (_on_configure), NULL);       
        
        _update_input_shape (GTK_WIDGET(this), AWN_BAR_DEFAULT_WIDTH, AWN_BAR_DEFAULT_HEIGHT);
        
               
        
        return GTK_WIDGET(this);
}

static void   
awn_bar_destroy       (GtkObject   *object)
{
  g_print("Destroyed\n");
  g_return_if_fail(object != NULL);
  g_return_if_fail(IS_AWN_BAR(object));

  AwnBar *bar = AWN_BAR(object);

  /* Chain up */
  if (GTK_OBJECT_CLASS(parent_class)->destroy)
    (* GTK_OBJECT_CLASS(parent_class)->destroy) (object);
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

static void 
render (cairo_t *cr, gint width, gint height)
{
	cairo_set_source_rgba (cr, 1.0f, 1.0f, 1.0f, 0.0f);
	cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
	cairo_paint (cr);
	
	cairo_pattern_t *pat;
	
	cairo_set_line_width(cr, 1.0);
	
	pat = cairo_pattern_create_linear (0.0, 0.0, 0.0, height);
	cairo_pattern_add_color_stop_rgba (pat, 0.5, 1.0, 1.0, 1.0, 0.3);
	cairo_pattern_add_color_stop_rgba (pat, 1, 0.0, 0.0, 0.0, 0.3);
	cairo_rectangle (cr, 0, height/2, width, height/2);
	cairo_set_source(cr, pat);
	cairo_fill(cr);
	cairo_pattern_destroy(pat);
	
	cairo_set_operator (cr, CAIRO_OPERATOR_OVER);
	
	
	pat = cairo_pattern_create_linear (0.0, 0.0, 0.0, height);
	cairo_pattern_add_color_stop_rgba (pat, 0.5, 1, 1, 1, 0.25);
	cairo_pattern_add_color_stop_rgba (pat, 1, 0.7, 0.7, 0.7, 0.05);
	cairo_rectangle (cr, 1, height/2, width-2, height/5);
	cairo_set_source(cr, pat);
	cairo_fill(cr);
	cairo_pattern_destroy(pat);
	
	cairo_set_source_rgba (cr, 1.0f, 1.0f, 1.0f, 0.2f);
	cairo_rectangle (cr, 1.5, (height/2)+1.5, width-2.5, (height/2)-2);
	cairo_stroke(cr);
	
	cairo_set_source_rgba (cr, 0.0f, 0.0f, 0.0f, 1.0);
	cairo_rectangle (cr, 0.5, (height/2)+0.5, width-0.5, (height/2));
	cairo_stroke(cr);
	
	GdkPixbuf *pixbuf = NULL;
	GError *err = NULL;
	pixbuf = gdk_pixbuf_new_from_file("browser.png", &err);
	gdk_cairo_set_source_pixbuf (cr, pixbuf, 50.0, 50.0);
	//cairo_paint(cr);
	cairo_paint_with_alpha(cr, 0.5);
}

static void 
render_pixmap (cairo_t *cr, gint width, gint height)
{
	
	cairo_scale (cr, (double) width, (double) height);
	cairo_set_source_rgba (cr, 1.0f, 1.0f, 1.0f, 0.0f);
	cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
	cairo_paint (cr);
	
	cairo_pattern_t *pat;
	
	cairo_set_line_width(cr, 0.05);
	pat = cairo_pattern_create_linear (0.0, 0.0, 0.0, 1.0);
	cairo_pattern_add_color_stop_rgba (pat, 0.5, 1.0, 1.0, 1.0, 1);
	cairo_pattern_add_color_stop_rgba (pat, 1, 0.8, 0.8, 0.8, 1);
	cairo_rectangle (cr, 0, 0.5, 1, 1);
	cairo_set_source(cr, pat);
	cairo_fill(cr);
	cairo_pattern_destroy(pat);
	
	
}

static void 
_update_input_shape (GtkWidget* window, int width, int height)
{
	static GdkBitmap* pShapeBitmap = NULL;
	static cairo_t* pCairoContext = NULL;

	pShapeBitmap = (GdkBitmap*) gdk_pixmap_new (NULL, width, height, 1);
	if (pShapeBitmap)
	{
		pCairoContext = gdk_cairo_create (pShapeBitmap);
		if (cairo_status (pCairoContext) == CAIRO_STATUS_SUCCESS)
		{
			render_pixmap (pCairoContext, width, height);
			cairo_destroy (pCairoContext);

			gtk_widget_input_shape_combine_mask (window, NULL, 0, 0);
			gtk_widget_input_shape_combine_mask (window, pShapeBitmap, 0, 0);
		}
		g_object_unref ((gpointer) pShapeBitmap);
	}
}

static gboolean 
_on_expose (GtkWidget *widget, GdkEventExpose *expose)
{
	static oWidth;
	static oHeight;
	gint width;
	gint height;
	cairo_t *cr = NULL;

	cr = gdk_cairo_create (widget->window);
	if (!cr)
		return FALSE;

	gtk_window_get_size (GTK_WINDOW (widget), &width, &height);
	render (cr, width, height);
	cairo_destroy (cr);
	
	if ( oWidth != width || oHeight != height)
		_position_window(GTK_WIDGET(widget));
	oWidth = width;
	oHeight = height;
	return FALSE;
}

static gboolean 
_on_configure (GtkWidget* pWidget, GdkEventConfigure* pEvent, gpointer userData)
{
	gint iNewWidth = pEvent->width;
	gint iNewHeight = pEvent->height;

	if (1)
	{
		_update_input_shape (pWidget, iNewWidth, iNewHeight);

	}

	return FALSE;
}

static void
_position_window (GtkWidget *window)
{
	GdkScreen *screen;
	gint ww, wh;
	gint sw, sh;
	gint x, y;
	
	gtk_window_get_size(GTK_WINDOW(window), &ww, &wh);
	screen = gtk_window_get_screen(GTK_WINDOW(window));
	sw = gdk_screen_get_width(screen);
	sh = gdk_screen_get_height(screen);
	
	x = (int) ((sw - ww) / 2);
	y = (int) (sh-wh);
	
	gtk_window_move(GTK_WINDOW(window), x, y);
}


void 
awn_bar_resize(AwnBar *bar)
{
	gtk_window_resize(GTK_WINDOW(bar), 1, 100);
}
















