/* Xlib utils */

/*
 * Copyright (C) 2001 Havoc Pennington
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <config.h>
#include "xutils.h"
#include <string.h>
#include <stdio.h>
#include "inlinepixbufs.h"



static WnckClientType client_type = 0; 



WnckClientType
_wnck_get_client_type ()
{
  /* If the type hasn't been set yet, use the default--treat it as a
   * normal application.
   */
  if (client_type == 0)
    client_type = WNCK_CLIENT_TYPE_APPLICATION;

  return client_type;
}

gboolean
_wnck_get_cardinal (Window  xwindow,
                    Atom    atom,
                    int    *val)
{
  Atom type;
  int format;
  gulong nitems;
  gulong bytes_after;
  gulong *num;
  int err, result;

  *val = 0;
  
  _wnck_error_trap_push ();
  type = None;
  result = XGetWindowProperty (gdk_display,
			       xwindow,
			       atom,
			       0, G_MAXLONG,
			       False, XA_CARDINAL, &type, &format, &nitems,
			       &bytes_after, (void*)&num);  
  err = _wnck_error_trap_pop ();
  if (err != Success ||
      result != Success)
    return FALSE;
  
  if (type != XA_CARDINAL)
    {
      XFree (num);
      return FALSE;
    }

  *val = *num;
  
  XFree (num);

  return TRUE;
}

int
_wnck_get_wm_state (Window  xwindow)
{
  Atom type;
  int format;
  gulong nitems;
  gulong bytes_after;
  gulong *num;
  int err, result;
  Atom wm_state;
  int retval;

  wm_state = _wnck_atom_get ("WM_STATE");
  retval = NormalState;
  
  _wnck_error_trap_push ();
  type = None;
  result = XGetWindowProperty (gdk_display,
			       xwindow,
			       wm_state,
			       0, G_MAXLONG,
			       False, wm_state, &type, &format, &nitems,
			       &bytes_after, (void*)&num);
  err = _wnck_error_trap_pop ();
  if (err != Success ||
      result != Success)
    return retval;
  
  if (type != wm_state)
    {
      XFree (num);
      return retval;
    }

  retval = *num;
  
  XFree (num);

  return retval;
}

gboolean
_wnck_get_window (Window  xwindow,
                  Atom    atom,
                  Window *val)
{
  Atom type;
  int format;
  gulong nitems;
  gulong bytes_after;
  Window *w;
  int err, result;

  *val = 0;
  
  _wnck_error_trap_push ();
  type = None;
  result = XGetWindowProperty (gdk_display,
			       xwindow,
			       atom,
			       0, G_MAXLONG,
			       False, XA_WINDOW, &type, &format, &nitems,
			       &bytes_after, (void*)&w);  
  err = _wnck_error_trap_pop ();
  if (err != Success ||
      result != Success)
    return FALSE;
  
  if (type != XA_WINDOW)
    {
      XFree (w);
      return FALSE;
    }

  *val = *w;
  
  XFree (w);

  return TRUE;
}

gboolean
_wnck_get_pixmap (Window  xwindow,
                  Atom    atom,
                  Pixmap *val)
{
  Atom type;
  int format;
  gulong nitems;
  gulong bytes_after;
  Window *w;
  int err, result;

  *val = 0;
  
  _wnck_error_trap_push ();
  type = None;
  result = XGetWindowProperty (gdk_display,
			       xwindow,
			       atom,
			       0, G_MAXLONG,
			       False, XA_PIXMAP, &type, &format, &nitems,
			       &bytes_after, (void*)&w);  
  err = _wnck_error_trap_pop ();
  if (err != Success ||
      result != Success)
    return FALSE;
  
  if (type != XA_PIXMAP)
    {
      XFree (w);
      return FALSE;
    }

  *val = *w;
  
  XFree (w);

  return TRUE;
}

gboolean
_wnck_get_atom (Window  xwindow,
                Atom    atom,
                Atom   *val)
{
  Atom type;
  int format;
  gulong nitems;
  gulong bytes_after;
  Atom *a;
  int err, result;

  *val = 0;
  
  _wnck_error_trap_push ();
  type = None;
  result = XGetWindowProperty (gdk_display,
			       xwindow,
			       atom,
			       0, G_MAXLONG,
			       False, XA_ATOM, &type, &format, &nitems,
			       &bytes_after, (void*)&a);  
  err = _wnck_error_trap_pop ();
  if (err != Success ||
      result != Success)
    return FALSE;
  
  if (type != XA_ATOM)
    {
      XFree (a);
      return FALSE;
    }

  *val = *a;
  
  XFree (a);

  return TRUE;
}

static char*
text_property_to_utf8 (const XTextProperty *prop)
{
  char **list;
  int count;
  char *retval;
  
  list = NULL;

  count = gdk_text_property_to_utf8_list (gdk_x11_xatom_to_atom (prop->encoding),
                                          prop->format,
                                          prop->value,
                                          prop->nitems,
                                          &list);

  if (count == 0)
    return NULL;

  retval = list[0];
  list[0] = g_strdup (""); /* something to free */
  
  g_strfreev (list);

  return retval;
}

char*
_wnck_get_text_property (Window  xwindow,
                         Atom    atom)
{
  XTextProperty text;
  char *retval;
  
  _wnck_error_trap_push ();

  text.nitems = 0;
  if (XGetTextProperty (gdk_display,
                        xwindow,
                        &text,
                        atom))
    {
      retval = text_property_to_utf8 (&text);

      if (text.value)
        XFree (text.value);
    }
  else
    {
      retval = NULL;
    }
  
  _wnck_error_trap_pop ();

  return retval;
}

static char*
_wnck_get_string_property_latin1 (Window  xwindow,
                                  Atom    atom)
{
  Atom type;
  int format;
  gulong nitems;
  gulong bytes_after;
  guchar *str;
  int err, result;
  char *retval;
  
  _wnck_error_trap_push ();
  str = NULL;
  result = XGetWindowProperty (gdk_display,
			       xwindow, atom,
			       0, G_MAXLONG,
			       False, XA_STRING, &type, &format, &nitems,
			       &bytes_after, (guchar **)&str);  

  err = _wnck_error_trap_pop ();
  if (err != Success ||
      result != Success)
    return NULL;
  
  if (type != XA_STRING)
    {
      XFree (str);
      return NULL;
    }

  retval = g_strdup ( (gchar *)str );
  
  XFree (str);
  
  return retval;
}

char*
_wnck_get_utf8_property (Window  xwindow,
                         Atom    atom)
{
  Atom type;
  int format;
  gulong nitems;
  gulong bytes_after;
  guchar *val;
  int err, result;
  char *retval;
  Atom utf8_string;

  utf8_string = _wnck_atom_get ("UTF8_STRING");
  
  _wnck_error_trap_push ();
  type = None;
  val = NULL;
  result = XGetWindowProperty (gdk_display,
			       xwindow,
			       atom,
			       0, G_MAXLONG,
			       False, utf8_string,
			       &type, &format, &nitems,
			       &bytes_after, (guchar **)&val);  
  err = _wnck_error_trap_pop ();

  if (err != Success ||
      result != Success)
    return NULL;
  
  if (type != utf8_string ||
      format != 8 ||
      nitems == 0)
    {
      if (val)
        XFree (val);
      return NULL;
    }

  if (!g_utf8_validate ( (gchar *)val, nitems, NULL))
    {
      g_warning ("Property %s contained invalid UTF-8\n",
                 _wnck_atom_name (atom));
      XFree (val);
      return NULL;
    }
  
  retval = g_strndup ( (gchar *)val, nitems);
  
  XFree (val);
  
  return retval;
}

