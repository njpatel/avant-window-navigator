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
#include <string.h>
#include <libgnome/gnome-desktop-item.h>
#include <libgnomevfs/gnome-vfs.h>

#include "awn-task.h"

#define AWN_TASK_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), AWN_TYPE_TASK, AwnTaskPrivate))

G_DEFINE_TYPE (AwnTask, awn_task, GTK_TYPE_DRAWING_AREA);

#define  AWN_FRAME_RATE	15

#define  AWN_TASK_EFFECT_DIR_DOWN		0
#define  AWN_TASK_EFFECT_DIR_UP			1
#define  AWN_TASK_EFFECT_DIR_LEFT		2
#define  AWN_TASK_EFFECT_DIR_RIGHT		3
#define  M_PI 					3.14159265358979323846

/* FORWARD DECLERATIONS */

static gboolean awn_task_expose (GtkWidget *task, GdkEventExpose *event);
static gboolean awn_task_button_press (GtkWidget *task, GdkEventButton *event);
static gboolean awn_task_proximity_in (GtkWidget *task, GdkEventCrossing *event);
static gboolean awn_task_proximity_out (GtkWidget *task, GdkEventCrossing *event);
static gboolean awn_task_win_enter_in (GtkWidget *window, GdkEventMotion *event, AwnTask *task);
static gboolean awn_task_win_enter_out (GtkWidget *window, GdkEventCrossing *event, AwnTask *task);

static gboolean awn_task_drag_motion (GtkWidget *task, 
		GdkDragContext *context, gint x, gint y, guint t);
static void
awn_task_drag_leave (GtkWidget *widget, GdkDragContext *drag_context,
                                        guint time);
static void awn_task_create_menu(AwnTask *task, GtkMenu *menu);


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
	AwnTaskManager *task_manager;
	AwnSettings *settings;
	
	GnomeDesktopItem *item;
	int pid;
	
	char *application;
	
	gboolean is_launcher;
	AwnSmartLauncher launcher;
	gboolean is_starter;

	WnckWindow *window;
	AwnTitle *title;
	
	gboolean is_active;
	gboolean needs_attention;
	gboolean is_closing;
	gboolean hover;
	
	GdkPixbuf *icon;
	gint icon_width;
	gint icon_height;
	
	/* EFFECT VARIABLES */
	gboolean effect_lock;
	AwnTaskEffect current_effect;
	gint effect_direction;
	gint count;
	
	gint x_offset;
	gint y_offset;
	gint width;
	gint height;
	double rotate_degrees;
	gfloat alpha;
	
	/* random */
	guint timestamp;
	/* signal handler ID's, for clean close */
	gulong icon_changed;
	gulong state_changed;
	gulong win_enter;
	gulong win_leave;
};

/* GLOBALS */

static const GtkTargetEntry drop_types[] = {
	{ "STRING", 0, 0 }
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
	widget_class->drag_motion = awn_task_drag_motion;
	widget_class->drag_leave = awn_task_drag_leave;
	
	g_type_class_add_private (obj_class, sizeof (AwnTaskPrivate));
}

static void
awn_task_init (AwnTask *task)
{
	gtk_widget_add_events (GTK_WIDGET (task),GDK_ALL_EVENTS_MASK);
	
	gtk_widget_set_size_request(GTK_WIDGET(task), 60, 100);
	
	gtk_drag_dest_set (GTK_WIDGET (task),
                           GTK_DEST_DEFAULT_MOTION | GTK_DEST_DEFAULT_DROP,
                           drop_types, n_drop_types,
                           GDK_ACTION_MOVE | GDK_ACTION_COPY);
	
	
#if !GTK_CHECK_VERSION(2,9,0)
			/* TODO: equivalent of following function: */
#else
			//gtk_drag_dest_set_track_motion  (GTK_WIDGET (task),TRUE);
#endif
	gtk_drag_dest_add_uri_targets (GTK_WIDGET (task));
	/* set all priv variables */
	AwnTaskPrivate *priv;
	priv = AWN_TASK_GET_PRIVATE (task);
	
	priv->item = FALSE;
	priv->pid = -1;
	priv->application = NULL;
	priv->is_launcher = FALSE;
	priv->is_starter = FALSE;
	priv->is_closing = FALSE;
	priv->hover = FALSE;
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
	g_timeout_add(AWN_FRAME_RATE, (GSourceFunc)_task_opening_effect, (gpointer)task);
}

