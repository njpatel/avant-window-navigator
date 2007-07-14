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

#ifndef __AWN_DEFINES_H__
#define __AWN_DEFINES_H__

#define AWN_GCONF_PATH "/apps/avant-window-navigator"
#define AWN_APPLET_GCONF_PATH "/apps/avant-window-navigator/applets"

#define AWN_MAX_HEIGHT 100
#define AWN_MIN_HEIGHT 12

typedef enum
{
        AWN_ORIENTATION_BOTTOM =0,
        AWN_ORIENTATION_TOP,
        AWN_ORIENTATION_RIGHT,
        AWN_ORIENTATION_LEFT

} AwnOrientation;

#endif