gboolean
_wnck_get_window_list (Window   xwindow,
                       Atom     atom,
                       Window **windows,
                       int     *len)
{
  Atom type;
  int format;
  gulong nitems;
  gulong bytes_after;
  Window *data;
  int err, result;

  *windows = NULL;
  *len = 0;
  
  _wnck_error_trap_push ();
  type = None;
  result = XGetWindowProperty (gdk_display,
			       xwindow,
			       atom,
			       0, G_MAXLONG,
			       False, XA_WINDOW, &type, &format, &nitems,
			       &bytes_after, (void*)&data);  
  err = _wnck_error_trap_pop ();
  if (err != Success ||
      result != Success)
    return FALSE;
  
  if (type != XA_WINDOW)
    {
      XFree (data);
      return FALSE;
    }

  *windows = g_new (Window, nitems);
  memcpy (*windows, data, sizeof (Window) * nitems);
  *len = nitems;
  
  XFree (data);

  return TRUE;  
}

gboolean
_wnck_get_atom_list (Window   xwindow,
                     Atom     atom,
                     Atom   **atoms,
                     int     *len)
{
  Atom type;
  int format;
  gulong nitems;
  gulong bytes_after;
  Atom *data;
  int err, result;

  *atoms = NULL;
  *len = 0;
  
  _wnck_error_trap_push ();
  type = None;
  result = XGetWindowProperty (gdk_display,
			       xwindow,
			       atom,
			       0, G_MAXLONG,
			       False, XA_ATOM, &type, &format, &nitems,
			       &bytes_after, (void*)&data);
  err = _wnck_error_trap_pop ();
  if (err != Success ||
      result != Success)
    return FALSE;
  
  if (type != XA_ATOM)
    {
      XFree (data);
      return FALSE;
    }

  *atoms = g_new (Atom, nitems);
  memcpy (*atoms, data, sizeof (Atom) * nitems);
  *len = nitems;
  
  XFree (data);

  return TRUE;
}

gboolean
_wnck_get_cardinal_list (Window   xwindow,
                         Atom     atom,
                         gulong **cardinals,
                         int     *len)
{
  Atom type;
  int format;
  gulong nitems;
  gulong bytes_after;
  gulong *nums;
  int err, result;

  *cardinals = NULL;
  *len = 0;
  
  _wnck_error_trap_push ();
  type = None;
  result = XGetWindowProperty (gdk_display,
			       xwindow,
			       atom,
			       0, G_MAXLONG,
			       False, XA_CARDINAL, &type, &format, &nitems,
			       &bytes_after, (void*)&nums);  
  err = _wnck_error_trap_pop ();
  if (err != Success ||
      result != Success)
    return FALSE;
  
  if (type != XA_CARDINAL)
    {
      XFree (nums);
      return FALSE;
    }

  *cardinals = g_new (gulong, nitems);
  memcpy (*cardinals, nums, sizeof (gulong) * nitems);
  *len = nitems;
  
  XFree (nums);

  return TRUE;
}

char**
_wnck_get_utf8_list (Window   xwindow,
                     Atom     atom)
{
  Atom type;
  int format;
  gulong nitems;
  gulong bytes_after;
  char *val;
  int err, result;
  Atom utf8_string;
  char **retval;
  guint i;
  guint n_strings;
  char *p;
  
  utf8_string = _wnck_atom_get ("UTF8_STRING");
  
  _wnck_error_trap_push ();
  type = None;
  val = NULL;
  result = XGetWindowProperty (gdk_display,
			       xwindow,
			       atom,
			       0, G_MAXLONG,
			       False, utf8_string,
			       &type, &format, &nitems,
			       &bytes_after, (void*)&val);  
  err = _wnck_error_trap_pop ();

  if (err != Success ||
      result != Success)
    return NULL;
  
  if (type != utf8_string ||
      format != 8 ||
      nitems == 0)
    {
      if (val)
        XFree (val);
      return NULL;
    }

  /* I'm not sure this is right, but I'm guessing the
   * property is nul-separated
   */
  i = 0;
  n_strings = 0;
  while (i < nitems)
    {
      if (val[i] == '\0')
        ++n_strings;
      ++i;
    }

  if (val[nitems - 1] != '\0')
    ++n_strings;

  /* we're guaranteed that val has a nul on the end
   * by XGetWindowProperty
   */
  
  retval = g_new0 (char*, n_strings + 1);

  p = val;
  i = 0;
  while (i < n_strings)
    {
      if (!g_utf8_validate (p, -1, NULL))
        {
          g_warning ("Property %s contained invalid UTF-8\n",
                     _wnck_atom_name (atom));
          XFree (val);
          g_strfreev (retval);
          return NULL;
        }

      retval[i] = g_strdup (p);
      
      p = p + strlen (p) + 1;
      ++i;
    }
  
  XFree (val);
  
  return retval;
}

void
_wnck_set_utf8_list (Window   xwindow,
                     Atom     atom,
                     char   **list)
{
  Atom utf8_string;
  GString *flattened;
  int i;
  
  utf8_string = _wnck_atom_get ("UTF8_STRING");  

  /* flatten to nul-separated list */
  flattened = g_string_new ("");
  i = 0;
  while (list[i] != NULL)
    {
      g_string_append_len (flattened, list[i],
                           strlen (list[i]) + 1);
      ++i;
    }

  _wnck_error_trap_push ();
  
  XChangeProperty (gdk_display,
		   xwindow,
                   atom,
		   utf8_string, 8, PropModeReplace,
		   (guchar *) flattened->str, flattened->len);
  
  _wnck_error_trap_pop ();

  g_string_free (flattened, TRUE);
}

void
_wnck_error_trap_push (void)
{
  gdk_error_trap_push ();
}

int
_wnck_error_trap_pop (void)
{
  XSync (gdk_display, False);
  return gdk_error_trap_pop ();
}

static GHashTable *atom_hash = NULL;
static GHashTable *reverse_atom_hash = NULL;

Atom
_wnck_atom_get (const char *atom_name)
{
  Atom retval;
  
  g_return_val_if_fail (atom_name != NULL, None);

  if (!atom_hash)
    {
      atom_hash = g_hash_table_new (g_str_hash, g_str_equal);
      reverse_atom_hash = g_hash_table_new (NULL, NULL);
    }
      
  retval = GPOINTER_TO_UINT (g_hash_table_lookup (atom_hash, atom_name));
  if (!retval)
    {
      retval = XInternAtom (gdk_display, atom_name, FALSE);

      if (retval != None)
        {
          char *name_copy;

          name_copy = g_strdup (atom_name);
          
          g_hash_table_insert (atom_hash,
                               name_copy,
                               GUINT_TO_POINTER (retval));
          g_hash_table_insert (reverse_atom_hash,
                               GUINT_TO_POINTER (retval),
                               name_copy);
        }
    }

  return retval;
}

const char*
_wnck_atom_name (Atom atom)
{
  if (reverse_atom_hash)
    return g_hash_table_lookup (reverse_atom_hash, GUINT_TO_POINTER (atom));
  else
    return NULL;
}

static GdkFilterReturn
filter_func (GdkXEvent  *gdkxevent,
             GdkEvent   *event,
             gpointer    data)
{


  
  return GDK_FILTER_CONTINUE;
}

void
_wnck_event_filter_init (void)
{
  static gboolean initialized = FALSE;

  if (!initialized)
    {
      gdk_window_add_filter (NULL, filter_func, NULL);
      initialized = TRUE;
    }
}

int
_wnck_xid_equal (gconstpointer v1,
                 gconstpointer v2)
{
  return *((const gulong*) v1) == *((const gulong*) v2);
}

guint
_wnck_xid_hash (gconstpointer v)
{
  gulong val = * (const gulong *) v;

  /* I'm not sure this works so well. */
#if G_SIZEOF_LONG > 4
  return (guint) (val ^ (val >> 32));
#else
  return val;
#endif
}