static gboolean
_task_launched_effect (AwnTask *task)
{
	AwnTaskPrivate *priv;
	priv = AWN_TASK_GET_PRIVATE (task);
	static gint max = 14;	
	
	static count = 0;
	
	if (priv->effect_lock) {
		if ( priv->current_effect != AWN_TASK_EFFECT_OPENING)
			return TRUE;
	} else {
		priv->effect_lock = TRUE;
		priv->current_effect = AWN_TASK_EFFECT_OPENING;
		priv->effect_direction = AWN_TASK_EFFECT_DIR_UP;
		priv->y_offset = 0;
		priv->count = 0;
	}
	
	if (priv->effect_direction) {
		priv->y_offset +=1;
		
		if (priv->y_offset >= max) 
			priv->effect_direction = AWN_TASK_EFFECT_DIR_DOWN;	
	
	} else {
		priv->y_offset-=1;
		
		if (priv->y_offset < 1) {
			priv->effect_direction = AWN_TASK_EFFECT_DIR_UP;	
			priv->count++;
		}
		if (priv->y_offset < 1 && (priv->window != NULL)) {
			/* finished bouncing, back to normal */
			priv->effect_lock = FALSE;
			priv->current_effect = AWN_TASK_EFFECT_NONE;
			priv->effect_direction = AWN_TASK_EFFECT_DIR_UP;
			priv->y_offset = 0;
			priv->count = 0;
		}
		if ( priv->count > 10 ) {
			priv->effect_lock = FALSE;
			priv->current_effect = AWN_TASK_EFFECT_NONE;
			priv->effect_direction = AWN_TASK_EFFECT_DIR_UP;
			priv->y_offset = 0;
			priv->count = 0;
		}
	}
	
	gtk_widget_queue_draw(GTK_WIDGET(task));
	
	
	if (priv->effect_lock == FALSE)
		return FALSE;
	
	return TRUE;
}


static void
launch_launched_effect (AwnTask *task )
{
	g_timeout_add(30, (GSourceFunc)_task_launched_effect, (gpointer)task);
}

static gboolean
_task_hover_effect (AwnTask *task)
{
	AwnTaskPrivate *priv;

	if (!AWN_IS_TASK (task)) return FALSE;
	
	priv = AWN_TASK_GET_PRIVATE (task);
	static gint max = 10;	
	

	if (priv->effect_lock) {
		if ( priv->current_effect != AWN_TASK_EFFECT_HOVER)
			return TRUE;
	} else {
		priv->effect_lock = TRUE;
		priv->current_effect = AWN_TASK_EFFECT_HOVER;
		priv->effect_direction = AWN_TASK_EFFECT_DIR_UP;
		priv->y_offset = 0;
	}
	
	if (priv->effect_direction) {
		priv->y_offset +=1;
		
		if (priv->y_offset >= max) 
			priv->effect_direction = AWN_TASK_EFFECT_DIR_DOWN;	
	
	} else {
		priv->y_offset-=1;
		
		if (priv->y_offset < 1) {
			/* finished bouncing, back to normal */
			if (priv->hover) 
				priv->effect_direction = AWN_TASK_EFFECT_DIR_UP;
			else {
				priv->effect_lock = FALSE;
				priv->current_effect = AWN_TASK_EFFECT_NONE;
				priv->effect_direction = AWN_TASK_EFFECT_DIR_UP;
				priv->y_offset = 0;
			}
		}
	}
	
	gtk_widget_queue_draw(GTK_WIDGET(task));
	
	
	if (priv->effect_lock == FALSE)
		return FALSE;
	
	return TRUE;
}

static gboolean
_task_hover_effect2 (AwnTask *task)
{
	AwnTaskPrivate *priv;
	priv = AWN_TASK_GET_PRIVATE (task);
	static float max = 1.0;
	static float min = 0.1;
		

	if (priv->effect_lock) {
		if ( priv->current_effect != AWN_TASK_EFFECT_HOVER)
			return TRUE;
	} else {
		priv->effect_lock = TRUE;
		priv->current_effect = AWN_TASK_EFFECT_HOVER;
		priv->effect_direction = AWN_TASK_EFFECT_DIR_UP;
		priv->alpha = 1.0;
	}
	
	if (priv->hover && (priv->alpha <= 0.1))
		return TRUE;
	
	if (priv->effect_direction) {
		priv->alpha -=0.05;
		
		if (priv->alpha <= 0.1) 
			priv->effect_direction = AWN_TASK_EFFECT_DIR_DOWN;	
	
	} else {
		priv->alpha+=0.05;
		
		if (priv->alpha >= 1.0) {
			/* finished bouncing, back to normal */
			if (priv->hover) 
				priv->effect_direction = AWN_TASK_EFFECT_DIR_UP;
			else {
				priv->effect_lock = FALSE;
				priv->current_effect = AWN_TASK_EFFECT_NONE;
				priv->effect_direction = AWN_TASK_EFFECT_DIR_UP;
				priv->alpha = 1.0;
			}
		}
	}
	
	gtk_widget_queue_draw(GTK_WIDGET(task));
	
	
	if (priv->effect_lock == FALSE)
		return FALSE;
	
	return TRUE;
}

