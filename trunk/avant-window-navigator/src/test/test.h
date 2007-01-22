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


#ifndef	_AWN_BAR_H
#define	_AWN_BAR_H

#include <glib.h>
#include <gtk/gtk.h>


G_BEGIN_DECLS

#define AWN_BAR_TYPE      (awn_bar_get_type())
#define AWN_BAR(o)        (G_TYPE_CHECK_INSTANCE_CAST((o), AWN_BAR_TYPE, AwnBar))
#define AWN_BAR_CLASS(c)  (G_TYPE_CHECK_CLASS_CAST((c), AWN_BAR_TYPE, AwnBarClass))
#define IS_AWN_BAR(o)     (G_TYPE_CHECK_INSTANCE_TYPE((o), AWN_BAR_TYPE))
#define IS_AWN_BAR_CLASS  (G_TYPE_INSTANCE_GET_CLASS((o), AWN_BAR_TYPE, AwnBarClass))

typedef struct _AwnBar AwnBar;
typedef struct _AwnBarClass AwnBarClass;

struct _AwnBar {
        GtkWindow win;
      
};

struct _AwnBarClass {
        GtkWindowClass parent_class;
};

static GtkWidgetClass *parent_class = NULL;

GType awn_bar_get_type(void);

GtkWidget *awn_bar_new(void);

void awn_bar_resize(AwnBar *bar);


G_END_DECLS


#endif /* _AWN_BAR_H */