void
_wnck_iconify (Window xwindow)
{
  _wnck_error_trap_push ();
  XIconifyWindow (gdk_display, xwindow, DefaultScreen (gdk_display));
  _wnck_error_trap_pop ();
}

void
_wnck_deiconify (Window xwindow)
{
  /* We need special precautions, because GDK doesn't like
   * XMapWindow() called on its windows, need to use the
   * GDK functions
   */
  GdkWindow *gdkwindow;

  gdkwindow = gdk_xid_table_lookup (xwindow);

  _wnck_error_trap_push ();
  if (gdkwindow)
    gdk_window_show (gdkwindow);
  else
    XMapRaised (gdk_display, xwindow);
  _wnck_error_trap_pop ();
}

void
_wnck_close (Screen *screen,
	     Window  xwindow,
	     Time    timestamp)
{
  XEvent xev;
  
  xev.xclient.type = ClientMessage;
  xev.xclient.serial = 0;
  xev.xclient.send_event = True;
  xev.xclient.display = gdk_display;
  xev.xclient.window = xwindow;
  xev.xclient.message_type = _wnck_atom_get ("_NET_CLOSE_WINDOW");
  xev.xclient.format = 32;
  xev.xclient.data.l[0] = timestamp;
  xev.xclient.data.l[1] = _wnck_get_client_type ();
  xev.xclient.data.l[2] = 0;
  xev.xclient.data.l[3] = 0;
  xev.xclient.data.l[4] = 0;

  XSendEvent (gdk_display,
              RootWindowOfScreen (screen),
              False,
	      SubstructureRedirectMask | SubstructureNotifyMask,
	      &xev); 
}

#define _NET_WM_MOVERESIZE_SIZE_TOPLEFT      0
#define _NET_WM_MOVERESIZE_SIZE_TOP          1
#define _NET_WM_MOVERESIZE_SIZE_TOPRIGHT     2
#define _NET_WM_MOVERESIZE_SIZE_RIGHT        3
#define _NET_WM_MOVERESIZE_SIZE_BOTTOMRIGHT  4
#define _NET_WM_MOVERESIZE_SIZE_BOTTOM       5
#define _NET_WM_MOVERESIZE_SIZE_BOTTOMLEFT   6
#define _NET_WM_MOVERESIZE_SIZE_LEFT         7
#define _NET_WM_MOVERESIZE_MOVE              8
#define _NET_WM_MOVERESIZE_SIZE_KEYBOARD     9
#define _NET_WM_MOVERESIZE_MOVE_KEYBOARD    10

void
_wnck_keyboard_move (Screen *screen,
                     Window  xwindow)
{
  XEvent xev;
  
  xev.xclient.type = ClientMessage;
  xev.xclient.serial = 0;
  xev.xclient.send_event = True;
  xev.xclient.display = gdk_display;
  xev.xclient.window = xwindow;
  xev.xclient.message_type = _wnck_atom_get ("_NET_WM_MOVERESIZE");
  xev.xclient.format = 32;
  xev.xclient.data.l[0] = 0; /* unused */
  xev.xclient.data.l[1] = 0; /* unused */
  xev.xclient.data.l[2] = _NET_WM_MOVERESIZE_MOVE_KEYBOARD;
  xev.xclient.data.l[3] = 0; /* unused */
  xev.xclient.data.l[4] = _wnck_get_client_type ();

  XSendEvent (gdk_display,
              RootWindowOfScreen (screen),
              False,
              SubstructureRedirectMask | SubstructureNotifyMask,
              &xev); 
}

void
_wnck_keyboard_size (Screen *screen,
                     Window  xwindow)
{
  XEvent xev;
  
  xev.xclient.type = ClientMessage;
  xev.xclient.serial = 0;
  xev.xclient.send_event = True;
  xev.xclient.display = gdk_display;
  xev.xclient.window = xwindow;
  xev.xclient.message_type = _wnck_atom_get ("_NET_WM_MOVERESIZE");
  xev.xclient.format = 32;
  xev.xclient.data.l[0] = 0; /* unused */
  xev.xclient.data.l[1] = 0; /* unused */
  xev.xclient.data.l[2] = _NET_WM_MOVERESIZE_SIZE_KEYBOARD;
  xev.xclient.data.l[3] = 0; /* unused */
  xev.xclient.data.l[4] = _wnck_get_client_type ();

  XSendEvent (gdk_display,
              RootWindowOfScreen (screen),
              False,
              SubstructureRedirectMask | SubstructureNotifyMask,
              &xev); 
}

void
_wnck_change_state (Screen  *screen,
		    Window   xwindow,
                    gboolean add,
                    Atom     state1,
                    Atom     state2)
{
  XEvent xev;

#define _NET_WM_STATE_REMOVE        0    /* remove/unset property */
#define _NET_WM_STATE_ADD           1    /* add/set property */
#define _NET_WM_STATE_TOGGLE        2    /* toggle property  */  
  
  xev.xclient.type = ClientMessage;
  xev.xclient.serial = 0;
  xev.xclient.send_event = True;
  xev.xclient.display = gdk_display;
  xev.xclient.window = xwindow;
  xev.xclient.message_type = _wnck_atom_get ("_NET_WM_STATE");
  xev.xclient.format = 32;
  xev.xclient.data.l[0] = add ? _NET_WM_STATE_ADD : _NET_WM_STATE_REMOVE;
  xev.xclient.data.l[1] = state1;
  xev.xclient.data.l[2] = state2;
  xev.xclient.data.l[3] = _wnck_get_client_type ();
  xev.xclient.data.l[4] = 0;

  XSendEvent (gdk_display,
	      RootWindowOfScreen (screen),
              False,
	      SubstructureRedirectMask | SubstructureNotifyMask,
	      &xev);
}

void
_wnck_change_workspace (Screen     *screen,
			Window      xwindow,
                        int         new_space)
{
  XEvent xev;
  
  xev.xclient.type = ClientMessage;
  xev.xclient.serial = 0;
  xev.xclient.send_event = True;
  xev.xclient.display = gdk_display;
  xev.xclient.window = xwindow;
  xev.xclient.message_type = _wnck_atom_get ("_NET_WM_DESKTOP");
  xev.xclient.format = 32;
  xev.xclient.data.l[0] = new_space;
  xev.xclient.data.l[1] = _wnck_get_client_type ();
  xev.xclient.data.l[2] = 0;
  xev.xclient.data.l[3] = 0;
  xev.xclient.data.l[4] = 0;

  XSendEvent (gdk_display,
	      RootWindowOfScreen (screen),
              False,
	      SubstructureRedirectMask | SubstructureNotifyMask,
	      &xev);
}

void
_wnck_activate (Screen *screen,
                Window  xwindow,
                Time    timestamp)
{
  XEvent xev;

  if (timestamp == 0)
    g_warning ("Received a timestamp of 0; window activation may not "
               "function properly.\n");
  
  xev.xclient.type = ClientMessage;
  xev.xclient.serial = 0;
  xev.xclient.send_event = True;
  xev.xclient.display = gdk_display;
  xev.xclient.window = xwindow;
  xev.xclient.message_type = _wnck_atom_get ("_NET_ACTIVE_WINDOW");
  xev.xclient.format = 32;
  xev.xclient.data.l[0] = _wnck_get_client_type ();
  xev.xclient.data.l[1] = timestamp;
  xev.xclient.data.l[2] = 0;
  xev.xclient.data.l[3] = 0;
  xev.xclient.data.l[4] = 0;

  XSendEvent (gdk_display,
	      RootWindowOfScreen (screen),
              False,
	      SubstructureRedirectMask | SubstructureNotifyMask,
	      &xev); 
}

