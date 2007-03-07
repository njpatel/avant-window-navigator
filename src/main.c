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
#include <libgnomevfs/gnome-vfs.h>
#include "dbus/dbus-glib.h"
#include <dbus/dbus-glib-bindings.h>

#include "config.h"

#include "awn-gconf.h"
#include "awn-bar.h"
#include "awn-window.h"
#include "awn-app.h"
#include "awn-win-manager.h"
#include "awn-task-manager.h"
#include "awn-hotspot.h"
#include "awn-utils.h"
#include "awn-task.h"

#define AWN_NAMESPACE "com.google.code.Awn"
#define AWN_OBJECT_PATH "/com/google/code/Awn"


static gboolean expose (GtkWidget *widget, GdkEventExpose *event, AwnSettings *settings);
static gboolean drag_motion (GtkWidget *widget, GdkDragContext *drag_context,
                                                     gint            x,
                                                     gint            y,
                                                     guint           time,
                                                     GtkWidget *win);
static gboolean enter_notify_event (GtkWidget *window, GdkEventCrossing *event, AwnSettings *settings);
static gboolean leave_notify_event (GtkWidget *window, GdkEventCrossing *event, AwnSettings *settings);
static gboolean button_press_event (GtkWidget *window, GdkEventButton *event);
                                                    
int 
main (int argc, char* argv[])
{
	
	AwnSettings* settings;
	GtkWidget *box = NULL;
	GtkWidget *task_manager = NULL;
	
	DBusGConnection *connection;
	DBusGProxy *proxy;
	GError *error = NULL;
	guint32 ret;
	
	g_type_init();	
	gtk_init (&argc, &argv);
	gnome_vfs_init ();
	
	settings = awn_gconf_new();
	settings->bar = awn_bar_new(settings);
	
	settings->window = awn_window_new (settings);
	
	gtk_window_set_policy (GTK_WINDOW (settings->window), FALSE, FALSE, TRUE);
	
	gtk_widget_add_events (GTK_WIDGET (settings->window),GDK_ALL_EVENTS_MASK);
	g_signal_connect(G_OBJECT(settings->window), "expose-event",
			 G_CALLBACK(expose), (gpointer)settings);
	

	box = gtk_hbox_new(FALSE, 2);
	
	if ( argc >= 2) {
		if (argv[1][1] == 'o') {
			task_manager = awn_win_mgr_new(settings);
		}
	}
		
	if (!task_manager)
		task_manager = awn_task_manager_new(settings);
	
	gtk_box_pack_start(GTK_BOX(box), gtk_label_new("    "), FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(box), task_manager, FALSE, TRUE, 0);	
	gtk_box_pack_start(GTK_BOX(box), gtk_label_new("    "), FALSE, FALSE, 0);

	gtk_container_add(GTK_CONTAINER(settings->window), box);
	
	gtk_widget_show_all(settings->bar);
	gtk_widget_show_all(settings->window);
	gtk_window_set_transient_for(GTK_WINDOW(settings->window), GTK_WINDOW(settings->bar));
	
	GtkWidget *hot = awn_hotspot_new (settings);
	gtk_widget_show (hot);
	
	g_signal_connect (G_OBJECT(settings->window), "drag-motion",
	                  G_CALLBACK(drag_motion), (gpointer)settings->window);
	
	g_signal_connect(G_OBJECT(hot), "enter-notify-event",
			 G_CALLBACK(enter_notify_event), (gpointer)settings);	                  
	g_signal_connect(G_OBJECT(settings->window), "leave-notify-event",
			 G_CALLBACK(leave_notify_event), (gpointer)settings);
	
	g_signal_connect(G_OBJECT(settings->window), "button-press-event",
			 G_CALLBACK(button_press_event), (gpointer)settings);
	
	
	/* Get the connection and ensure the name is not used yet */
	connection = dbus_g_bus_get (DBUS_BUS_SESSION, &error);
	if (connection == NULL) {
		g_warning ("Failed to make connection to session bus: %s",
			   error->message);
		g_error_free (error);
		//exit(1);
	}
		
	proxy = dbus_g_proxy_new_for_name (connection, DBUS_SERVICE_DBUS,
					   DBUS_PATH_DBUS, DBUS_INTERFACE_DBUS);
	if (!org_freedesktop_DBus_request_name (proxy, AWN_NAMESPACE,
						0, &ret, &error)) {
		g_warning ("There was an error requesting the name: %s",
			   error->message);
		g_error_free (error);
		//exit(1);
	}
	
	if (ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
		/* Someone else registered the name before us */
		//exit(1);
	}
		
	/* Register the app on the bus */
	dbus_g_connection_register_g_object (connection,
					     AWN_OBJECT_PATH,
					     G_OBJECT (task_manager));
	
	gtk_main ();
	
	return 0;
}

