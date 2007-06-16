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
 *  Notes : This manages the icons on the bar. Opens, closes, hides, and shows
 *          the icons according to the preferences.
*/

#include "config.h"

#include "awn-applet-manager.h"

#include <libawn/awn-applet.h>

#include "awn-applet-proxy.h"

#define AWN_APPLET_MANAGER_GET_PRIVATE(obj) \
        (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        AWN_TYPE_APPLET_MANAGER, AwnAppletManagerPrivate))

G_DEFINE_TYPE (AwnAppletManager, awn_applet_manager, GTK_TYPE_HBOX);

#define AWN_APPLETS_KEY "/apps/avant-window-navigator/applets_list"

static GQuark touch_quark       = 0;

/* STRUCTS & ENUMS */

typedef struct _AwnAppletManagerPrivate AwnAppletManagerPrivate;

struct _AwnAppletManagerPrivate
{
	AwnSettings *settings;
	
	GHashTable *applets;
};

enum
{
	SIGNAL_0,
	ORIENT_CHANGED,
	HEIGHT_CHANGED,
	DELETE_NOTIFY,
	DELETE_APPLET,

	LAST_SIGNAL
};

static guint _appman_signals[LAST_SIGNAL] = { 0 };

static GtkHBoxClass *parent_class = NULL;

void
awn_applet_manager_quit (AwnAppletManager *manager)
{
 	g_signal_emit (manager, _appman_signals[DELETE_NOTIFY], 0);
}

void
awn_applet_manager_load_applets (AwnAppletManager *manager)
{
	AwnAppletManagerPrivate *priv;
	GError *err = NULL;
	GSList *keys = NULL, *k;
	GConfClient *client = gconf_client_get_default ();
	
	g_return_if_fail (AWN_IS_APPLET_MANAGER (manager));
	priv = AWN_APPLET_MANAGER_GET_PRIVATE (manager);     
	
	priv->applets = g_hash_table_new (g_str_hash, g_str_equal);
	
	keys = gconf_client_get_list (client,
                                      AWN_APPLETS_KEY,
                                      GCONF_VALUE_STRING,
                                      &err); 

        if (keys == NULL || err) {
                if (err)
                        g_print ("%s\n", err->message);
                return;        
        }
        
        for (k = keys; k != NULL; k = k->next) {
                GtkWidget *applet = NULL;
                
                gchar **tokens = NULL;
                tokens = g_strsplit (k->data, "::", 2);
                
                if (tokens == NULL) {
                        g_warning ("Bad key: %s", (gchar*)k->data);
                        continue;
                }
                
                applet = awn_applet_proxy_new (tokens[0], tokens[1]);
                g_object_set (G_OBJECT (applet), 
                              "orient", AWN_ORIENTATION_BOTTOM,
                              "height", 50,
                              NULL);
                
                gtk_widget_set_size_request (applet, -1, 100);
                gtk_box_pack_start (GTK_BOX (manager), applet, 
                                    FALSE, FALSE, 0);
                gtk_widget_show_all (GTK_WIDGET (applet));
                
                g_object_set_qdata (G_OBJECT (applet), 
                                    touch_quark, GINT_TO_POINTER (0));
                
                g_hash_table_insert (priv->applets, 
                                     g_strdup (tokens[1]),
                                     applet);
                
                awn_applet_proxy_exec (AWN_APPLET_PROXY (applet));
                

                
                g_print ("APPLET : %s\n", tokens[0]);
                g_strfreev (tokens);
        }
}