void
_wnck_activate_workspace (Screen *screen,
                          int     new_active_space,
                          Time    timestamp)
{
  XEvent xev;
  
  xev.xclient.type = ClientMessage;
  xev.xclient.serial = 0;
  xev.xclient.send_event = True;
  xev.xclient.display = gdk_display;
  xev.xclient.window = RootWindowOfScreen (screen);
  xev.xclient.message_type = _wnck_atom_get ("_NET_CURRENT_DESKTOP");
  xev.xclient.format = 32;
  xev.xclient.data.l[0] = new_active_space;
  xev.xclient.data.l[1] = timestamp;
  xev.xclient.data.l[2] = 0;
  xev.xclient.data.l[3] = 0;
  xev.xclient.data.l[4] = 0;

  XSendEvent (gdk_display,
	      RootWindowOfScreen (screen),
              False,
	      SubstructureRedirectMask | SubstructureNotifyMask,
	      &xev);
}

void
_wnck_change_viewport (Screen *screen,
		       int     x,
		       int     y)
{
  XEvent xev;
  
  xev.xclient.type = ClientMessage;
  xev.xclient.serial = 0;
  xev.xclient.send_event = True;
  xev.xclient.display = gdk_display;
  xev.xclient.window = RootWindowOfScreen (screen);
  xev.xclient.message_type = _wnck_atom_get ("_NET_DESKTOP_VIEWPORT");
  xev.xclient.format = 32;
  xev.xclient.data.l[0] = x;
  xev.xclient.data.l[1] = y;
  xev.xclient.data.l[2] = 0;
  xev.xclient.data.l[3] = 0;
  xev.xclient.data.l[4] = 0;

  XSendEvent (gdk_display,
	      RootWindowOfScreen (screen),
              False,
	      SubstructureRedirectMask | SubstructureNotifyMask,
	      &xev);
}

void
_wnck_toggle_showing_desktop (Screen  *screen,
                              gboolean show)
{
  XEvent xev;
  
  xev.xclient.type = ClientMessage;
  xev.xclient.serial = 0;
  xev.xclient.send_event = True;
  xev.xclient.display = DisplayOfScreen (screen);
  xev.xclient.window = RootWindowOfScreen (screen);
  xev.xclient.message_type = _wnck_atom_get ("_NET_SHOWING_DESKTOP");
  xev.xclient.format = 32;
  xev.xclient.data.l[0] = show != FALSE;
  xev.xclient.data.l[1] = 0;
  xev.xclient.data.l[2] = 0;
  xev.xclient.data.l[3] = 0;
  xev.xclient.data.l[4] = 0;

  XSendEvent (DisplayOfScreen (screen),
	      RootWindowOfScreen (screen),
              False,
	      SubstructureRedirectMask | SubstructureNotifyMask,
	      &xev);
}

char*
_wnck_get_session_id (Window xwindow)
{
  Window client_leader;
  
  client_leader = None;
  _wnck_get_window (xwindow,
                    _wnck_atom_get ("WM_CLIENT_LEADER"),
                    &client_leader);

  if (client_leader == None)
    return NULL;

  return _wnck_get_string_property_latin1 (client_leader,
                                           _wnck_atom_get ("SM_CLIENT_ID"));
}

int
_wnck_get_pid (Window xwindow)
{
  int val;

  if (!_wnck_get_cardinal (xwindow,
                           _wnck_atom_get ("_NET_WM_PID"),
                           &val))
    return 0;
  else
    return val;
}

char*
_wnck_get_name (Window xwindow)
{
  char *name;
  
  name = _wnck_get_utf8_property (xwindow,
                                  _wnck_atom_get ("_NET_WM_VISIBLE_NAME"));

  if (name == NULL)
    name = _wnck_get_utf8_property (xwindow,
                                    _wnck_atom_get ("_NET_WM_NAME"));

  if (name == NULL)
    name = _wnck_get_text_property (xwindow,
                                    XA_WM_NAME);

  return name;
}

static char*
latin1_to_utf8 (const char *latin1)
{
  GString *str;
  const char *p;
  
  str = g_string_new (NULL);

  p = latin1;
  while (*p)
    {
      g_string_append_unichar (str, (gunichar) *p);
      ++p;
    }

  return g_string_free (str, FALSE);
}

char*
_wnck_get_res_class_utf8 (Window xwindow)
{
  char *res_class;

  _wnck_get_wmclass (xwindow, &res_class, NULL);

  return res_class;
}

void
_wnck_get_wmclass (Window xwindow,
                   char **res_class,
                   char **res_name)
{
  XClassHint ch;
  char *retval;
  
  _wnck_error_trap_push ();

  ch.res_name = NULL;
  ch.res_class = NULL;

  XGetClassHint (gdk_display, xwindow,
                 &ch);

  _wnck_error_trap_pop ();
  
  retval = NULL;

  if (res_class)
    *res_class = NULL;

  if (res_name)
    *res_name = NULL;
  
  if (ch.res_name)
    {
      if (res_name)
        *res_name = latin1_to_utf8 (ch.res_name);
      
      XFree (ch.res_name);
    }

  if (ch.res_class)
    {
      if (res_class)
        *res_class = latin1_to_utf8 (ch.res_class);
      
      XFree (ch.res_class);
    }
}

void
_wnck_select_input (Window xwindow,
                    int    mask)
{
  GdkWindow *gdkwindow;
  
  gdkwindow = gdk_xid_table_lookup (xwindow);

  _wnck_error_trap_push ();
  if (gdkwindow)
    {
      /* Avoid breaking GDK's setup,
       * this somewhat relies on people setting
       * event masks right after realization
       * and not changing them again
       */
      XWindowAttributes attrs;
      XGetWindowAttributes (gdk_display, xwindow, &attrs);
      mask |= attrs.your_event_mask;
    }
  
  XSelectInput (gdk_display, xwindow, mask);
  _wnck_error_trap_pop ();
}
  
/* The icon-reading code is copied
 * from metacity, please sync bugfixes
 */
static gboolean
find_largest_sizes (gulong *data,
                    gulong  nitems,
                    int    *width,
                    int    *height)
{
  *width = 0;
  *height = 0;
  
  while (nitems > 0)
    {
      int w, h;
      gboolean replace;

      replace = FALSE;
      
      if (nitems < 3)
        return FALSE; /* no space for w, h */
      
      w = data[0];
      h = data[1];
      
      if (nitems < ((w * h) + 2))
        return FALSE; /* not enough data */

      *width = MAX (w, *width);
      *height = MAX (h, *height);
      
      data += (w * h) + 2;
      nitems -= (w * h) + 2;
    }

  return TRUE;
}

static gboolean
find_best_size (gulong  *data,
                gulong   nitems,
                int      ideal_width,
                int      ideal_height,
                int     *width,
                int     *height,
                gulong **start)
{
  int best_w;
  int best_h;
  gulong *best_start;
  int max_width, max_height;
  
  *width = 0;
  *height = 0;
  *start = NULL;

  if (!find_largest_sizes (data, nitems, &max_width, &max_height))
    return FALSE;

  if (ideal_width < 0)
    ideal_width = max_width;
  if (ideal_height < 0)
    ideal_height = max_height;
  
  best_w = 0;
  best_h = 0;
  best_start = NULL;
  
  while (nitems > 0)
    {
      int w, h;
      gboolean replace;

      replace = FALSE;
      
      if (nitems < 3)
        return FALSE; /* no space for w, h */
      
      w = data[0];
      h = data[1];
      
      if (nitems < ((w * h) + 2))
        break; /* not enough data */

      if (best_start == NULL)
        {
          replace = TRUE;
        }
      else
        {
          /* work with averages */
          const int ideal_size = (ideal_width + ideal_height) / 2;
          int best_size = (best_w + best_h) / 2;
          int this_size = (w + h) / 2;
          
          /* larger than desired is always better than smaller */
          if (best_size < ideal_size &&
              this_size >= ideal_size)
            replace = TRUE;
          /* if we have too small, pick anything bigger */
          else if (best_size < ideal_size &&
                   this_size > best_size)
            replace = TRUE;
          /* if we have too large, pick anything smaller
           * but still >= the ideal
           */
          else if (best_size > ideal_size &&
                   this_size >= ideal_size &&
                   this_size < best_size)
            replace = TRUE;
        }

      if (replace)
        {
          best_start = data + 2;
          best_w = w;
          best_h = h;
        }

      data += (w * h) + 2;
      nitems -= (w * h) + 2;
    }

  if (best_start)
    {
      *start = best_start;
      *width = best_w;
      *height = best_h;
      return TRUE;
    }
  else
    return FALSE;
}