static void
launch_hover_effect (AwnTask *task )
{
	g_timeout_add(25, (GSourceFunc)_task_hover_effect, (gpointer)task);
}

static gboolean
_task_attention_effect (AwnTask *task)
{
	AwnTaskPrivate *priv;
	priv = AWN_TASK_GET_PRIVATE (task);
	static gint max = 20;	

	if (priv->effect_lock) {
		if ( priv->current_effect != AWN_TASK_EFFECT_ATTENTION)
			return TRUE;
	} else {
		priv->effect_lock = TRUE;
		priv->current_effect = AWN_TASK_EFFECT_ATTENTION;
		priv->effect_direction = AWN_TASK_EFFECT_DIR_UP;
		priv->y_offset = 0;
		priv->rotate_degrees = 0.0;
	}
	
	if (priv->effect_direction) {
		priv->y_offset +=1;
		
		if (priv->y_offset >= max) 
			priv->effect_direction = AWN_TASK_EFFECT_DIR_DOWN;	
	
	} else {
		priv->y_offset-=1;
		if (priv->y_offset < 1) {
			/* finished bouncing, back to normal */
			if (priv->needs_attention) 
				priv->effect_direction = AWN_TASK_EFFECT_DIR_UP;
			else {
				priv->effect_lock = FALSE;
				priv->current_effect = AWN_TASK_EFFECT_NONE;
				priv->effect_direction = AWN_TASK_EFFECT_DIR_UP;
				priv->y_offset = 0;
				priv->rotate_degrees = 0.0;
			}
		}
	}
		
	gtk_widget_queue_draw(GTK_WIDGET(task));
	
	
	if (priv->effect_lock == FALSE)
		return FALSE;
	
	return TRUE;
}

static void
launch_attention_effect (AwnTask *task )
{
	g_timeout_add(25, (GSourceFunc)_task_attention_effect, (gpointer)task);
}

static gboolean
_task_fade_out_effect (AwnTask *task)
{
	AwnTaskPrivate *priv;
	priv = AWN_TASK_GET_PRIVATE (task);
	static float min = 0.1;
		

	if (priv->hover) {
		//priv->alpha = 1.0;
		return FALSE;
	}
	
	priv->alpha-=0.05;
	
	if (priv->alpha <= 0.2) {
		return FALSE;
	}
	
	gtk_widget_queue_draw(GTK_WIDGET(task));

	return TRUE;

}

static void
launch_fade_out_effect (AwnTask *task )
{
	g_timeout_add(25, (GSourceFunc)_task_fade_out_effect, (gpointer)task);
}


static gboolean
_task_fade_in_effect (AwnTask *task)
{
	AwnTaskPrivate *priv;
	priv = AWN_TASK_GET_PRIVATE (task);
	static float max = 1.0;
	
	priv->alpha+=0.05;
	
	if (priv->alpha >= 1.0 ) {
		gtk_widget_queue_draw(GTK_WIDGET(task));
		return FALSE;
	}
	
	gtk_widget_queue_draw(GTK_WIDGET(task));

	return TRUE;

}

static void
launch_fade_in_effect (AwnTask *task )
{
	g_timeout_add(25, (GSourceFunc)_task_fade_in_effect, (gpointer)task);
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
	

	//g_print("%d, %f\n",priv->y_offset, priv->alpha);	
	if (priv->y_offset >= max) {
		if (priv->is_launcher)
			awn_task_manager_remove_launcher(priv->task_manager, task);
		else
			awn_task_manager_remove_task(priv->task_manager, task);
		
		priv->title = NULL;
		g_free(priv->application);
		gdk_pixbuf_unref (priv->icon);
		gtk_widget_hide (GTK_WIDGET(task));
		gtk_object_destroy (GTK_OBJECT(task));
		task = NULL;
		return FALSE;
	
	} else {
		
		priv->y_offset +=1;
		i = (float) priv->y_offset / max;
		priv->alpha = 1.0 - i;
		gtk_widget_queue_draw(GTK_WIDGET(task));
	}
	
	return TRUE;
	
}