static GtkWidget*
_create_applet (AwnAppletManager *manager, const gchar *path, const gchar *uid)
{
	AwnAppletManagerPrivate *priv;
        GtkWidget *applet = NULL;
        	
	g_return_val_if_fail (AWN_IS_APPLET_MANAGER (manager), NULL);
	priv = AWN_APPLET_MANAGER_GET_PRIVATE (manager);   
	
        applet = awn_applet_proxy_new (path, uid);
        g_object_set (G_OBJECT (applet), 
                      "orient", AWN_ORIENTATION_BOTTOM,
                      "height", 50,
                      NULL);
                
        gtk_widget_set_size_request (applet, -1, 100);
        gtk_box_pack_start (GTK_BOX (manager), applet, 
                            FALSE, FALSE, 0);
        gtk_widget_show_all (GTK_WIDGET (applet));
                
        g_object_set_qdata (G_OBJECT (applet), 
                            touch_quark, GINT_TO_POINTER (0));
                
        g_hash_table_insert (priv->applets, 
                             g_strdup (uid),
                             applet);
                
        awn_applet_proxy_exec (AWN_APPLET_PROXY (applet));
        
        return applet;
}

static void
_zero_applets (gpointer key, GtkWidget *applet, AwnAppletManager *manager)
{
        if (G_IS_OBJECT (applet)) {
                g_object_set_qdata (G_OBJECT (applet), touch_quark,
                                    GINT_TO_POINTER (0));
        }
}

static void
_kill_applets (gpointer key, GtkWidget *applet, AwnAppletManager *manager)
{
        const gchar *uid;
        
        if (!G_IS_OBJECT (applet))
                return;
                
        gint touched = GPOINTER_TO_INT (g_object_get_qdata (G_OBJECT (applet),
                                        touch_quark));
        
        if (!touched) {
                g_object_get (applet, "uid", &uid, NULL);
                g_signal_emit (manager, _appman_signals[DELETE_APPLET], 0, uid);
                gtk_widget_destroy (applet);
        }
}

static void 
awn_applet_manger_refresh_applets (GConfClient *client, 
                                   guint cid, 
                                   GConfEntry *entry, 
                                   AwnAppletManager *manager)
{
	AwnAppletManagerPrivate *priv;
	GError *err = NULL;
	GSList *keys = NULL, *k;
        gint i = 0;
	
	g_return_if_fail (AWN_IS_APPLET_MANAGER (manager));
	priv = AWN_APPLET_MANAGER_GET_PRIVATE (manager);     
	
	keys = gconf_client_get_list (client,
                                      AWN_APPLETS_KEY,
                                      GCONF_VALUE_STRING,
                                      &err); 

        if (keys == NULL || err) {
                if (err)
                        g_print ("%s\n", err->message);
                return;        
        }
        
        /* Set the current applets to untouched */
        g_hash_table_foreach (priv->applets, (GHFunc)_zero_applets, manager);
        
        /* Re-order the list & make those which are new */
        for (k = keys; k != NULL; k = k->next) {
                GtkWidget *applet = NULL;
                
                gchar **tokens = NULL;
                tokens = g_strsplit (k->data, "::", 2);
                
                if (tokens == NULL) {
                        g_warning ("Bad key: %s", (gchar*)k->data);
                        continue;
                }
                
                applet = g_hash_table_lookup (priv->applets, tokens[1]);
                
                if (!applet) {
                        g_print ("Creating new applet :%s uid:%s\n", 
                                 tokens[0],
                                 tokens[1]);
                        applet = _create_applet (manager, tokens[0], tokens[1]);
                }
                
                gtk_box_reorder_child (GTK_BOX (manager),GTK_WIDGET (applet),i);
                g_object_set_qdata (G_OBJECT (applet), 
                                    touch_quark, GINT_TO_POINTER (1));

                g_strfreev (tokens);
                i++;
        }
        
        /* foreach applet that wasn't in the list, delete it */
        g_hash_table_foreach (priv->applets, (GHFunc)_kill_applets, manager);
}

static gboolean
awn_applet_manager_delete_applet (AwnAppletManager         *manager,
                                  gchar                    *uid,
                                  GError                  **error)
{
	AwnAppletManagerPrivate *priv;
	GtkWidget *applet = NULL;
	
	g_return_val_if_fail (AWN_IS_APPLET_MANAGER (manager), TRUE);
	priv = AWN_APPLET_MANAGER_GET_PRIVATE (manager);             
        
        applet = g_hash_table_lookup (priv->applets, uid);
        
        if (applet == NULL) {
                g_warning ("Unable to find applet %s", uid);
                *error = g_error_new (GTK_INPUT_ERROR, 1,
                                      "%s", "Cannot find applet");
                return TRUE;
        }
        
        gtk_widget_destroy (applet);
        return TRUE;
}


