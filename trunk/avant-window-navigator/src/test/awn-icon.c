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
 *  Notes : Main Icon widget for each window on the bar
*/

#include <gtk/gtk.h>
#include <math.h>
#include <time.h>

#include "clock.h"
#include "clock-marshallers.h"

#define AWN_ICON_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), AWN_TYPE_ICON, AwnIconPrivate))

G_DEFINE_TYPE (AwnIcon, awn_icon, GTK_TYPE_DRAWING_AREA);

static gboolean awn_icon_expose (GtkWidget *clock, GdkEventExpose *event);
static gboolean awn_icon_button_press (GtkWidget *clock,
					     GdkEventButton *event);


typedef struct _AwnIconPrivate AwnIconPrivate;

struct _AwnIconPrivate
{
	GdkPixbuf *icon;
	GdkPixbuf *wnck_icon;
	GdkPixbuf *user_icon;
};



static void
awn_icon_class_init (AwnIconClass *class)
{
	GObjectClass *obj_class;
	GtkWidgetClass *widget_class;

	obj_class = G_OBJECT_CLASS (class);
	widget_class = GTK_WIDGET_CLASS (class);

	/* GtkWidget signals */
	widget_class->expose_event = awn_icon_expose;
	widget_class->button_press_event = awn_icon_button_press;

	g_type_class_add_private (obj_class, sizeof (AwnIconPrivate));
}

static void
awn_icon_init (AwnIcon *clock)
{
	gtk_widget_add_events (GTK_WIDGET (clock),
			GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
			GDK_POINTER_MOTION_MASK);

}

static void
draw (GtkWidget *clock, cairo_t *cr)
{
	AwnIconPrivate *priv;
	
	priv = AWN_ICON_GET_PRIVATE (clock);
	
	
}

static gboolean
awn_icon_expose (GtkWidget *clock, GdkEventExpose *event)
{
	cairo_t *cr;

	/* get a cairo_t */
	cr = gdk_cairo_create (clock->window);

	cairo_rectangle (cr,
			event->area.x, event->area.y,
			event->area.width, event->area.height);
	cairo_clip (cr);
	
	draw (clock, cr);

	cairo_destroy (cr);

	return FALSE;
}

static gboolean
awn_icon_button_press (GtkWidget *clock, GdkEventButton *event)
{
	AwnIconPrivate *priv;
	int minutes;
	double lx, ly;
	double px, py;
	double u, d2;
	
	priv = AWN_ICON_GET_PRIVATE (clock);

	minutes = priv->time.tm_min + priv->minute_offset;

	/* From
	 * http://mathworld.wolfram.com/Point-LineDistance2-Dimensional.html
	 */
	px = event->x - clock->allocation.width / 2;
	py = clock->allocation.height / 2 - event->y;
	lx = sin (M_PI / 30 * minutes); ly = cos (M_PI / 30 * minutes);
	u = lx * px + ly * py;

	/* on opposite side of origin */
	if (u < 0) return FALSE;

	d2 = pow (px - u * lx, 2) + pow (py - u * ly, 2);

	if (d2 < 25) /* 5 pixels away from the line */
	{
		priv->dragging = TRUE;
		g_print ("got minute hand\n");
	}
	
	return FALSE;
}

GtkWidget *
awn_icon_new (void)
{
	return g_object_new (AWN_TYPE_ICON, NULL);
}
