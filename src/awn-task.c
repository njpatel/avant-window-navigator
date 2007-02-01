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

#include "awn-task.h"

#define AWN_TASK_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), AWN_TYPE_TASK, AwnTaskPrivate))

G_DEFINE_TYPE (AwnTask, awn_task, GTK_TYPE_DRAWING_AREA);

#define  AWN_FRAME_RATE	15

#define  AWN_TASK_EFFECT_DIR_DOWN		0
#define  AWN_TASK_EFFECT_DIR_UP			1
#define  AWN_TASK_EFFECT_DIR_LEFT		2
#define  AWN_TASK_EFFECT_DIR_RIGHT		3


/* FORWARD DECLERATIONS */

static gboolean awn_task_expose (GtkWidget *task, GdkEventExpose *event);
static gboolean awn_task_button_press (GtkWidget *task, GdkEventButton *event);
static gboolean awn_task_proximity_in (GtkWidget *task, GdkEventCrossing *event);
static gboolean awn_task_proximity_out (GtkWidget *task, GdkEventCrossing *event);
static gboolean awn_task_drag_motion (GtkWidget *task, 
		GdkDragContext *context, gint x, gint y, guint t);


/* STRUCTS & ENUMS */

typedef enum {
	AWN_TASK_EFFECT_NONE,
	AWN_TASK_EFFECT_OPENING,
	AWN_TASK_EFFECT_HOVER,
	AWN_TASK_EFFECT_ATTENTION,
	AWN_TASK_EFFECT_CLOSING

} AwnTaskEffect;

typedef struct _AwnTaskPrivate AwnTaskPrivate;

struct _AwnTaskPrivate
{
	AwnSettings *settings;
	
	gboolean is_launcher;
	AwnSmartLauncher launcher;
	gboolean is_starter;

	WnckWindow *window;
	AwnTitle *title;
	
	gboolean is_active;
	gboolean needs_attention;
	gboolean is_closing;
	
	GdkPixbuf *icon;
	gint icon_width;
	gint icon_height;
	
	/* EFFECT VARIABLES */
	gboolean effect_lock;
	AwnTaskEffect current_effect;
	gint effect_direction;
	
	gint x_offset;
	gint y_offset;
	gint width;
	gint height;
	gfloat rotate_degrees;
	gfloat alpha;
};

/* GLOBALS */

static const GtkTargetEntry drop_types[] = {
	{ "text/uri-list", 0, 0 }
};
static const gint n_drop_types = G_N_ELEMENTS (drop_types);

static void
awn_task_class_init (AwnTaskClass *class)
{
	GObjectClass *obj_class;
	GtkWidgetClass *widget_class;

	obj_class = G_OBJECT_CLASS (class);
	widget_class = GTK_WIDGET_CLASS (class);

	/* GtkWidget signals */
	widget_class->expose_event = awn_task_expose;
	widget_class->button_press_event = awn_task_button_press;
	widget_class->enter_notify_event = awn_task_proximity_in;
	widget_class->leave_notify_event = awn_task_proximity_out;
	//widget_class->drag_motion_event = awn_task_drag_motion;

	g_type_class_add_private (obj_class, sizeof (AwnTaskPrivate));
}

static void
awn_task_init (AwnTask *task)
{
	gtk_widget_add_events (GTK_WIDGET (task),
			GDK_BUTTON_PRESS_MASK | GDK_ENTER_NOTIFY_MASK |
			GDK_LEAVE_NOTIFY_MASK | GDK_POINTER_MOTION_MASK);
	
	gtk_widget_set_size_request(GTK_WIDGET(task), 60, 100);
	
	gtk_drag_dest_set (GTK_WIDGET (task),
                           GTK_DEST_DEFAULT_ALL,
                           drop_types, n_drop_types,
                           GDK_ACTION_MOVE);
	
	
#if !GTK_CHECK_VERSION(2,9,0)
			/* TODO: equivalent of following function: */
#else
			gtk_drag_dest_set_track_motion  (GTK_WIDGET (task),TRUE);
#endif

	/* set all priv variables */
	AwnTaskPrivate *priv;
	priv = AWN_TASK_GET_PRIVATE (task);
	
	priv->is_launcher = FALSE;
	priv->is_starter = FALSE;
	priv->is_closing = FALSE;
	priv->window = NULL;
	priv->title = NULL;
	priv->icon = NULL;
	priv->is_active = FALSE;
	priv->needs_attention = FALSE;
	priv->effect_lock = FALSE;
	priv->effect_direction = AWN_TASK_EFFECT_DIR_UP;
	priv->current_effect = AWN_TASK_EFFECT_NONE;
	priv->x_offset = 0;
	priv->y_offset = 0;
	priv->width = 0;
	priv->height = 0;
	priv->rotate_degrees = 0.0;
	priv->alpha = 1.0;
}