static void
awn_applet_manager_dispose (GObject *object)
{
 	AwnAppletManagerPrivate *priv;
	
	g_return_if_fail (AWN_IS_APPLET_MANAGER (object));
	priv = AWN_APPLET_MANAGER_GET_PRIVATE (object);
	
	g_hash_table_destroy (priv->applets);

        if (G_OBJECT_CLASS(parent_class)->dispose)
		G_OBJECT_CLASS(parent_class)->dispose(object);  
}

#include "awn-applet-manager-glue.h"

static void
awn_applet_manager_class_init (AwnAppletManagerClass *class)
{
	GObjectClass *obj_class;
	GtkWidgetClass *widget_class;

	obj_class = G_OBJECT_CLASS (class);
	obj_class->dispose = awn_applet_manager_dispose;	
	widget_class = GTK_WIDGET_CLASS (class);
	parent_class = g_type_class_peek_parent (class);
	
	
	/* Class signals */
	_appman_signals[ORIENT_CHANGED] =
		g_signal_new ("orient_changed",
			G_OBJECT_CLASS_TYPE (class),
			G_SIGNAL_RUN_LAST,
			G_STRUCT_OFFSET (AwnAppletManagerClass, orient_changed),
			NULL, NULL,
			g_cclosure_marshal_VOID__INT,
			G_TYPE_NONE,
			1, G_TYPE_INT);

	_appman_signals[HEIGHT_CHANGED] =
		g_signal_new ("height_changed",
			G_OBJECT_CLASS_TYPE (class),
			G_SIGNAL_RUN_LAST,
			G_STRUCT_OFFSET (AwnAppletManagerClass, height_changed),
			NULL, NULL,
			g_cclosure_marshal_VOID__INT,
			G_TYPE_NONE,
			1, G_TYPE_INT);
			
	_appman_signals[DELETE_NOTIFY] =
		g_signal_new ("destroy_notify",
			G_OBJECT_CLASS_TYPE (class),
			G_SIGNAL_RUN_LAST,
			G_STRUCT_OFFSET (AwnAppletManagerClass, destroy_notify),
			NULL, NULL,
			g_cclosure_marshal_VOID__VOID,
			G_TYPE_NONE,
			0, G_TYPE_NONE);

	_appman_signals[DELETE_APPLET] =
		g_signal_new ("destroy_applet",
			G_OBJECT_CLASS_TYPE (class),
			G_SIGNAL_RUN_LAST,
			G_STRUCT_OFFSET (AwnAppletManagerClass, destroy_applet),
			NULL, NULL,
			g_cclosure_marshal_VOID__STRING,
			G_TYPE_NONE,
			1, G_TYPE_STRING);

	g_type_class_add_private (obj_class, sizeof (AwnAppletManagerPrivate));
	
	dbus_g_object_type_install_info (G_TYPE_FROM_CLASS (class),
                                     &dbus_glib_awn_applet_manager_object_info);
}

static void
awn_applet_manager_init (AwnAppletManager *applet_manager)
{
	AwnAppletManagerPrivate *priv;
	GConfClient *client = gconf_client_get_default ();
	
	priv = AWN_APPLET_MANAGER_GET_PRIVATE (applet_manager);

	priv->applets = NULL;
	
	touch_quark = g_quark_from_string ("applets-touch-quark");
        
	/* Setup GConf to notify us if the launchers list changes */
	gconf_client_notify_add (client, AWN_APPLETS_KEY, 
                (GConfClientNotifyFunc)awn_applet_manger_refresh_applets, 
                applet_manager, NULL, NULL);
}

GtkWidget *
awn_applet_manager_new (AwnSettings *settings)
{
	GtkWidget *applet_manager;

	applet_manager = g_object_new (AWN_TYPE_APPLET_MANAGER,
			     "homogeneous", FALSE,
			     "spacing", 0 ,
			     NULL);
	return applet_manager;
}