static void
argbdata_to_pixdata (gulong *argb_data, int len, guchar **pixdata)
{
  guchar *p;
  int i;
  
  *pixdata = g_new (guchar, len * 4);
  p = *pixdata;

  /* One could speed this up a lot. */
  i = 0;
  while (i < len)
    {
      guint argb;
      guint rgba;
      
      argb = argb_data[i];
      rgba = (argb << 8) | (argb >> 24);
      
      *p = rgba >> 24;
      ++p;
      *p = (rgba >> 16) & 0xff;
      ++p;
      *p = (rgba >> 8) & 0xff;
      ++p;
      *p = rgba & 0xff;
      ++p;
      
      ++i;
    }
}

static gboolean
read_rgb_icon (Window         xwindow,
               int            ideal_width,
               int            ideal_height,
               int            ideal_mini_width,
               int            ideal_mini_height,
               int           *width,
               int           *height,
               guchar       **pixdata,
               int           *mini_width,
               int           *mini_height,
               guchar       **mini_pixdata)
{
  Atom type;
  int format;
  gulong nitems;
  gulong bytes_after;
  int result, err;
  gulong *data;
  gulong *best;
  int w, h;
  gulong *best_mini;
  int mini_w, mini_h;
  
  _wnck_error_trap_push ();
  type = None;
  data = NULL;
  result = XGetWindowProperty (gdk_display,
			       xwindow,
			       _wnck_atom_get ("_NET_WM_ICON"),
			       0, G_MAXLONG,
			       False, XA_CARDINAL, &type, &format, &nitems,
			       &bytes_after, (void*)&data);
  
  err = _wnck_error_trap_pop ();
  
  if (err != Success ||
      result != Success)
    return FALSE;

  if (type != XA_CARDINAL)
    {
      XFree (data);
      return FALSE;
    }
  
  if (!find_best_size (data, nitems,
                       ideal_width, ideal_height,
                       &w, &h, &best))
    {
      XFree (data);
      return FALSE;
    }

  if (!find_best_size (data, nitems,
                       ideal_mini_width, ideal_mini_height,
                       &mini_w, &mini_h, &best_mini))
    {
      XFree (data);
      return FALSE;
    }
  
  *width = w;
  *height = h;

  *mini_width = mini_w;
  *mini_height = mini_h;

  argbdata_to_pixdata (best, w * h, pixdata);
  argbdata_to_pixdata (best_mini, mini_w * mini_h, mini_pixdata);

  XFree (data);
  
  return TRUE;
}

static void
free_pixels (guchar *pixels, gpointer data)
{
  g_free (pixels);
}

static void
get_pixmap_geometry (Pixmap       pixmap,
                     int         *w,
                     int         *h,
                     int         *d)
{
  Window root_ignored;
  int x_ignored, y_ignored;
  guint width, height;
  guint border_width_ignored;
  guint depth;

  if (w)
    *w = 1;
  if (h)
    *h = 1;
  if (d)
    *d = 1;
  
  XGetGeometry (gdk_display,
                pixmap, &root_ignored, &x_ignored, &y_ignored,
                &width, &height, &border_width_ignored, &depth);

  if (w)
    *w = width;
  if (h)
    *h = height;
  if (d)
    *d = depth;
}

static GdkPixbuf*
apply_mask (GdkPixbuf *pixbuf,
            GdkPixbuf *mask)
{
  int w, h;
  int i, j;
  GdkPixbuf *with_alpha;
  guchar *src;
  guchar *dest;
  int src_stride;
  int dest_stride;
  
  w = MIN (gdk_pixbuf_get_width (mask), gdk_pixbuf_get_width (pixbuf));
  h = MIN (gdk_pixbuf_get_height (mask), gdk_pixbuf_get_height (pixbuf));
  
  with_alpha = gdk_pixbuf_add_alpha (pixbuf, FALSE, 0, 0, 0);

  dest = gdk_pixbuf_get_pixels (with_alpha);
  src = gdk_pixbuf_get_pixels (mask);

  dest_stride = gdk_pixbuf_get_rowstride (with_alpha);
  src_stride = gdk_pixbuf_get_rowstride (mask);
  
  i = 0;
  while (i < h)
    {
      j = 0;
      while (j < w)
        {
          guchar *s = src + i * src_stride + j * 3;
          guchar *d = dest + i * dest_stride + j * 4;
          
          /* s[0] == s[1] == s[2], they are 255 if the bit was set, 0
           * otherwise
           */
          if (s[0] == 0)
            d[3] = 0;   /* transparent */
          else
            d[3] = 255; /* opaque */
          
          ++j;
        }
      
      ++i;
    }

  return with_alpha;
}

static GdkColormap*
get_cmap (GdkPixmap *pixmap)
{
  GdkColormap *cmap;

  cmap = gdk_drawable_get_colormap (pixmap);
  if (cmap)
    g_object_ref (G_OBJECT (cmap));

  if (cmap == NULL)
    {
      if (gdk_drawable_get_depth (pixmap) == 1)
        {
          /* try null cmap */
          cmap = NULL;
        }
      else
        {
          /* Try system cmap */
          GdkScreen *screen = gdk_drawable_get_screen (GDK_DRAWABLE (pixmap));
          cmap = gdk_screen_get_system_colormap (screen);
          g_object_ref (G_OBJECT (cmap));
        }
    }

  /* Be sure we aren't going to blow up due to visual mismatch */
  if (cmap &&
      (gdk_colormap_get_visual (cmap)->depth !=
       gdk_drawable_get_depth (pixmap)))
    cmap = NULL;
  
  return cmap;
}

GdkPixbuf*
_wnck_gdk_pixbuf_get_from_pixmap (GdkPixbuf   *dest,
                                  Pixmap       xpixmap,
                                  int          src_x,
                                  int          src_y,
                                  int          dest_x,
                                  int          dest_y,
                                  int          width,
                                  int          height)
{
  GdkDrawable *drawable;
  GdkPixbuf *retval;
  GdkColormap *cmap;
  
  retval = NULL;
  cmap = NULL;
  
  drawable = gdk_xid_table_lookup (xpixmap);

  if (drawable)
    g_object_ref (G_OBJECT (drawable));
  else
    drawable = gdk_pixmap_foreign_new (xpixmap);

  if (drawable)
    {
			cmap = get_cmap (drawable);

			/* GDK is supposed to do this but doesn't in GTK 2.0.2,
			 * fixed in 2.0.3
			 */
			if (width < 0)
				gdk_drawable_get_size (drawable, &width, NULL);
			if (height < 0)
				gdk_drawable_get_size (drawable, NULL, &height);
			
			retval = gdk_pixbuf_get_from_drawable (dest,
																						 drawable,
																						 cmap,
																						 src_x, src_y,
																						 dest_x, dest_y,
																						 width, height);
    }

  if (cmap)
    g_object_unref (G_OBJECT (cmap));
  if (drawable)
    g_object_unref (G_OBJECT (drawable));

  return retval;
}