/************* EFFECTS *****************/

static gboolean
_task_opening_effect (AwnTask *task)
{
	AwnTaskPrivate *priv;
	priv = AWN_TASK_GET_PRIVATE (task);
	static gint max = 10;	

	if (priv->effect_lock) {
		if ( priv->current_effect != AWN_TASK_EFFECT_OPENING)
			return TRUE;
	} else {
		priv->effect_lock = TRUE;
		priv->current_effect = AWN_TASK_EFFECT_OPENING;
		priv->effect_direction = AWN_TASK_EFFECT_DIR_UP;
		priv->y_offset = -48;
	}
	
	if (priv->effect_direction) {
		priv->y_offset +=1;
		
		if (priv->y_offset >= max) 
			priv->effect_direction = AWN_TASK_EFFECT_DIR_DOWN;	
	
	} else {
		priv->y_offset-=1;
		
		if (priv->y_offset < 1) {
			/* finished bouncing, back to normal */
			priv->effect_lock = FALSE;
			priv->current_effect = AWN_TASK_EFFECT_NONE;
			priv->effect_direction = AWN_TASK_EFFECT_DIR_UP;
			priv->y_offset = 0;
			
		}
	}
	
	gtk_widget_queue_draw(GTK_WIDGET(task));
	
	
	if (priv->effect_lock == FALSE)
		return FALSE;
	
	return TRUE;
}


static void
launch_opening_effect (AwnTask *task )
{
	g_print("Opening effect\n");
	g_timeout_add(AWN_FRAME_RATE, (GSourceFunc)_task_opening_effect, (gpointer)task);
}





/******************************CALLBACKS**********************/

static void
draw (GtkWidget *task, cairo_t *cr)
{
	AwnTaskPrivate *priv;
	double x, y;
	double width, height;
	priv = AWN_TASK_GET_PRIVATE (task);
	
	width = task->allocation.width;
	height = task->allocation.height;
	
	/* task back */
	cairo_set_source_rgba (cr, 1, 0, 0, 0.0);
	cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
	cairo_paint (cr);
	
	/* active */
	if (priv->is_active) {
		cairo_set_operator (cr, CAIRO_OPERATOR_OVER);
		cairo_set_source_rgba(cr, 1, 1, 1, 0.2);
		cairo_rectangle(cr, 0, 51, 60, 48);
		cairo_fill(cr);
	}
	
	/* content */
	if (priv->icon) {
		double x1, y1;
		
		x1 = (width-priv->icon_width)/2;
		
		y1 = ((50-priv->icon_height)/2) + 50 + (-1 * priv->y_offset);
		
		gdk_cairo_set_source_pixbuf (cr, priv->icon, x1, y1);
		cairo_paint_with_alpha(cr, priv->alpha);
	}
}

static gboolean
awn_task_expose (GtkWidget *task, GdkEventExpose *event)
{
	cairo_t *cr;

	/* get a cairo_t */
	cr = gdk_cairo_create (task->window);

	draw (task, cr);

	cairo_destroy (cr);

	return FALSE;
}

static gboolean
awn_task_button_press (GtkWidget *task, GdkEventButton *event)
{
	AwnTaskPrivate *priv;
	GtkWidget *menu = NULL;
	
	priv = AWN_TASK_GET_PRIVATE (task);

	if (priv->window) {
	
		switch (event->button) {
			case 1:
				if ( wnck_window_is_active( priv->window ) ) {
					wnck_window_minimize( priv->window );
					return TRUE;
				}
				wnck_window_activate( priv->window, 
						gtk_get_current_event_time() );
				break;
		
			case 3:
				menu = wnck_create_window_action_menu(priv->window);
				gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, 
						       NULL, 3, event->time);
				break;
			default:
				return FALSE;
		}
	}
	 
	return FALSE;
}

