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
 *  Notes : Main Icon widget for each icon on the bar
*/


#ifndef __AWN_ICON_H__
#define __AWN_ICON_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define AWN_TYPE_ICON		(awn_icon_get_type ())
#define AWN_ICON(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), AWN_TYPE_ICON, AwnIcon))
#define AWN_ICON_CLASS(obj)	(G_TYPE_CHECK_CLASS_CAST ((obj), AWN_ICON, AwnIconClass))
#define AWN_IS_ICON(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), AWN_TYPE_ICON))
#define AWN_IS_ICON_CLASS(obj)	(G_TYPE_CHECK_CLASS_TYPE ((obj), AWN_TYPE_ICON))
#define AWN_ICON_GET_CLASS	(G_TYPE_INSTANCE_GET_CLASS ((obj), AWN_TYPE_ICON, AwnIconClass))

typedef struct _AwnIcon		AwnIcon;
typedef struct _AwnIconClass	AwnIconClass;

struct _AwnIcon
{
	GtkDrawingArea parent;
	
	WnckWindow *window;
	glong xid;
	
	gboolean launcher;

	/* < private > */
};

struct _AwnIconClass
{
	GtkDrawingAreaClass parent_class;

};

GtkWidget *awn_icon_new (void);

G_END_DECLS

#endif