static gboolean
try_pixmap_and_mask (Pixmap      src_pixmap,
                     Pixmap      src_mask,
                     GdkPixbuf **iconp,
                     int         ideal_width,
                     int         ideal_height,
                     GdkPixbuf **mini_iconp,
                     int         ideal_mini_width,
                     int         ideal_mini_height)
{
  GdkPixbuf *unscaled = NULL;
  GdkPixbuf *mask = NULL;
  int w, h;

  if (src_pixmap == None)
    return FALSE;
      
  _wnck_error_trap_push ();

  get_pixmap_geometry (src_pixmap, &w, &h, NULL);
      
  unscaled = _wnck_gdk_pixbuf_get_from_pixmap (NULL,
                                               src_pixmap,
                                               0, 0, 0, 0,
                                               w, h);

  if (unscaled && src_mask != None)
    {
      get_pixmap_geometry (src_mask, &w, &h, NULL);
      mask = _wnck_gdk_pixbuf_get_from_pixmap (NULL,
                                               src_mask,
                                               0, 0, 0, 0,
                                               w, h);
    }
  
  _wnck_error_trap_pop ();

  if (mask)
    {
      GdkPixbuf *masked;
      
      masked = apply_mask (unscaled, mask);
      g_object_unref (G_OBJECT (unscaled));
      unscaled = masked;

      g_object_unref (G_OBJECT (mask));
      mask = NULL;
    }
  
  if (unscaled)
    {
      *iconp =
        gdk_pixbuf_scale_simple (unscaled,
                                 ideal_width > 0 ? ideal_width :
                                 gdk_pixbuf_get_width (unscaled),
                                 ideal_height > 0 ? ideal_height :
                                 gdk_pixbuf_get_height (unscaled),
                                 GDK_INTERP_BILINEAR);
      *mini_iconp =
        gdk_pixbuf_scale_simple (unscaled,
                                 ideal_mini_width > 0 ? ideal_mini_width :
                                 gdk_pixbuf_get_width (unscaled),
                                 ideal_mini_height > 0 ? ideal_mini_height :
                                 gdk_pixbuf_get_height (unscaled),
                                 GDK_INTERP_BILINEAR);      
      
      g_object_unref (G_OBJECT (unscaled));
      return TRUE;
    }
  else
    return FALSE;
}

static void
get_kwm_win_icon (Window  xwindow,
                  Pixmap *pixmap,
                  Pixmap *mask)
{
  Atom type;
  int format;
  gulong nitems;
  gulong bytes_after;
  Pixmap *icons;
  int err, result;

  *pixmap = None;
  *mask = None;
  
  _wnck_error_trap_push ();
  icons = NULL;
  result = XGetWindowProperty (gdk_display, xwindow,
			       _wnck_atom_get ("KWM_WIN_ICON"),
			       0, G_MAXLONG,
			       False,
			       _wnck_atom_get ("KWM_WIN_ICON"),
			       &type, &format, &nitems,
			       &bytes_after, (void*)&icons);  

  err = _wnck_error_trap_pop ();
  if (err != Success ||
      result != Success)
    return;
  
  if (type != _wnck_atom_get ("KWM_WIN_ICON"))
    {
      XFree (icons);
      return;
    }
  
  *pixmap = icons[0];
  *mask = icons[1];
  
  XFree (icons);

  return;
}

typedef enum
{
  /* These MUST be in ascending order of preference;
   * i.e. if we get _NET_WM_ICON and already have
   * WM_HINTS, we prefer _NET_WM_ICON
   */
  USING_NO_ICON,
  USING_FALLBACK_ICON,
  USING_KWM_WIN_ICON,
  USING_WM_HINTS,
  USING_NET_WM_ICON
} IconOrigin;

struct _WnckIconCache
{
  IconOrigin origin;
  Pixmap prev_pixmap;
  Pixmap prev_mask;
  GdkPixbuf *icon;
  GdkPixbuf *mini_icon;
  int ideal_width;
  int ideal_height;
  int ideal_mini_width;
  int ideal_mini_height;
  guint want_fallback : 1;
  /* TRUE if these props have changed */
  guint wm_hints_dirty : 1;
  guint kwm_win_icon_dirty : 1;
  guint net_wm_icon_dirty : 1;  
};

WnckIconCache*
_wnck_icon_cache_new (void)
{
  WnckIconCache *icon_cache;

  icon_cache = g_new0 (WnckIconCache, 1);

  icon_cache->origin = USING_NO_ICON;
  icon_cache->prev_pixmap = None;
  icon_cache->icon = NULL;
  icon_cache->mini_icon = NULL;
  icon_cache->ideal_width = -1; /* won't be a legit width */
  icon_cache->ideal_height = -1;
  icon_cache->ideal_mini_width = -1;
  icon_cache->ideal_mini_height = -1;
  icon_cache->want_fallback = TRUE;
  icon_cache->wm_hints_dirty = TRUE;
  icon_cache->kwm_win_icon_dirty = TRUE;
  icon_cache->net_wm_icon_dirty = TRUE;
  
  return icon_cache;
}

static void
clear_icon_cache (WnckIconCache *icon_cache,
                  gboolean       dirty_all)
{
  if (icon_cache->icon)
    g_object_unref (G_OBJECT (icon_cache->icon));
  icon_cache->icon = NULL;
  
  if (icon_cache->mini_icon)
    g_object_unref (G_OBJECT (icon_cache->mini_icon));
  icon_cache->mini_icon = NULL;

  icon_cache->origin = USING_NO_ICON;

  if (dirty_all)
    {
      icon_cache->wm_hints_dirty = TRUE;
      icon_cache->kwm_win_icon_dirty = TRUE;
      icon_cache->net_wm_icon_dirty = TRUE;
    }
}

void
_wnck_icon_cache_free (WnckIconCache *icon_cache)
{
  clear_icon_cache (icon_cache, FALSE);
  
  g_free (icon_cache);
}

void
_wnck_icon_cache_property_changed (WnckIconCache *icon_cache,
                                   Atom           atom)
{  
  if (atom == _wnck_atom_get ("_NET_WM_ICON"))
    icon_cache->net_wm_icon_dirty = TRUE;
  else if (atom == _wnck_atom_get ("KWM_WIN_ICON"))
    icon_cache->kwm_win_icon_dirty = TRUE;
  else if (atom == _wnck_atom_get ("WM_HINTS"))
    icon_cache->wm_hints_dirty = TRUE;
}

gboolean
_wnck_icon_cache_get_icon_invalidated (WnckIconCache *icon_cache)
{
  if (icon_cache->origin <= USING_KWM_WIN_ICON &&
      icon_cache->kwm_win_icon_dirty)
    return TRUE;
  else if (icon_cache->origin <= USING_WM_HINTS &&
           icon_cache->wm_hints_dirty)
    return TRUE;
  else if (icon_cache->origin <= USING_NET_WM_ICON &&
           icon_cache->net_wm_icon_dirty)
    return TRUE;
  else if (icon_cache->origin < USING_FALLBACK_ICON &&
           icon_cache->want_fallback)
    return TRUE;
  else if (icon_cache->origin == USING_NO_ICON)
    return TRUE;
  else if (icon_cache->origin == USING_FALLBACK_ICON &&
           !icon_cache->want_fallback)
    return TRUE;
  else
    return FALSE;
}

void
_wnck_icon_cache_set_want_fallback (WnckIconCache *icon_cache,
                                    gboolean       setting)
{
  icon_cache->want_fallback = setting;
}

gboolean
_wnck_icon_cache_get_is_fallback (WnckIconCache *icon_cache)
{
  return icon_cache->origin == USING_FALLBACK_ICON;
}

