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

#include "awn-gconf.h"

#include "config.h"


#define BAR_PATH	"/apps/avant-window-navigator/bar"

#define BAR_ROUNDED_CORNERS	"/apps/avant-window-navigator/bar/rounded_corners"	/* bool */
#define BAR_RENDER_PATTERN	"/apps/avant-window-navigator/bar/render_pattern"	/* bool */
#define BAR_PATTERN_URI		"/apps/avant-window-navigator/bar/pattern_uri" 		/* string */
#define BAR_PATTERN_ALPHA 	"/apps/avant-window-navigator/bar/pattern_alpha" 	/* float */



/* globals */
static AwnSettings *settings		= NULL;
static GConfClient *client 		= NULL;
static GtkWidget *update_window		= NULL;

/* prototypes */
static void awn_load_bool(GConfClient *client, const gchar* key, gboolean *data);
static void awn_load_string(GConfClient *client, const gchar* key, gchar **data);
static void awn_load_float(GConfClient *client, const gchar* key, gfloat *data);

static void awn_notify_bool (GConfClient *client, guint cid, GConfEntry *entry, gboolean* data);
static void awn_notify_string (GConfClient *client, guint cid, GConfEntry *entry, gchar** data);
static void awn_notify_float (GConfClient *client, guint cid, GConfEntry *entry, gfloat* data);


AwnSettings* 
awn_gconf_new()
{
	AwnSettings *s = NULL;
	
	s = g_new(AwnSettings, 1);
	settings = s;
	client = gconf_client_get_default();
	
	
	/* Bar settings first */
	gconf_client_add_dir(client, BAR_PATH, GCONF_CLIENT_PRELOAD_NONE, NULL);
	
	awn_load_bool(client, BAR_ROUNDED_CORNERS, &s->rounded_corners);	
	awn_load_bool(client, BAR_RENDER_PATTERN, &s->render_pattern);	
	awn_load_string(client, BAR_PATTERN_URI, &s->pattern_uri);
	awn_load_float(client, BAR_PATTERN_ALPHA, &s->pattern_alpha);
	
	return s;
}

static void
awn_update_window ()
{
	GdkRectangle rect;
	if (update_window == NULL)
		return;
	/* Force a update of the bar */	
	gtk_window_get_size (GTK_WINDOW(update_window), &rect.width, &rect.height);
	gtk_window_get_position (GTK_WINDOW(update_window), &rect.x, &rect.y);
	gdk_window_invalidate_rect (update_window->window, &rect, TRUE);
	gdk_window_process_all_updates ();
	//_on_expose (update_window, NULL);
}

static void 
awn_notify_bool (GConfClient *client, guint cid, GConfEntry *entry, gboolean* data)
{
	GConfValue *value = NULL;
	
	value = gconf_entry_get_value(entry);
	*data = gconf_value_get_bool(value);
}

static void 
awn_notify_string (GConfClient *client, guint cid, GConfEntry *entry, gchar** data)
{
	GConfValue *value = NULL;
	
	value = gconf_entry_get_value(entry);
	*data = gconf_value_get_string(value);
	
	g_print("%s : %s\n",settings->pattern_uri, *data );
}

static void 
awn_notify_float (GConfClient *client, guint cid, GConfEntry *entry, gfloat* data)
{
	GConfValue *value = NULL;
	
	value = gconf_entry_get_value(entry);
	*data = gconf_value_get_float(value);
	
	g_print("%4.2f : %4.2f\n",settings->pattern_alpha, *data );
}

static void
awn_load_bool(GConfClient *client, const gchar* key, gboolean *data)
{
	*data = gconf_client_get_bool(client, key, NULL);
	gconf_client_notify_add (client, key, awn_notify_bool, data, NULL, NULL);
	awn_update_window ();
}

static void
awn_load_string(GConfClient *client, const gchar* key, gchar **data)
{
	*data = gconf_client_get_string(client, key, NULL);
	gconf_client_notify_add (client, key, awn_notify_string, data, NULL, NULL);
	awn_update_window ();
}

static void
awn_load_float(GConfClient *client, const gchar* key, gfloat *data)
{
	*data = gconf_client_get_float(client, key, NULL);
	gconf_client_notify_add (client, key, awn_notify_float, data, NULL, NULL);
	awn_update_window ();
}

void 
awn_gconf_set_window_to_update(GtkWidget *window)
{
	update_window = window;
}