/**********************  CALLBACKS  **********************/

static void
draw (GtkWidget *task, cairo_t *cr)
{
	AwnTaskPrivate *priv;
	AwnSettings *settings;
	GdkPixbuf *pixbuf = NULL;
	double x, y;
	double width, height;
	
	priv = AWN_TASK_GET_PRIVATE (task);
	settings = priv->settings;
	
	width = task->allocation.width;
	height = task->allocation.height;
	
	/* task back */
	cairo_set_source_rgba (cr, 1, 0, 0, 0.0);
	cairo_set_operator (cr, CAIRO_OPERATOR_CLEAR);
	cairo_rectangle(cr, -1, -1, width+1, height+1);
	cairo_fill (cr);
	
	cairo_set_operator (cr, CAIRO_OPERATOR_OVER);
	
	/* active */
	if (priv->window && wnck_window_is_active(priv->window)) {
		
		cairo_set_source_rgba(cr, 1, 1, 1, 0.2);
		cairo_rectangle(cr, 0 , 52, 60,48);
		cairo_fill(cr);
	}	
	/* content */
	if (priv->icon) {
		double x1, y1;
		
		x1 = (width-priv->icon_width)/2;
		y1 = ((50-priv->icon_height)/2) + 50 + (-1 * priv->y_offset);
		
		
		gdk_cairo_set_source_pixbuf (cr, priv->icon, x1, y1);
		if (priv->hover) 
			cairo_paint_with_alpha(cr, 1.0);
		else
			cairo_paint_with_alpha(cr, priv->alpha);
	}
	
	/* arrows */
	double x1, y1;
	x1 = width/2.0;
	cairo_set_source_rgba (cr, settings->arrow_color.red, 
				   settings->arrow_color.green, 
				   settings->arrow_color.blue,
				   settings->arrow_color.alpha);
	cairo_move_to(cr, x1-5, 100);
	cairo_line_to(cr, x1, 95);
	cairo_line_to(cr, x1+5, 100);
	cairo_close_path (cr);

	if (settings->tasks_have_arrows) {
		if (priv->window != NULL) 
			cairo_fill(cr);				
	
	} else if (priv->is_launcher && (priv->window == NULL)) 
		cairo_fill(cr);
	else
		;
}

static gboolean
awn_task_expose (GtkWidget *task, GdkEventExpose *event)
{
	AwnTaskPrivate *priv;
	cairo_t *cr;
	priv = AWN_TASK_GET_PRIVATE (task);
	
	/* get a cairo_t */
	cr = gdk_cairo_create (task->window);

	draw (task, cr);

	cairo_destroy (cr);
	
	return FALSE;
}

static void
awn_task_launch (AwnTask *task, const char *arg_str)
{
	AwnTaskPrivate *priv;
	priv = AWN_TASK_GET_PRIVATE (task);;
	
	GError *err = NULL;
	priv->pid = gnome_desktop_item_launch_on_screen
                                            (priv->item,
                                             NULL,
                                             0,
                                             gdk_screen_get_default(),
                                             -1,
                                             &err);
        
        if (err)
        	g_print("Error: %s", err->message);
        else
        	g_print("Launched application : %d\n", priv->pid);
	
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
				awn_task_create_menu(AWN_TASK(task), menu);
				gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, 
						       NULL, 3, event->time);
				break;
			default:
				return FALSE;
		}
	} else if (priv->is_launcher) {
		switch (event->button) {
			case 1:
				awn_task_launch(task, NULL);
				launch_launched_effect(task);			   
				break;
		
			case 3:
				menu = gtk_menu_new();
				awn_task_create_menu(AWN_TASK(task), menu);
				gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, 
						       NULL, 3, event->time);
				break;
			default:
				return FALSE;
		}
	} else {
		;
	}
	 
	return TRUE;
}

