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

#define WNCK_I_KNOW_THIS_IS_UNSTABLE 1
#include <libwnck/libwnck.h>
#include <libgnome/gnome-desktop-item.h>


#include "config.h"
#include "awn-task-manager.h"

#include "awn-title.h"
#include "awn-task.h"

#define AWN_TASK_MANAGER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), AWN_TYPE_TASK_MANAGER, AwnTaskManagerPrivate))

G_DEFINE_TYPE (AwnTaskManager, awn_task_manager, GTK_TYPE_HBOX);

/* FORWARD DECLERATIONS */
static void _task_manager_window_opened (WnckScreen *screen, WnckWindow *window, 
						AwnTaskManager *task_manager);
static void _task_manager_window_closed (WnckScreen *screen, WnckWindow *window,
						 AwnTaskManager *task_manager);
static void _task_manager_window_activate (WnckScreen *screen,
						AwnTaskManager *task_manager);
static void _task_manager_workspace_changed (WnckScreen *screen, 
						AwnTaskManager *task_manager);
static void _refresh_box(AwnTaskManager *task_manager);
static void _task_manager_load_launchers(AwnTaskManager *task_manager);


/* STRUCTS & ENUMS */

typedef struct _AwnTaskManagerPrivate AwnTaskManagerPrivate;

struct _AwnTaskManagerPrivate
{
	AwnSettings *settings;
	
	WnckScreen *screen;
	
	GtkWidget *title_window;
	
	GList *launchers;
	GList *tasks;
	
	
};

/* GLOBALS */


static void
awn_task_manager_class_init (AwnTaskManagerClass *class)
{
	GObjectClass *obj_class;
	GtkWidgetClass *widget_class;

	obj_class = G_OBJECT_CLASS (class);
	widget_class = GTK_WIDGET_CLASS (class);

	g_type_class_add_private (obj_class, sizeof (AwnTaskManagerPrivate));
}

static void
awn_task_manager_init (AwnTaskManager *task_manager)
{
	/* set all priv variables */
	AwnTaskManagerPrivate *priv;
	priv = AWN_TASK_MANAGER_GET_PRIVATE (task_manager);
	
	priv->screen = wnck_screen_get_default();
	
	priv->title_window = NULL;
	priv->launchers = NULL;
	priv->tasks = NULL;
}

/***************************************************************/

static void
_load_launchers_func (const char *uri, AwnTaskManager *task_manager)
{	
	AwnTaskManagerPrivate *priv;
	GtkWidget *task = NULL;
	GnomeDesktopItem *item= NULL;
	
	priv = AWN_TASK_MANAGER_GET_PRIVATE (task_manager);
	
	item = gnome_desktop_item_new_from_file (uri,                             
				GNOME_DESKTOP_ITEM_LOAD_ONLY_IF_EXISTS, NULL);
	if (item == NULL)
		return;
	
	task = awn_task_new(priv->settings);
	awn_task_set_title (AWN_TASK(task), priv->title_window);
	if (awn_task_set_launcher (AWN_TASK (task), item)) {
		priv->launchers = g_list_append(priv->launchers, (gpointer)task);
		gtk_box_pack_start(GTK_BOX(task_manager), task, FALSE, FALSE, 0);
	
		_refresh_box (task_manager);
		g_print("LOADED : %s\n", uri);
	} else {
		gtk_widget_destroy(task);
		gnome_desktop_item_unref(item);
		g_print("FAILED : %s\n", uri);
	}
}

static void
_task_manager_load_launchers(AwnTaskManager *task_manager)
{
	AwnTaskManagerPrivate *priv;
	GtkWidget *task = NULL;
	
	priv = AWN_TASK_MANAGER_GET_PRIVATE (task_manager);
	
	g_slist_foreach (priv->settings->launchers, (GFunc)_load_launchers_func,
								 task_manager);
}

typedef struct {
	AwnTask *task;
	int pid;
	
} AwnLauncherTerm;