static gboolean
expose (GtkWidget *widget, GdkEventExpose *event, AwnSettings *settings)
{
        gint width, height;
        
        gtk_window_get_size (GTK_WINDOW (widget), &width, &height);
        
        awn_bar_resize(settings->bar, width);
        //awn_window_resize(window, width);
        return FALSE;
}


static gboolean mouse_over_window = FALSE;

static gboolean 
drag_motion (GtkWidget *widget, GdkDragContext *drag_context,
                                                     gint            x,
                                                     gint            y,
                                                     guint           time,
                                                     GtkWidget       *window)
{
	mouse_over_window = TRUE;
	return FALSE;
}

static gboolean 
enter_notify_event (GtkWidget *window, GdkEventCrossing *event, AwnSettings *settings)
{
	awn_show (settings);	
	return FALSE;
}

static gboolean 
leave_notify_event (GtkWidget *window, GdkEventCrossing *event, AwnSettings *settings)
{
	gint width, height;
	gint x, y;
	
	if (settings->auto_hide == FALSE) {
		if (settings->hidden == TRUE)
			awn_show (settings);
		return FALSE;
	}
	gtk_window_get_position (GTK_WINDOW (settings->window), &x, &y);
	gtk_window_get_size (GTK_WINDOW (settings->window), &width, &height);
	
	gint x_root = (int)event->x_root;
	
	if ( (x < x_root) && (x_root < x+width) && ( ( settings->monitor.height - 50) < event->y_root)) {
		
		//g_print ("Do nothing\n", event->y_root);
	} else {
		awn_hide (settings);
	}
	//g_print ("%d < %f < %d", x, event->x_root, x+width);
	return FALSE;
}

static void
prefs_function (GtkMenuItem *menuitem, gpointer null)
{
	GError *err = NULL;
	
	gdk_spawn_command_line_on_screen (gdk_screen_get_default(),
					  "avant-preferences", &err);
	
	if (err)
		g_print("%s\n", err->message);
}


static void
close_function (GtkMenuItem *menuitem, gpointer null)
{
	gtk_main_quit ();
}

static GtkWidget *
create_menu (void)
{
	GtkWidget *menu;
	GtkWidget *item;
	
	menu = gtk_menu_new ();
	
	item = gtk_image_menu_item_new_from_stock (GTK_STOCK_PREFERENCES , NULL);
	gtk_menu_shell_append (GTK_MENU_SHELL(menu), item);
	g_signal_connect (G_OBJECT(item), "activate", G_CALLBACK(prefs_function), NULL);
	
	item = gtk_image_menu_item_new_from_stock (GTK_STOCK_CLOSE, NULL);
	gtk_menu_shell_append (GTK_MENU_SHELL(menu), item);
	g_signal_connect (G_OBJECT(item), "activate", G_CALLBACK(close_function), NULL);
	
	gtk_widget_show_all(menu);
	return menu;
}

static gboolean
button_press_event (GtkWidget *window, GdkEventButton *event)
{
	GtkWidget *menu = NULL;
	
	switch (event->button) {
		case 3:
			menu = create_menu ();
			gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, 
					       NULL, 3, event->time);
			break;
		case 2: // 3v1n0 run preferences on middleclick
			prefs_function (NULL, NULL);
			break;
		default:
			return FALSE;
	}
 
	return FALSE;
}