static void 
_task_drag_data_recieved (GtkWidget *widget, GdkDragContext *context, 
				  gint x, gint y, GtkSelectionData *selection_data, 
				  guint target_type, guint time,
                                              AwnTask *task)
{
        
        glong   *_idata;
        gchar   *_sdata;
        GList      *li;
	GList      *list;
	
        gboolean dnd_success = FALSE;
        gboolean delete_selection_data = FALSE;
        
        AwnTaskPrivate *priv;
	priv = AWN_TASK_GET_PRIVATE (task);
	gtk_drag_finish (context, TRUE, FALSE, time);
	
	if (!priv->is_launcher) {
		gtk_drag_finish (context, FALSE, delete_selection_data, time);
		return;
	}
	
	char *res = NULL;
	res = strstr (selection_data->data, ".desktop");
	if (res)
		return;
	
	list = NULL;
        list = gnome_vfs_uri_list_parse ((const char*) selection_data->data);
	for (li = list; li != NULL; li = li->next) {
		GnomeVFSURI *uri = li->data;
		li->data = gnome_vfs_uri_to_string (uri, 0 /* hide_options */);
		gnome_vfs_uri_unref (uri);
	}
	
	GError *err = NULL;
	priv->pid = gnome_desktop_item_launch_on_screen
                                            (priv->item,
                                             list,
                                             0,
                                             gdk_screen_get_default(),
                                             -1,
                                             &err);
        
        if (err)
        	g_print("Error: %s", err->message);
        else
        	g_print("Launched application : %d\n", priv->pid);

	g_list_foreach (list, (GFunc)g_free, NULL);
	g_list_free (list);
	
	return;
        
        
}