static void
_find_launcher (AwnTask *task, AwnLauncherTerm *term)
{
	int pid = awn_task_get_pid(task);
	
	//g_print("%d == %d\n", pid, term->pid);
	if (term->pid == pid) {
		term->task = task;
	} else
		return;
}

GtkWidget *
_task_manager_window_has_launcher (AwnTaskManager *task_manager, 
							   WnckWindow *window)
{
	AwnTaskManagerPrivate *priv;
	GtkWidget *task = NULL;
	AwnLauncherTerm term;
	
	priv = AWN_TASK_MANAGER_GET_PRIVATE (task_manager);
	
	term.task = NULL;
	term.pid = wnck_window_get_pid(window);
	
	if (term.pid == 0) {
		WnckApplication *app = wnck_window_get_application(window);
		term.pid = wnck_application_get_pid(app);
	}
	//g_print("New window Pid = %d\n", term.pid);
	
	g_list_foreach(priv->launchers, (GFunc)_find_launcher, (gpointer)&term);
	
	task = term.task;
	
	if (task != NULL)
		return term.task;	
	return NULL;
}

GtkWidget *
_task_manager_window_has_starter (WnckWindow *window)
{
	/*
	Go through each task
		if task is starter for window
		return starter;
	*/
	return NULL;
}

static void 
_task_manager_window_opened (WnckScreen *screen, WnckWindow *window, 
						AwnTaskManager *task_manager)
{
	AwnTaskManagerPrivate *priv;
	GtkWidget *task = NULL;

	priv = AWN_TASK_MANAGER_GET_PRIVATE (task_manager);
	
	/* first check if it has a launcher */
	task = _task_manager_window_has_launcher(task_manager, window);
	if (task != NULL) {
		//g_print("\n\n\nFound launcher for %s\n\n\n", wnck_window_get_name(window));
		awn_task_set_window (AWN_TASK (task), window);
	}
	
	/* check startup notification */
	if (task == NULL) {
		task = _task_manager_window_has_starter(window);
		if (task)
			awn_task_set_window (AWN_TASK (task), window);	
	}	
	
	/* if not launcher & no starter, create new task */
	if (task == NULL) {
		task = awn_task_new(priv->settings);
		if (awn_task_set_window (AWN_TASK (task), window))
			;//g_print("Created for %s\n", wnck_window_get_name(window));
		awn_task_set_title (AWN_TASK(task), priv->title_window);
		priv->tasks = g_list_append(priv->tasks, (gpointer)task);
		gtk_box_pack_start(GTK_BOX(task_manager), task, FALSE, FALSE, 0);
	}
	
	_refresh_box (task_manager);
}

typedef struct {
	gulong xid;
	GList *list;
} AwnDestroyTerm;

static void
_task_destroy (AwnTask *task, AwnDestroyTerm *term) 
{
	guint window_id, task_id;
	
	g_return_if_fail(AWN_IS_TASK(task));
	
	//window_id = GPOINTER_TO_UINT(xid);
	task_id = awn_task_get_xid(task);
	
	if (term->xid == task_id)
		awn_task_close(task, term->list);
}

static void 
_task_manager_window_closed (WnckScreen *screen, WnckWindow *window,
						 AwnTaskManager *task_manager)
{
	g_return_if_fail (WNCK_IS_WINDOW (window));
	
	AwnTaskManagerPrivate *priv;
	AwnDestroyTerm term;
	guint xid;
	
	priv = AWN_TASK_MANAGER_GET_PRIVATE (task_manager);
	
	xid = wnck_window_get_xid (window);
	term.xid = xid;
	term.list = priv->launchers;
	g_list_foreach (priv->launchers, _task_destroy, (gpointer)&term);
	term.list = priv->tasks;
	g_list_foreach (priv->tasks, _task_destroy, (gpointer)&term);
	
	_refresh_box(task_manager);
}