static GdkPixbuf*
scaled_from_pixdata (guchar *pixdata,
                     int     w,
                     int     h,
                     int     new_w,
                     int     new_h)
{
  GdkPixbuf *src;
  GdkPixbuf *dest;
  
  src = gdk_pixbuf_new_from_data (pixdata,
                                  GDK_COLORSPACE_RGB,
                                  TRUE,
                                  8,
                                  w, h, w * 4,
                                  free_pixels, 
                                  NULL);

  if (src == NULL)
    return NULL;

  if (w != h)
    {
      GdkPixbuf *tmp;
      int size;
      
      size = MAX (w, h);
      
      tmp = gdk_pixbuf_new (GDK_COLORSPACE_RGB, TRUE, 8, size, size);
      
      if (tmp != NULL)
	{
	  gdk_pixbuf_fill (tmp, 0);
	  gdk_pixbuf_copy_area (src, 0, 0, w, h,
				tmp,
				(size - w) / 2, (size - h) / 2);
	  
	  if (src)
	  	g_object_unref (src);
	  src = tmp;
	}
    }
  
  if (w != new_w || h != new_h)
    {
      dest = gdk_pixbuf_scale_simple (src, new_w, new_h, GDK_INTERP_BILINEAR);
      
      g_object_unref (G_OBJECT (src));
    }
  else
    {
      dest = src;
    }

  return dest;
}


void
_wnck_get_fallback_icons (GdkPixbuf **iconp,
                          int         ideal_width,
                          int         ideal_height,
                          GdkPixbuf **mini_iconp,
                          int         ideal_mini_width,
                          int         ideal_mini_height)
{

}


void
_wnck_get_window_geometry (Screen *screen,
			   Window  xwindow,
                           int    *xp,
                           int    *yp,
                           int    *widthp,
                           int    *heightp)
{
  int x, y;
  unsigned int width, height, bw, depth;
  Window root_window;

  width = 1;
  height = 1;
  
  _wnck_error_trap_push ();

  XGetGeometry (gdk_display,
                xwindow,
                &root_window,
                &x, &y, &width, &height, &bw, &depth);
  
  _wnck_error_trap_pop ();

  _wnck_get_window_position (screen, xwindow, xp, yp);

  if (widthp)
    *widthp = width;
  if (heightp)
    *heightp = height;
}

void _wnck_set_window_geometry (Screen *screen,
                                Window  xwindow,
                                int     gravity_and_flags,
                                int     x,
                                int     y,
                                int     width,
                                int     height)
{
  XEvent xev;

  xev.xclient.type = ClientMessage;
  xev.xclient.serial = 0;
  xev.xclient.send_event = True;
  xev.xclient.display = gdk_display;
  xev.xclient.window = xwindow;
  xev.xclient.message_type = _wnck_atom_get ("_NET_MOVERESIZE_WINDOW");
  xev.xclient.format = 32;
  xev.xclient.data.l[0] = gravity_and_flags;
  xev.xclient.data.l[1] = x;
  xev.xclient.data.l[2] = y;
  xev.xclient.data.l[3] = width;
  xev.xclient.data.l[4] = height;

  XSendEvent (gdk_display,
              RootWindowOfScreen (screen),
              False,
              SubstructureRedirectMask | SubstructureNotifyMask,
              &xev);
}

void
_wnck_get_window_position (Screen *screen,
			   Window  xwindow,
                           int    *xp,
                           int    *yp)
{
  int x, y;
  Window child;

  x = 0;
  y = 0;
  
  _wnck_error_trap_push ();
  XTranslateCoordinates (gdk_display,
                         xwindow,
			 RootWindowOfScreen (screen),
                         0, 0,
                         &x, &y, &child);
  _wnck_error_trap_pop ();

  if (xp)
    *xp = x;
  if (yp)
    *yp = y;
}

void
_wnck_set_icon_geometry  (Window xwindow,
			  int    x,
			  int    y,
			  int    width,
			  int    height)
{
  gulong data[4];

  data[0] = x;
  data[1] = y;
  data[2] = width;
  data[3] = height;
  
  _wnck_error_trap_push ();

  XChangeProperty (gdk_display,
		   xwindow,
		   _wnck_atom_get ("_NET_WM_ICON_GEOMETRY"),
		   XA_CARDINAL, 32, PropModeReplace,
		   (guchar *)&data, 4);

  _wnck_error_trap_pop ();
}

void
_wnck_set_dock_type_hint (Window xwindow)
{
  Atom atom;

  atom = _wnck_atom_get ("_NET_WM_WINDOW_TYPE_DOCK");
  
  _wnck_error_trap_push ();

  XChangeProperty (gdk_display,
                   xwindow, 
                   _wnck_atom_get ("_NET_WM_WINDOW_TYPE"),
		   XA_ATOM, 32, PropModeReplace,
		   (guchar *)&atom, 1);

  _wnck_error_trap_pop ();
}

/* orientation of pager */
#define _NET_WM_ORIENTATION_HORZ 0
#define _NET_WM_ORIENTATION_VERT 1

/* starting corner for counting spaces */
#define _NET_WM_TOPLEFT     0
#define _NET_WM_TOPRIGHT    1
#define _NET_WM_BOTTOMRIGHT 2
#define _NET_WM_BOTTOMLEFT  3

void
_wnck_set_desktop_layout (Screen *xscreen,
                          int     rows,
                          int     columns)
{
  gulong data[4];

  /* FIXME: hack, hack, hack so as not
   * to have to add a orientation param
   * to wnck_screen_try_set_workspace_layout.
   *
   * Remove this crack asap.
   */
  g_assert ((rows == 0) || (columns == 0));

  data[0] = (columns == 0) ? _NET_WM_ORIENTATION_HORZ : _NET_WM_ORIENTATION_VERT;
  data[1] = columns;
  data[2] = rows;
  data[3] = _NET_WM_TOPLEFT;
  
  _wnck_error_trap_push ();

  XChangeProperty (gdk_display,
                   RootWindowOfScreen (xscreen),
		   _wnck_atom_get ("_NET_DESKTOP_LAYOUT"),
		   XA_CARDINAL, 32, PropModeReplace,
		   (guchar *)&data, 4);

  _wnck_error_trap_pop ();
}

typedef struct 
{
  Window window;
  Atom timestamp_prop_atom;
} TimeStampInfo;

static Bool
timestamp_predicate (Display *display,
		     XEvent  *xevent,
		     XPointer arg)
{
  TimeStampInfo *info = (TimeStampInfo *)arg;

  if (xevent->type == PropertyNotify &&
      xevent->xproperty.window == info->window &&
      xevent->xproperty.atom == info->timestamp_prop_atom)
    return True;

  return False;
}

/**
 * get_server_time:
 * @display: display from which to get the time
 * @window: a #Window, used for communication with the server.
 *          The window must have PropertyChangeMask in its
 *          events mask or a hang will result.
 * 
 * Routine to get the current X server time stamp. 
 * 
 * Return value: the time stamp.
 **/
static Time
get_server_time (Display *display,
		 Window   window)
{
  unsigned char c = 'a';
  XEvent xevent;
  TimeStampInfo info;

  info.timestamp_prop_atom = XInternAtom  (display, "_TIMESTAMP_PROP", False);
  info.window = window;

  XChangeProperty (display, window,
		   info.timestamp_prop_atom, info.timestamp_prop_atom,
		   8, PropModeReplace, &c, 1);

  XIfEvent (display, &xevent,
	    timestamp_predicate, (XPointer)&info);

  return xevent.xproperty.time;
}

typedef struct
{
  int screen_number;
  int token;
  Window window;
  Atom selection_atom;
  Atom manager_atom;
} LayoutManager;

static GSList *layout_managers = NULL;
static int next_token = 1;

