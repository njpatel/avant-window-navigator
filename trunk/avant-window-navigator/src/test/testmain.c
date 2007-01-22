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
*/

#include <gtk/gtk.h>

#include "awn-bar.h"

#define WNCK_I_KNOW_THIS_IS_UNSTABLE 1
#include <libwnck/libwnck.h>

static void 
render (cairo_t *cr, gint width, gint height)
{
	cairo_set_source_rgba (cr, 1.0f, 1.0f, 1.0f, 0.0f);
	cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
	cairo_paint (cr);
	
	
	cairo_set_operator (cr, CAIRO_OPERATOR_OVER);
	
	GdkPixbuf *pixbuf = NULL;
	GError *err = NULL;
	pixbuf = gdk_pixbuf_new_from_file("browser.png", &err);
	gdk_cairo_set_source_pixbuf (cr, pixbuf, 0, 50);
	//cairo_paint(cr);
	cairo_paint_with_alpha(cr, 0.5);
}


static gboolean
expose (GtkWidget *widget, GdkEventExpose *expose)
{
	static oWidth;
	static oHeight;
	gint width;
	gint height;
	cairo_t *cr = NULL;

	cr = gdk_cairo_create (widget->window);
	if (!cr)
		return FALSE;

	render (cr, 60, 100);
	cairo_destroy (cr);
	
	return FALSE;
}

int 
main (int argc, char** argv)
{
	
	GtkWidget *bar = NULL;
	GtkWidget *box = NULL;
	GtkWidget *winman = NULL;
	GtkWidget *lab = NULL;
	
	gtk_init (&argc, &argv);
	
	bar = awn_bar_new ();
	//gtk_window_set_policy (GTK_WINDOW (bar), FALSE, FALSE, TRUE);
	
	box = gtk_hbox_new(FALSE, 0);
	GdkWindow *eb = gdk_window_new(NULL, );

	
	g_signal_connect(G_OBJECT(eb), "expose-event", G_CALLBACK(expose), NULL);
	
	gtk_widget_show_all(bar);
	
	gtk_main ();
}