static void
_task_activate (AwnTask *task, gpointer xid) 
{
	guint window_id, task_id;
	
	g_return_if_fail(AWN_IS_TASK(task));
	
	window_id = GPOINTER_TO_UINT(xid);
	task_id = awn_task_get_xid(task);
	
	if (window_id == task_id)
		awn_task_set_is_active(task, TRUE);
	else
		awn_task_set_is_active(task, FALSE);
}

static void 
_task_manager_window_activate (WnckScreen *screen,
						AwnTaskManager *task_manager)
{
	AwnTaskManagerPrivate *priv;
	WnckWindow *window;
	guint xid;
	
	priv = AWN_TASK_MANAGER_GET_PRIVATE (task_manager);	
	
	window = wnck_screen_get_active_window(screen);
	if (!window)
		return;
	
	xid = wnck_window_get_xid(window);
	g_list_foreach (priv->launchers, _task_activate, GUINT_TO_POINTER(xid));
	g_list_foreach (priv->tasks, _task_activate, GUINT_TO_POINTER(xid));
	
	_refresh_box(task_manager);
}

static void 
_task_manager_workspace_changed (WnckScreen *screen, 
						AwnTaskManager *task_manager)
{
	g_print("\n***AWN-TASK-MANAGER*** - Workspace Changed\n");
}

static void
_task_refresh (AwnTask *task, WnckWorkspace *space)
{
	WnckWindow *window;
	AwnSettings *settings;
	
	if (task == NULL)
		return;
	
	g_return_if_fail(AWN_IS_TASK (task));
	
	settings = awn_task_get_settings(task);
	window = awn_task_get_window(task);
	
	if (!space)
		return;
	
	if (awn_task_is_launcher (task)) {
		gtk_widget_show (GTK_WIDGET (task));
		return;
	}	
	
	if ( wnck_window_is_skip_tasklist (window)) {
		gtk_widget_hide (GTK_WIDGET (task));
		return;
	}
	
	if (settings->show_all_windows) {
		gtk_widget_show_all (GTK_WIDGET (task));
		return;
	}
	
	if (wnck_window_is_in_viewport (window, space))
		gtk_widget_show_all (GTK_WIDGET (task));
	else
		gtk_widget_hide (GTK_WIDGET (task));
}

static void 
_refresh_box(AwnTaskManager *task_manager)
{
	AwnTaskManagerPrivate *priv;
	WnckWorkspace *space;
	
	priv = AWN_TASK_MANAGER_GET_PRIVATE (task_manager);
	
	space = wnck_screen_get_active_workspace(priv->screen);
	g_list_foreach(priv->launchers, _task_refresh, (gpointer)space);
	g_list_foreach(priv->tasks, _task_refresh, (gpointer)space);
}


/********************* awn_task_manager_new * *******************/

GtkWidget *
awn_task_manager_new (AwnSettings *settings)
{
	GtkWidget *task_manager;
	AwnTaskManagerPrivate *priv;
	
	task_manager = g_object_new (AWN_TYPE_TASK_MANAGER, 
			     "homogeneous", FALSE,
			     "spacing", 0 ,
			     NULL);
	priv = AWN_TASK_MANAGER_GET_PRIVATE (task_manager);
	
	priv->settings = settings;
	
	priv->title_window = awn_title_new(priv->settings);
	awn_title_show(priv->title_window, " ", 0, 0);
	gtk_widget_show(priv->title_window);
	
	
	_task_manager_load_launchers(task_manager);
	
	/* LIBWNCK SIGNALS */
	g_signal_connect (G_OBJECT(priv->screen), "window_opened",
	                  G_CALLBACK (_task_manager_window_opened), 
	                  (gpointer)task_manager);
	
	g_signal_connect (G_OBJECT(priv->screen), "window_closed",
	                  G_CALLBACK(_task_manager_window_closed), 
	                  (gpointer)task_manager);
	
	g_signal_connect (G_OBJECT(priv->screen), "active_window_changed",
	                  G_CALLBACK(_task_manager_window_activate), 
	                  (gpointer)task_manager);	
	return task_manager;
}