static gboolean 
awn_task_proximity_in (GtkWidget *task, GdkEventCrossing *event)
{
	AwnTaskPrivate *priv;
	priv = AWN_TASK_GET_PRIVATE (task);
	
	if (priv->title) {
		gint i = (int)event->x_root;
		gint x, y, x1;
		gdk_window_get_origin(task->window, &x, &y);
		
		awn_title_show(AWN_TITLE (priv->title), awn_task_get_name(AWN_TASK(task)), x+30, 0);
	
	}
	return TRUE;
}

static gboolean 
awn_task_proximity_out (GtkWidget *task, GdkEventCrossing *event)
{
	AwnTaskPrivate *priv;
	priv = AWN_TASK_GET_PRIVATE (task);
	
	if (priv->title)
		awn_title_show(AWN_TITLE (priv->title), " ", 20, 0);
	
	return TRUE;
}

static gboolean 
awn_task_drag_motion (GtkWidget *task, 
		GdkDragContext *context, gint x, gint y, guint t)
{
	g_print("Drag Motion Event\n");
	return TRUE;
}

static void 
_task_wnck_icon_changed (WnckWindow *window, AwnTask *task)
{
        AwnTaskPrivate *priv;
	priv = AWN_TASK_GET_PRIVATE (task);
	
	if (priv->is_launcher)
		return;
        
        GdkPixbuf *old = NULL;
        old = priv->icon;
        priv->icon = gdk_pixbuf_copy (awn_x_get_icon (priv->window, 48, 48));
	priv->icon_width = gdk_pixbuf_get_width(priv->icon);
	priv->icon_height = gdk_pixbuf_get_height(priv->icon);
        gdk_pixbuf_unref(old);
        gtk_widget_queue_draw(GTK_WIDGET(task));
}

/**********************Gets & Sets **********************/

gboolean 
awn_task_get_is_launcher (AwnTask *task)
{
	AwnTaskPrivate *priv;
	priv = AWN_TASK_GET_PRIVATE (task);
	return priv->is_launcher;
}

gboolean 
awn_task_set_window (AwnTask *task, WnckWindow *window)
{
	g_return_val_if_fail(WNCK_IS_WINDOW(window), 0);
	
	AwnTaskPrivate *priv;
	priv = AWN_TASK_GET_PRIVATE (task);

	if (priv->window != NULL)
		return FALSE;
	
	priv->window = window;
	priv->icon = gdk_pixbuf_copy (awn_x_get_icon (priv->window, 48, 48) );
	priv->icon_width = gdk_pixbuf_get_width(priv->icon);
	priv->icon_height = gdk_pixbuf_get_height(priv->icon);
	
	g_signal_connect (G_OBJECT (priv->window), "icon_changed",
        		  G_CALLBACK (_task_wnck_icon_changed), (gpointer)task); 
        
        /* if launcher, set a launch_sequence
        else if starter, stop the launch_sequence, disable starter flag*/
        
        if (wnck_window_is_skip_tasklist(window))
        	return TRUE;
        	
        launch_opening_effect(task);
	return TRUE;
}

WnckWindow * 
awn_task_get_window (AwnTask *task)
{
	AwnTaskPrivate *priv;
	priv = AWN_TASK_GET_PRIVATE (task);
	return priv->window;
}

gulong 
awn_task_get_xid (AwnTask *task)
{
	AwnTaskPrivate *priv;
	priv = AWN_TASK_GET_PRIVATE (task);
	
	if (priv->window == NULL)
		return 0;
	
	g_return_val_if_fail(WNCK_IS_WINDOW(priv->window), 0);
	return wnck_window_get_xid(priv->window);
}

void 
awn_task_set_is_active (AwnTask *task, gboolean is_active)
{
	AwnTaskPrivate *priv;
	priv = AWN_TASK_GET_PRIVATE (task);
	
	if (is_active == priv->is_active)
		return;
	
	priv->is_active = is_active;
	gtk_widget_queue_draw(GTK_WIDGET(task));
}

void 
awn_task_set_needs_attention (AwnTask *task, gboolean needs_attention)
{
	AwnTaskPrivate *priv;
	priv = AWN_TASK_GET_PRIVATE (task);
	
	if (needs_attention == priv->needs_attention)
		return;
	
	priv->needs_attention = needs_attention;
	if (needs_attention)
		;
		//g_timeout_add(20, (GSourceFunc)_effect_needs_attention, (gpointer)task)
}	