static gboolean 
awn_task_proximity_in (GtkWidget *task, GdkEventCrossing *event)
{
	AwnTaskPrivate *priv;
	AwnSettings *settings;
	
	priv = AWN_TASK_GET_PRIVATE (task);
	settings = priv->settings;
	
	if (priv->title) {
		gint i = (int)event->x_root;
		gint x, y, x1;
		gdk_window_get_origin (task->window, &x, &y);
		
		awn_title_show (AWN_TITLE (priv->title), awn_task_get_name(AWN_TASK(task)), x+30, 0);
	
	}
	
	if (priv->hover)
		return TRUE;
	else {
		if (priv->needs_attention)
			return FALSE;
		priv->hover = TRUE;
		
		if (!settings->fade_effect) {
			if (priv->current_effect != AWN_TASK_EFFECT_HOVER)
				launch_hover_effect (AWN_TASK (task));
		} else {
			priv->alpha = 1.0;
			gtk_widget_queue_draw(GTK_WIDGET(task));
			launch_fade_in_effect(task);
		}
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
	priv->hover = FALSE;
	//priv->alpha = 1.0;
	//gtk_widget_queue_draw(GTK_WIDGET(task));
	//launch_fade_in_effect(task);
	return FALSE;
}

static gboolean 
awn_task_win_enter_in (GtkWidget *window, GdkEventMotion *event, AwnTask *task)
{
	AwnTaskPrivate *priv;
	AwnSettings *settings;
	//g_return_if_fail(AWN_IS_TASK(task));
	if (!AWN_IS_TASK (task)) return;
	
	priv = AWN_TASK_GET_PRIVATE (task);
	settings = priv->settings;
	//priv->alpha = 0.2;
	//gtk_widget_queue_draw(GTK_WIDGET(task));
	if (settings->fade_effect)
		launch_fade_out_effect(task);
	return FALSE;
}

static gboolean 
awn_task_win_enter_out (GtkWidget *window, GdkEventCrossing *event, AwnTask *task)
{
	AwnTaskPrivate *priv;
	
	//g_return_if_fail(AWN_IS_TASK(task));
	if (!AWN_IS_TASK (task)) return;
	
	priv = AWN_TASK_GET_PRIVATE (task);
	priv->alpha = 1.0;
	gtk_widget_queue_draw(GTK_WIDGET(task));
	//launch_fade_in_effect(task);
	return FALSE;
}

static gboolean 
activate_window(AwnTask *task)
{
	AwnTaskPrivate *priv;
	priv = AWN_TASK_GET_PRIVATE (task);
	
	if (priv->hover)
		wnck_window_activate( priv->window, priv->timestamp );
	
	return FALSE;
}

static gboolean 
awn_task_drag_motion (GtkWidget *task, 
		GdkDragContext *context, gint x, gint y, guint t)
{
        AwnTaskPrivate *priv;
	priv = AWN_TASK_GET_PRIVATE (task);
	
	if (priv->is_closing)
		return FALSE;
	
	if (priv->window) {
		
		if ( wnck_window_is_active( priv->window ) )
			return FALSE;
		else {
			priv->hover = TRUE;
			priv->timestamp = gtk_get_current_event_time();
			g_timeout_add (1000, (GSourceFunc)activate_window, (gpointer)task);
		}
	}
	
	return FALSE;
}

static void
awn_task_drag_leave (GtkWidget *task, GdkDragContext *drag_context,
                                        guint time)
{
 	AwnTaskPrivate *priv;
	priv = AWN_TASK_GET_PRIVATE (task);
	priv->hover = FALSE;                                       
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

static void
_task_wnck_state_changed (WnckWindow *window, WnckWindowState  old, 
                                WnckWindowState  new, AwnTask *task)
{
	AwnTaskPrivate *priv;
	priv = AWN_TASK_GET_PRIVATE (task);
	
	if (priv->window == NULL )
		return;
	
	if (wnck_window_is_skip_tasklist(priv->window))
		gtk_widget_hide (GTK_WIDGET(task));
	else	
		gtk_widget_show (GTK_WIDGET(task));
	
	if (wnck_window_needs_attention(priv->window)) {
		if (!priv->needs_attention) {
			launch_attention_effect(task);
			priv->needs_attention = TRUE;
		}
	} else
		priv->needs_attention = FALSE;
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
	if (!priv->is_launcher) {
		priv->icon = gdk_pixbuf_copy (awn_x_get_icon (priv->window, 48, 48) );
		priv->icon_width = gdk_pixbuf_get_width(priv->icon);
		priv->icon_height = gdk_pixbuf_get_height(priv->icon);
	
	}
	priv->icon_changed = g_signal_connect (G_OBJECT (priv->window), "icon_changed",
        		  G_CALLBACK (_task_wnck_icon_changed), (gpointer)task); 
        
	priv->state_changed = g_signal_connect (G_OBJECT (priv->window), "state_changed",
        		  G_CALLBACK (_task_wnck_state_changed), (gpointer)task);
        
        /* if launcher, set a launch_sequence
        else if starter, stop the launch_sequence, disable starter flag*/
        
        //awn_task_refresh_icon_geometry(task);
        if (wnck_window_is_skip_tasklist(window))
        	return TRUE;
        
        if (priv->is_launcher)
        	return TRUE;
        	
        launch_opening_effect(task);
	return TRUE;
}

/******************************************************************************
	This function takes a icon name, and searchs through know directories 
	for th icon. 
	
	TODO: There _has_ to be a better way than this, may something I am 
	missing?
*/

GdkPixbuf * 
icon_loader_get_icon( const char *name )
{
        GdkPixbuf *icon = NULL;
        GdkScreen *screen = gdk_screen_get_default();
        GtkIconTheme *theme = NULL;
        theme = gtk_icon_theme_get_for_screen (screen);
        
        if (!name)
                return NULL;
        
        GError *error = NULL;
          
        /* first we try gtkicontheme */
        if (theme)
        	icon = gtk_icon_theme_load_icon( theme, name, 48, 0, &error);
        else {
        	g_print("Icon theme could not be loaded");
        	error = 1;  
        }
        if (error) {
                /* lets try and load directly from file */
                error = NULL;
                GString *str;
                
                if ( strstr(name, "/") != NULL )
                        str = g_string_new(name);
                else {
                        str = g_string_new("/usr/share/pixmaps/");
                        g_string_append(str, name);
                }
                
                icon = gdk_pixbuf_new_from_file_at_scale(str->str, 
                                                         48,
                                                         48,
                                                         TRUE, &error);
                g_string_free(str, TRUE);
        }
        
        if (icon == NULL) {
                /* lets try and load directly from file */
                error = NULL;
                GString *str;
                
                if ( strstr(name, "/") != NULL )
                        str = g_string_new(name);
                else {
                        str = g_string_new("/usr/local/share/pixmaps/");
                        g_string_append(str, name);
                }
                
                icon = gdk_pixbuf_new_from_file_at_scale(str->str, 
                                                         48,
                                                         48,
                                                         TRUE, &error);
                g_string_free(str, TRUE);
        }
        
        if (icon == NULL) {
                error = NULL;
                GString *str;
                
                str = g_string_new("/usr/share/");
                g_string_append(str, name);
                g_string_erase(str, (str->len - 4), -1 );
                g_string_append(str, "/");
		g_string_append(str, "pixmaps/");
		g_string_append(str, name);
		
		icon = gdk_pixbuf_new_from_file_at_scale
       		                             (str->str,
                                             48,
                                             48,
                                             TRUE,
                                             &error);
                g_string_free(str, TRUE);
        }
        
        return icon;
}

gboolean 
awn_task_set_launcher (AwnTask *task, GnomeDesktopItem *item)
{
	AwnTaskPrivate *priv;
	const char *icon_name;
	GtkIconTheme *theme = gtk_icon_theme_get_default();
	
	priv = AWN_TASK_GET_PRIVATE (task);

	priv->is_launcher = TRUE;
	icon_name = gnome_desktop_item_get_icon (item, priv->settings->icon_theme );
	if (!icon_name)
		return FALSE;
	priv->item = item;
	priv->icon = gdk_pixbuf_copy (icon_loader_get_icon(icon_name));
        
	priv->icon_width = gdk_pixbuf_get_width(priv->icon);
	priv->icon_height = gdk_pixbuf_get_height(priv->icon);
	launch_opening_effect(task);
	
	return TRUE;
}

gboolean 
awn_task_is_launcher (AwnTask *task)
{
	AwnTaskPrivate *priv;
	priv = AWN_TASK_GET_PRIVATE (task);
	return priv->is_launcher;
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

gint 
awn_task_get_pid (AwnTask *task)
{
	AwnTaskPrivate *priv;
	priv = AWN_TASK_GET_PRIVATE (task);
	return priv->pid;
}
void 
awn_task_set_is_active (AwnTask *task, gboolean is_active)
{
	AwnTaskPrivate *priv;
	priv = AWN_TASK_GET_PRIVATE (task);
	
	if (priv->window == NULL)
		return;
	priv->is_active = wnck_window_is_active(priv->window);
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
		launch_opening_effect(task);
}	

const char* 
awn_task_get_name (AwnTask *task)
{
	AwnTaskPrivate *priv;
	priv = AWN_TASK_GET_PRIVATE (task);
	
	const char *name = NULL;
		
	
	if (priv->window)
		name =  wnck_window_get_name(priv->window);
	
	else if (priv->is_launcher)
		name = gnome_desktop_item_get_localestring (priv->item, 
						       GNOME_DESKTOP_ITEM_NAME);
	else
		name =  "No name";
	return name;
}

const char* 
awn_task_get_application(AwnTask *task)
{
	AwnTaskPrivate *priv;
	WnckApplication *wnck_app;
	char *app;
	GString *str;
	
	priv = AWN_TASK_GET_PRIVATE (task);
	app = NULL;
	
	if (priv->application)
		return priv->application;
	
	if (priv->is_launcher) {
		
		str = g_string_new(gnome_desktop_item_get_string (priv->item, 
					GNOME_DESKTOP_ITEM_EXEC));
		int i = 0;
		for (i=0; i < str->len; i++) {
			if ( str->str[i] == ' ')
				break;
		}
		
		if (i)
			str = g_string_truncate (str,i);
		priv->application = g_strdup(str->str);
		app = g_string_free (str, TRUE);
		
	} else if (priv->window) {
		wnck_app = wnck_window_get_application(priv->window);
		str = g_string_new (wnck_application_get_name(wnck_app));
		str = g_string_ascii_down (str);
		priv->application = g_strdup(str->str);
		app = g_string_free (str, TRUE);
	
	} else
		priv->application = NULL;
	
	return priv->application;
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

void
awn_task_refresh_icon_geometry (AwnTask *task)
{
	AwnTaskPrivate *priv;
	static gint old_x = 0;
	static gint old_y = 0;
	gint x, y, width, height;
	
	priv = AWN_TASK_GET_PRIVATE (task);
	if (priv->window == NULL)
		return;
	gdk_window_get_origin (GTK_WIDGET(task)->window, &x, &y);
	
	if ( (x != old_x) || (y != old_y) ) {
		gint res = 0;
		
		if (x > old_x)
			res = x - old_x;
		else
			res = old_x - x;
		if (res > 10) {
			//awn_x_set_icon_geometry  (awn_task_get_xid(AWN_TASK(task)), x, y, 60, 50);
			wnck_window_set_icon_geometry (priv->window, x, y, 60, 50);
		}
		old_x = x;
		old_y = y;
	}

}


/********************* MISC FUNCTIONS *******************/

static void
_task_show_prefs (GtkMenuItem *item, AwnTask *task)
{
	g_print("Preferences for %s\n", awn_task_get_name(task));
	/*
		TODO : Provide an interface to change the icon of the current
		       application to one of the users choice. Save it.
	*/
}

typedef struct {
	const char *uri;
	AwnSettings *settings;

} AwnListTerm;

static void
_slist_foreach (char *uri, AwnListTerm *term)
{
	AwnSettings *settings = term->settings;
	
	if ( strcmp (uri, term->uri) == 0 ) {
		//g_print ("URI : %s\n", uri);
		settings->launchers = g_slist_remove(settings->launchers, uri);
		//g_slist_foreach(settings->launchers, (GFunc)_slist_print, (gpointer)term);		
	}
}

static void
_task_remove_launcher (GtkMenuItem *item, AwnTask *task)
{
	AwnTaskPrivate *priv;
	AwnSettings *settings;
	AwnListTerm term;
	GString *uri;
	GSList *res;
	
	priv = AWN_TASK_GET_PRIVATE (task);
	settings = priv->settings;

	priv->is_closing = TRUE;
	priv->hover = FALSE;
	uri = g_string_new (gnome_desktop_item_get_location (priv->item));
	uri = g_string_erase(uri, 0, 7);
	
	g_print ("Remove : %s\n", uri->str);
	term.uri = uri->str;
	term.settings = settings;
	g_slist_foreach(settings->launchers, (GFunc)_slist_foreach, (gpointer)&term);
	
	GConfClient *client = gconf_client_get_default();
		gconf_client_set_list(client,
					"/apps/avant-window-navigator/window_manager/launchers",
					GCONF_VALUE_STRING,settings->launchers,NULL);
					
	priv->window = NULL;
	priv->needs_attention = FALSE;
	/* start closing effect */
	priv->is_closing = TRUE;
	g_timeout_add(AWN_FRAME_RATE, (GSourceFunc)_task_destroy, (gpointer)task);
}

static void 
awn_task_create_menu(AwnTask *task, GtkMenu *menu)
{
	AwnTaskPrivate *priv;
	GtkWidget *item;
	
	priv = AWN_TASK_GET_PRIVATE (task);
	item = NULL;
	
	if (priv->window != NULL) {
		item = gtk_separator_menu_item_new ();
		gtk_menu_shell_prepend (GTK_MENU_SHELL (menu), item);
		gtk_widget_show(item);
		
		item = gtk_image_menu_item_new_from_stock ("gtk-preferences", 
							   NULL);
		gtk_menu_shell_prepend (GTK_MENU_SHELL (menu), item);
		gtk_widget_show(item);
		g_signal_connect (G_OBJECT(item), "activate",
				  G_CALLBACK(_task_show_prefs), (gpointer)task);
	} else if (priv->is_launcher) {
		
		
		item = gtk_image_menu_item_new_from_stock ("gtk-remove", 
							   NULL);
		gtk_menu_shell_prepend (GTK_MENU_SHELL (menu), item);
		gtk_widget_show(item);
		g_signal_connect (G_OBJECT(item), "activate",
				  G_CALLBACK(_task_remove_launcher), (gpointer)task);
	} else {
		;
	}
}

/********************* awn_task_new * *******************/

GtkWidget *
awn_task_new (AwnTaskManager *task_manager, AwnSettings *settings)
{
	GtkWidget *task;
	AwnTaskPrivate *priv;
	task = g_object_new (AWN_TYPE_TASK, NULL);
	priv = AWN_TASK_GET_PRIVATE (task);
	
	priv->task_manager = task_manager;
	priv->settings = settings;
	
	/* This is code which I will add later for better hover effects over 
	the bar */
	priv->win_enter = g_signal_connect(G_OBJECT(settings->window), "motion-notify-event",
			 G_CALLBACK(awn_task_win_enter_in), AWN_TASK(task));
	
	priv->win_leave = g_signal_connect(G_OBJECT(settings->window), "leave-notify-event",
			 G_CALLBACK(awn_task_win_enter_out), AWN_TASK(task));
	
	g_signal_connect (G_OBJECT(task), "drag-data-received",
		  G_CALLBACK(_task_drag_data_recieved), (gpointer)task);
	g_signal_connect (G_OBJECT(task), "drag-end",
		  G_CALLBACK(_task_drag_data_recieved), (gpointer)task);
	return task;
}

void 
awn_task_close (AwnTask *task)
{
	AwnTaskPrivate *priv;
	priv = AWN_TASK_GET_PRIVATE (task);
	
	g_signal_handler_disconnect ((gpointer)priv->window, priv->icon_changed);
	g_signal_handler_disconnect ((gpointer)priv->window, priv->state_changed);
	g_signal_handler_disconnect ((gpointer)priv->settings->window, priv->win_enter);
	g_signal_handler_disconnect ((gpointer)priv->settings->window, priv->win_leave);
	
	priv->window = NULL;
	priv->needs_attention = FALSE;
	if (priv->is_launcher) {
		gtk_widget_queue_draw(GTK_WIDGET(task));
		return;
	}
	/* start closing effect */
	priv->is_closing = TRUE;
	g_timeout_add(AWN_FRAME_RATE, (GSourceFunc)_task_destroy, (gpointer)task);
}






