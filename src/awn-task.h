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

#ifndef __AWN_TASK_H__
#define __AWN_TASK_H__

#include <gtk/gtk.h>
#include <libgnome/gnome-desktop-item.h>


#define WNCK_I_KNOW_THIS_IS_UNSTABLE 1
#include <libwnck/libwnck.h>

#include "awn-gconf.h"
#include "awn-utils.h" 
#include "awn-title.h"

G_BEGIN_DECLS

#define AWN_TYPE_TASK		(awn_task_get_type ())
#define AWN_TASK(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), AWN_TYPE_TASK, AwnTask))
#define AWN_TASK_CLASS(obj)	(G_TYPE_CHECK_CLASS_CAST ((obj), AWN_TASK, AwnTaskClass))
#define AWN_IS_TASK(obj)	(G_TYPE_CHECK_INSTANCE_TYPE ((obj), AWN_TYPE_TASK))
#define AWN_IS_TASK_CLASS(obj)	(G_TYPE_CHECK_CLASS_TYPE ((obj), AWN_TYPE_TASK))
#define AWN_TASK_GET_CLASS	(G_TYPE_INSTANCE_GET_CLASS ((obj), AWN_TYPE_TASK, AwnTaskClass))

typedef struct _AwnTask		AwnTask;
typedef struct _AwnTaskClass	AwnTaskClass;

struct _AwnTask
{
	GtkDrawingArea parent;

	/* < private > */
};

struct _AwnTaskClass
{
	GtkDrawingAreaClass parent_class;

};

GtkWidget *awn_task_new (AwnSettings *settings);
GtkWidget *awn_task_new_from_window (AwnSettings *settings, WnckWindow *window);
GtkWidget *awn_task_new_from_launcher(AwnSettings *settings, AwnSmartLauncher *launcher);
GtkWidget *awn_task_new_from_startup_notify(AwnSettings *settings);

void awn_task_close (AwnTask *task, GList *remove_from);

gboolean awn_task_get_is_launcher (AwnTask *task);

void awn_task_set_title_(AwnTitle *title);

gboolean awn_task_set_window (AwnTask *task, WnckWindow *window);
WnckWindow * awn_task_get_window (AwnTask *task);

gboolean awn_task_set_launcher (AwnTask *task, GnomeDesktopItem *item);
gboolean awn_task_is_launcher (AwnTask *task);

gulong awn_task_get_xid (AwnTask *task);
gint awn_task_get_pid (AwnTask *task);

void awn_task_set_is_active (AwnTask *task, gboolean is_active);

void awn_task_set_needs_attention (AwnTask *task, gboolean needs_attention);

const char* awn_task_get_name (AwnTask *task);
const char* awn_task_get_application(AwnTask *task);

void awn_task_set_title (AwnTask *task, AwnTitle *title);

AwnSettings* awn_task_get_settings (AwnTask *task);

G_END_DECLS

#endif