const char* 
awn_task_get_name (AwnTask *task)
{
	AwnTaskPrivate *priv;
	priv = AWN_TASK_GET_PRIVATE (task);
	
	const char *name = NULL;
		
	/* if (priv->is_launcher)
		name = gnome_desktop_item_get_localestring (priv->item, 
						       GNOME_DESKTOP_ITEM_NAME);
	else */if (priv->window)
		name =  wnck_window_get_name(priv->window);
	else
		name =  "No name";
	return name;
}

void 
awn_task_set_title (AwnTask *task, AwnTitle *title)
{
	AwnTaskPrivate *priv;
	priv = AWN_TASK_GET_PRIVATE (task);
	
	priv->title = title;
}

AwnSettings* 
awn_task_get_settings (AwnTask *task)
{
	AwnTaskPrivate *priv;
	priv = AWN_TASK_GET_PRIVATE (task);
	return priv->settings;
}

/********************* awn_task_new * *******************/

GtkWidget *
awn_task_new (AwnSettings *settings)
{
	GtkWidget *task;
	AwnTaskPrivate *priv;
	task = g_object_new (AWN_TYPE_TASK, NULL);
	priv = AWN_TASK_GET_PRIVATE (task);
	
	priv->settings = settings;
	return task;
}

GtkWidget *
awn_task_new_from_window (AwnSettings *settings, WnckWindow *window)
{
	GtkWidget *task;
	AwnTaskPrivate *priv;
	
	g_return_val_if_fail(WNCK_IS_WINDOW(window), 0);
	
	task = awn_task_new(settings);
	priv = AWN_TASK_GET_PRIVATE (AWN_TASK(task));
	
	priv->window = window;
	priv->icon = gdk_pixbuf_copy (awn_x_get_icon (priv->window, 48, 48) );
	priv->icon_width = gdk_pixbuf_get_width(priv->icon);
	priv->icon_height = gdk_pixbuf_get_height(priv->icon);
		
	g_signal_connect (G_OBJECT (priv->window), "icon_changed",
        		  G_CALLBACK (_task_wnck_icon_changed), (gpointer)task);
}

GtkWidget *
awn_task_new_from_launcher(AwnSettings *settings, AwnSmartLauncher *launcher)
{

}

GtkWidget *awn_task_new_from_startup_notify(AwnSettings *settings)
{

}

static gboolean
_task_destroy (AwnTask *task)
{
	AwnTaskPrivate *priv;
	priv = AWN_TASK_GET_PRIVATE (task);
	const gint max = 48;	
	gfloat i;

	if (priv->effect_lock) {
		if ( priv->current_effect != AWN_TASK_EFFECT_CLOSING)
			return TRUE;
	} else {
		priv->effect_lock = TRUE;
		priv->current_effect = AWN_TASK_EFFECT_CLOSING;
		priv->effect_direction = AWN_TASK_EFFECT_DIR_UP;
		priv->y_offset = 0;
	}
	

		
	if (priv->y_offset >= max) {
		
		if (priv->width <=2) {
			priv->width -=2;
			gtk_widget_set_size_request(GTK_WIDGET(task), priv->width, 100);
			return TRUE;
		}
		
		priv->title = NULL;
		gdk_pixbuf_unref (priv->icon);
		
		gtk_widget_hide (GTK_WIDGET(task));
		gtk_object_destroy (GTK_OBJECT(task));
		
		task = NULL;
		return FALSE;
	
	} else {
		priv->y_offset +=1;
		i = (float) priv->y_offset / max;
		priv->alpha = 1.0 - i;
		priv->width = 60;
		gtk_widget_queue_draw(GTK_WIDGET(task));
	}
	return TRUE;
	
}

void 
awn_task_close (AwnTask *task)
{
	AwnTaskPrivate *priv;
	priv = AWN_TASK_GET_PRIVATE (task);
	
	priv->window = NULL;
	
	if (priv->is_launcher)
		return;

	/* start closing effect */
	priv->is_closing = TRUE;
	g_timeout_add(AWN_FRAME_RATE, (GSourceFunc)_task_destroy, (gpointer)task);
}