int
_wnck_try_desktop_layout_manager (Screen *xscreen,
                                  int     current_token)
{
  GSList *tmp;
  int number;
  Time timestamp;
  XClientMessageEvent xev;  
  char buffer[256];
  LayoutManager *lm;
  
  number = XScreenNumberOfScreen (xscreen);
  
  tmp = layout_managers;
  while (tmp != NULL)
    {
      lm = tmp->data;

      if (number == lm->screen_number)
        {
          if (current_token == lm->token)
            return current_token; /* we still have the selection */
          else
            return WNCK_NO_MANAGER_TOKEN; /* someone else has it */
        }
      
      tmp = tmp->next;
    }
  
  /* No one in-process has the selection at the moment */

  lm = g_new0 (LayoutManager, 1);

  lm->screen_number = number;
  lm->token = next_token;
  ++next_token;

  sprintf (buffer, "_NET_DESKTOP_LAYOUT_S%d", number);
  lm->selection_atom = XInternAtom (gdk_display, buffer, False);
  lm->manager_atom = XInternAtom (gdk_display, "MANAGER", False);

  lm->window = XCreateSimpleWindow (gdk_display,
                                    RootWindowOfScreen (xscreen),
                                    0, 0, 10, 10, 0,
                                    WhitePixel (gdk_display, number),
                                    WhitePixel (gdk_display, number));

  XSelectInput (gdk_display, lm->window, PropertyChangeMask);
  timestamp = get_server_time (gdk_display, lm->window);

  XSetSelectionOwner (gdk_display, lm->selection_atom,
		      lm->window, timestamp);

  /* Check to see if we managed to claim the selection. */

  if (XGetSelectionOwner (gdk_display, lm->selection_atom) !=
      lm->window)
    {
      g_free (lm);
      return WNCK_NO_MANAGER_TOKEN;
    }
  
  xev.type = ClientMessage;
  xev.window = RootWindow (gdk_display, number);
  xev.message_type = lm->manager_atom;
  xev.format = 32;
  xev.data.l[0] = timestamp;
  xev.data.l[1] = lm->selection_atom;
  xev.data.l[2] = lm->window;
  xev.data.l[3] = 0;	/* manager specific data */
  xev.data.l[4] = 0;	/* manager specific data */
  
  XSendEvent (gdk_display, RootWindow (gdk_display, number),
              False, StructureNotifyMask, (XEvent *)&xev);

  layout_managers = g_slist_prepend (layout_managers,
                                     lm);
  
  return lm->token;
}

void
_wnck_release_desktop_layout_manager (Screen *xscreen,
                                      int     current_token)
{
  GSList *tmp;
  int number;
  LayoutManager *lm;
  
  number = XScreenNumberOfScreen (xscreen);
  
  tmp = layout_managers;
  while (tmp != NULL)
    {
      lm = tmp->data;

      if (number == lm->screen_number)
        {
          if (current_token == lm->token)
            {
              XDestroyWindow (gdk_display,
                              lm->window);

              g_free (lm);
              layout_managers = g_slist_remove (layout_managers, lm);
              
              return;
            }
        }
      
      tmp = tmp->next;
    }
}

gboolean
_wnck_desktop_layout_manager_process_event (XEvent *xev)
{
  GSList *tmp;
  LayoutManager *lm;

  if (xev->type != SelectionClear)
    return FALSE;
  
  tmp = layout_managers;
  while (tmp != NULL)
    {
      lm = tmp->data;

      if (xev->xany.window == lm->window &&
          xev->xselectionclear.selection == lm->selection_atom)
        {
          XDestroyWindow (gdk_display,
                          lm->window);
          
          g_free (lm);
          layout_managers = g_slist_remove (layout_managers, lm);
          
          return TRUE;
        }
      
      tmp = tmp->next;
    }

  return FALSE;
}

gboolean
_wnck_read_icons_ (Window         xwindow,
                  GtkWidget *icon_cache,
                  GdkPixbuf    **iconp,
                  int            ideal_width,
                  int            ideal_height,
                  GdkPixbuf    **mini_iconp,
                  int            ideal_mini_width,
                  int            ideal_mini_height)
{
  guchar *pixdata;     
  int w, h;
  guchar *mini_pixdata;
  int mini_w, mini_h;
  Pixmap pixmap;
  Pixmap mask;
  XWMHints *hints;

 
  
  *iconp = NULL;
  *mini_iconp = NULL;
  
  pixdata = NULL;


   
   if (read_rgb_icon (xwindow,
                         ideal_width, ideal_height,
                         ideal_mini_width, ideal_mini_height,
                         &w, &h, &pixdata,
                         &mini_w, &mini_h, &mini_pixdata))
        {
          *iconp = scaled_from_pixdata (pixdata, w, h, ideal_width, ideal_height);
          
          *mini_iconp = scaled_from_pixdata (mini_pixdata, mini_w, mini_h,
                                            ideal_mini_width, ideal_mini_height);

          return TRUE;
        }
   

  _wnck_error_trap_push ();
      hints = XGetWMHints (gdk_display, xwindow);
      _wnck_error_trap_pop ();
      pixmap = None;
      mask = None;
      if (hints)
        {
          if (hints->flags & IconPixmapHint)
            pixmap = hints->icon_pixmap;
          if (hints->flags & IconMaskHint)
            mask = hints->icon_mask;

          XFree (hints);
          hints = NULL;
        }


      if (try_pixmap_and_mask (pixmap, mask,
                                   iconp, ideal_width, ideal_height,
                                   mini_iconp, ideal_mini_width, ideal_mini_height))
            {
              return TRUE;
            }
    
  get_kwm_win_icon (xwindow, &pixmap, &mask);

  if (try_pixmap_and_mask (pixmap, mask,
                                   iconp, ideal_width, ideal_height,
                                   mini_iconp, ideal_mini_width, ideal_mini_height))
            {
              return TRUE;
            }
      


  return FALSE;
}


enum {
	STRUT_LEFT = 0,
	STRUT_RIGHT = 1,
	STRUT_TOP = 2,
	STRUT_BOTTOM = 3,
	STRUT_LEFT_START = 4,
	STRUT_LEFT_END = 5,
	STRUT_RIGHT_START = 6,
	STRUT_RIGHT_END = 7,
	STRUT_TOP_START = 8,
	STRUT_TOP_END = 9,
	STRUT_BOTTOM_START = 10,
	STRUT_BOTTOM_END = 11
};

static Atom net_wm_strut              = 0;
static Atom net_wm_strut_partial      = 0;

void
xutils_set_strut (GdkWindow        *gdk_window,
			guint32           strut,
			guint32           strut_start,
			guint32           strut_end)
 {
	Display *display;
	Window   window;
	gulong   struts [12] = { 0, };

	if (!GDK_IS_DRAWABLE (gdk_window))
		return;
	if (!GDK_IS_WINDOW (gdk_window))
		return;

	display = GDK_WINDOW_XDISPLAY (gdk_window);
	window  = GDK_WINDOW_XWINDOW (gdk_window);

	if (net_wm_strut == 0)
		net_wm_strut = XInternAtom (display, "_NET_WM_STRUT", False);
	if (net_wm_strut_partial == 0)
		net_wm_strut_partial = XInternAtom (display, "_NET_WM_STRUT_PARTIAL", False);

	struts [STRUT_BOTTOM] = strut;
	struts [STRUT_BOTTOM_START] = strut_start;
	struts [STRUT_BOTTOM_END] = strut_end;
	
	gdk_error_trap_push ();
	XChangeProperty (display, window, net_wm_strut,
			 XA_CARDINAL, 32, PropModeReplace,
			 (guchar *) &struts, 4);
	XChangeProperty (display, window, net_wm_strut_partial,
			 XA_CARDINAL, 32, PropModeReplace,
			 (guchar *) &struts, 12);
	gdk_error_trap_pop ();
}
