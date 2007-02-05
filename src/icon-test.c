#define WNCK_I_KNOW_THIS_IS_UNSTABLE 1
#include <libwnck/libwnck.h>
#include <gtk/gtk.h>
#include <string.h>


#include "xutils.h"

static GtkWidget *global_tree_view;
static GtkTreeModel *global_tree_model;
static guint refill_idle;

static gboolean use_wnck = FALSE;

static void active_window_changed_callback    (WnckScreen      *screen,
                                               gpointer         data);
static void active_workspace_changed_callback (WnckScreen      *screen,
                                               gpointer         data);
static void window_stacking_changed_callback  (WnckScreen      *screen,
                                               gpointer         data);
static void window_opened_callback            (WnckScreen      *screen,
                                               WnckWindow      *window,
                                               gpointer         data);
static void window_closed_callback            (WnckScreen      *screen,
                                               WnckWindow      *window,
                                               gpointer         data);
static void workspace_created_callback        (WnckScreen      *screen,
                                               WnckWorkspace   *space,
                                               gpointer         data);
static void workspace_destroyed_callback      (WnckScreen      *screen,
                                               WnckWorkspace   *space,
                                               gpointer         data);
static void application_opened_callback       (WnckScreen      *screen,
                                               WnckApplication *app);
static void application_closed_callback       (WnckScreen      *screen,
                                               WnckApplication *app);
static void showing_desktop_changed_callback  (WnckScreen      *screen,
                                               gpointer         data);
static void window_name_changed_callback      (WnckWindow      *window,
                                               gpointer         data);
static void window_state_changed_callback     (WnckWindow      *window,
                                               WnckWindowState  changed,
                                               WnckWindowState  new,
                                               gpointer         data);
static void window_workspace_changed_callback (WnckWindow      *window,
                                               gpointer         data);
static void window_icon_changed_callback      (WnckWindow      *window,
                                               gpointer         data);
static void window_geometry_changed_callback  (WnckWindow      *window,
                                               gpointer         data);

static GtkTreeModel* create_tree_model (void);
static GtkWidget*    create_tree_view  (void);
static void          refill_tree_model (GtkTreeModel *model,
                                        WnckScreen   *screen);
static void          update_window     (GtkTreeModel *model,
                                        WnckWindow   *window);
static void          queue_refill_model (void);

int
main (int argc, char **argv)
{
  WnckScreen *screen;
  GtkWidget *sw;
  GtkWidget *win;
  
  gtk_init (&argc, &argv);
  
  if (argc > 1 ) {
  
  	if ( strcmp (argv[1], "--use-wnck") == 0) {
  		use_wnck = TRUE;
  		g_print ("Using libwnck to fetch window icons\n");
  	}
   }

  screen = wnck_screen_get (0);

  g_signal_connect (G_OBJECT (screen), "active_window_changed",
                    G_CALLBACK (active_window_changed_callback),
                    NULL);
  g_signal_connect (G_OBJECT (screen), "active_workspace_changed",
                    G_CALLBACK (active_workspace_changed_callback),
                    NULL);
  g_signal_connect (G_OBJECT (screen), "window_stacking_changed",
                    G_CALLBACK (window_stacking_changed_callback),
                    NULL);
  g_signal_connect (G_OBJECT (screen), "window_opened",
                    G_CALLBACK (window_opened_callback),
                    NULL);
  g_signal_connect (G_OBJECT (screen), "window_closed",
                    G_CALLBACK (window_closed_callback),
                    NULL);
  g_signal_connect (G_OBJECT (screen), "workspace_created",
                    G_CALLBACK (workspace_created_callback),
                    NULL);
  g_signal_connect (G_OBJECT (screen), "workspace_destroyed",
                    G_CALLBACK (workspace_destroyed_callback),
                    NULL);
  g_signal_connect (G_OBJECT (screen), "application_opened",
                    G_CALLBACK (application_opened_callback),
                    NULL);
  g_signal_connect (G_OBJECT (screen), "application_closed",
                    G_CALLBACK (application_closed_callback),
                    NULL);
  g_signal_connect (G_OBJECT (screen), "showing_desktop_changed",
                    G_CALLBACK (showing_desktop_changed_callback),
                    NULL);
  
  global_tree_model = create_tree_model ();
  global_tree_view = create_tree_view ();
  
  gtk_tree_view_set_model (GTK_TREE_VIEW (global_tree_view),
                           global_tree_model);
  
  win = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  gtk_window_set_title (GTK_WINDOW (win), "Window List");
  
  sw = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
                                  GTK_POLICY_AUTOMATIC,
                                  GTK_POLICY_AUTOMATIC);
  
  gtk_container_add (GTK_CONTAINER (sw), global_tree_view);
  gtk_container_add (GTK_CONTAINER (win), sw);

  gtk_window_set_default_size (GTK_WINDOW (win), 650, 550);

  /* quit on window close */
  g_signal_connect (G_OBJECT (win), "destroy",
                    G_CALLBACK (gtk_main_quit),
                    NULL);
  
  gtk_widget_show_all (win);
  
  gtk_main ();
  
  return 0;
}

static void
active_window_changed_callback    (WnckScreen    *screen,
                                   gpointer       data)
{
  WnckWindow *window;
  
  //g_print ("Active window changed\n");

  window = wnck_screen_get_active_window (screen);

  if (window)
    update_window (global_tree_model, window);
}

static void
active_workspace_changed_callback (WnckScreen    *screen,
                                   gpointer       data)
{
  //g_print ("Active workspace changed\n");
}

static void
window_stacking_changed_callback  (WnckScreen    *screen,
                                   gpointer       data)
{
  //g_print ("Stacking changed\n");
}

static void
window_opened_callback            (WnckScreen    *screen,
                                   WnckWindow    *window,
                                   gpointer       data)
{
  //g_print ("Window '%s' opened (pid = %d session_id = %s)\n",
  //         wnck_window_get_name (window),
  //         wnck_window_get_pid (window),
  //         wnck_window_get_session_id (window) ?
  //         wnck_window_get_session_id (window) : "none");
  
  g_signal_connect (G_OBJECT (window), "name_changed",
                    G_CALLBACK (window_name_changed_callback),
                    NULL);
  g_signal_connect (G_OBJECT (window), "state_changed",
                    G_CALLBACK (window_state_changed_callback),
                    NULL);
  g_signal_connect (G_OBJECT (window), "workspace_changed",
                    G_CALLBACK (window_workspace_changed_callback),
                    NULL);    
  g_signal_connect (G_OBJECT (window), "icon_changed",
                    G_CALLBACK (window_icon_changed_callback),
                    NULL);
  g_signal_connect (G_OBJECT (window), "geometry_changed",
                    G_CALLBACK (window_geometry_changed_callback),
                    NULL);
  
  queue_refill_model ();
}

static void
window_closed_callback            (WnckScreen    *screen,
                                   WnckWindow    *window,
                                   gpointer       data)
{
  //g_print ("Window '%s' closed\n",
           //wnck_window_get_name (window));

  queue_refill_model ();
}

static void
workspace_created_callback        (WnckScreen    *screen,
                                   WnckWorkspace *space,
                                   gpointer       data)
{
  //g_print ("Workspace created\n");
}

static void
workspace_destroyed_callback      (WnckScreen    *screen,
                                   WnckWorkspace *space,
                                   gpointer       data)
{
  //g_print ("Workspace destroyed\n");
}

static void
application_opened_callback (WnckScreen      *screen,
                             WnckApplication *app)
{
  //g_print ("Application opened\n");
  queue_refill_model ();
}

static void
application_closed_callback (WnckScreen      *screen,
                             WnckApplication *app)
{
  //g_print ("Application closed\n");
  queue_refill_model ();
}

static void
showing_desktop_changed_callback (WnckScreen *screen,
                                  gpointer    data)
{
  //g_print ("Showing desktop now = %d\n",
           //wnck_screen_get_showing_desktop (screen));
}

static void
window_name_changed_callback (WnckWindow    *window,
                              gpointer       data)
{
  //g_print ("Name changed on window '%s'\n",
           //wnck_window_get_name (window));

  update_window (global_tree_model, window);
}

static void
window_state_changed_callback (WnckWindow     *window,
                               WnckWindowState changed,
                               WnckWindowState new,
                               gpointer        data)
{
  //g_print ("State changed on window '%s'\n",
  //         wnck_window_get_name (window));

  if (changed & WNCK_WINDOW_STATE_MINIMIZED)
    ;//g_print (" minimized = %d\n", wnck_window_is_minimized (window));

  if (changed & WNCK_WINDOW_STATE_MAXIMIZED_HORIZONTALLY)
    ;//g_print (" maximized horiz = %d\n", wnck_window_is_maximized_horizontally (window));

  if (changed & WNCK_WINDOW_STATE_MAXIMIZED_VERTICALLY)
    ;//g_print (" maximized vert = %d\n", wnck_window_is_maximized_vertically (window));

  if (changed & WNCK_WINDOW_STATE_SHADED)
    ;//g_print (" shaded = %d\n", wnck_window_is_shaded (window));

  if (changed & WNCK_WINDOW_STATE_SKIP_PAGER)
    ;//g_print (" skip pager = %d\n", wnck_window_is_skip_pager (window));

  if (changed & WNCK_WINDOW_STATE_SKIP_TASKLIST)
    ;//g_print (" skip tasklist = %d\n", wnck_window_is_skip_tasklist (window));

  if (changed & WNCK_WINDOW_STATE_STICKY)
    ;//g_print (" sticky = %d\n", wnck_window_is_sticky (window));

  if (changed & WNCK_WINDOW_STATE_FULLSCREEN)
    ;//g_print (" fullscreen = %d\n", wnck_window_is_fullscreen (window));

  g_assert ( ((new & WNCK_WINDOW_STATE_MINIMIZED) != 0) ==
             wnck_window_is_minimized (window) );
  g_assert ( ((new & WNCK_WINDOW_STATE_MAXIMIZED_HORIZONTALLY) != 0) ==
             wnck_window_is_maximized_horizontally (window) );
  g_assert ( ((new & WNCK_WINDOW_STATE_MAXIMIZED_VERTICALLY) != 0) ==
             wnck_window_is_maximized_vertically (window) );
  g_assert ( ((new & WNCK_WINDOW_STATE_SHADED) != 0) ==
             wnck_window_is_shaded (window) );
  g_assert ( ((new & WNCK_WINDOW_STATE_SKIP_PAGER) != 0) ==
             wnck_window_is_skip_pager (window) );
  g_assert ( ((new & WNCK_WINDOW_STATE_SKIP_TASKLIST) != 0) ==
             wnck_window_is_skip_tasklist (window) );
  g_assert ( ((new & WNCK_WINDOW_STATE_STICKY) != 0) ==
             wnck_window_is_sticky (window) );

  update_window (global_tree_model, window);
}

static void
window_workspace_changed_callback (WnckWindow    *window,
                                   gpointer       data)
{
  WnckWorkspace *space;

  space = wnck_window_get_workspace (window);

  if (space)
    g_print ("Workspace changed on window '%s' to %d\n",
             wnck_window_get_name (window),
             wnck_workspace_get_number (space));
  else
    g_print ("Window '%s' is now pinned to all workspaces\n",
             wnck_window_get_name (window));

  update_window (global_tree_model, window);
}

static void
window_icon_changed_callback (WnckWindow    *window,
                              gpointer       data)
{
  g_print ("Icon changed on window '%s'\n",
           wnck_window_get_name (window));

  update_window (global_tree_model, window);
}

static void
window_geometry_changed_callback  (WnckWindow      *window,
                                   gpointer         data)
{
  int x, y, width, height;

  wnck_window_get_geometry (window, &x, &y, &width, &height);
  
}

static GtkTreeModel*
create_tree_model (void)
{
  GtkListStore *store;
  
  store = gtk_list_store_new (1, WNCK_TYPE_WINDOW);

  return GTK_TREE_MODEL (store);
}

static void
refill_tree_model (GtkTreeModel *model,
                   WnckScreen   *screen)
{
  GList *tmp;
  
  gtk_list_store_clear (GTK_LIST_STORE (model));

  tmp = wnck_screen_get_windows (screen);
  while (tmp != NULL)
    {
      GtkTreeIter iter;
      WnckWindow *window = tmp->data;

      gtk_list_store_append (GTK_LIST_STORE (model), &iter);
      gtk_list_store_set (GTK_LIST_STORE (model), &iter, 0, window, -1);

      if (wnck_window_is_active (window))
        {
          GtkTreeSelection *selection;
          
          selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (global_tree_view));

          gtk_tree_selection_unselect_all (selection);
          
          gtk_tree_selection_select_iter (selection, &iter);
        }
      
      tmp = tmp->next;
    }
  
  gtk_tree_view_columns_autosize (GTK_TREE_VIEW (global_tree_view));
}

static void
update_window (GtkTreeModel *model,
               WnckWindow   *window)
{
  GtkTreeIter iter;
  GList *windows;
  int i;
  
  /* The trick here is to find the right row, we assume
   * the screen and the model are in sync, unless we have a
   * model refill queued, in which case they aren't and we'll update
   * this window in the idle queue anyhow.
   */
  if (refill_idle != 0)
    return;
  
  windows = wnck_screen_get_windows (wnck_window_get_screen (window));

  i = g_list_index (windows, window);

  g_return_if_fail (i >= 0);

  if (gtk_tree_model_iter_nth_child (model, &iter, NULL, i))
    {
      /* Reset the list store value to trigger a recompute */
      gtk_list_store_set (GTK_LIST_STORE (model), &iter, 0, window, -1);

      if (wnck_window_is_active (window))
        {
          GtkTreeSelection *selection;
          
          selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (global_tree_view));

          gtk_tree_selection_unselect_all (selection);

          gtk_tree_selection_select_iter (selection, &iter);
        }
    }
  else
    g_warning ("Tree model has no row %d", i);
}

static WnckWindow*
get_window (GtkTreeModel *model,
            GtkTreeIter  *iter)
{
  WnckWindow *window;
  
  gtk_tree_model_get (model, iter,
                      0, &window,
                      -1);

  /* window may be NULL after we append to the list store and
   * before we set the value with gtk_list_store_set()
   */
  if (window)
    {
      /* we know the model and screen are still holding a reference,
       * so cheat a bit
       */
      g_object_unref (G_OBJECT (window));
    }

  return window;
}

static GdkPixbuf *
_fetch_icon (WnckWindow *window) 
{
	if (use_wnck)
		return wnck_window_get_icon (window);
	
	GdkPixbuf *icon;
  	GdkPixbuf *mini_icon;

  	icon = NULL;
  	mini_icon = NULL;
  	
  	
  	if ( _wnck_read_icons_ (wnck_window_get_xid(window),
                        NULL,
                        &icon,
                        48, 48,
                        &mini_icon,
                        24,
                        24) ) {
                        
        	if (icon)
        		return icon;
  	}
  	g_print("Unable to get icon for :%s, reverting to libwnck\n",
  		wnck_application_get_name(wnck_window_get_application(window)));
  	return wnck_window_get_icon (window);
}

static void
icon_set_func (GtkTreeViewColumn *tree_column,
               GtkCellRenderer   *cell,
               GtkTreeModel      *model,
               GtkTreeIter       *iter,
               gpointer           data)
{
  WnckWindow *window;
  
  window = get_window (model, iter);
  if (window == NULL)
    return;
  
  g_object_set (GTK_CELL_RENDERER (cell),
                "pixbuf", _fetch_icon(window),
                NULL);
}

static void
title_set_func (GtkTreeViewColumn *tree_column,
                GtkCellRenderer   *cell,
                GtkTreeModel      *model,
                GtkTreeIter       *iter,
                gpointer           data)
{
  WnckWindow *window;

  window = get_window (model, iter);
  if (window == NULL)
    return;
  
  g_object_set (GTK_CELL_RENDERER (cell),
                "text", wnck_window_get_name (window),
                NULL);
}

static void
workspace_set_func (GtkTreeViewColumn *tree_column,
                    GtkCellRenderer   *cell,
                    GtkTreeModel      *model,
                    GtkTreeIter       *iter,
                    gpointer           data)
{
  WnckWindow *window;
  WnckWorkspace *space;
  char *name;
  
  window = get_window (model, iter);
  if (window == NULL)
    return;
  
  space = wnck_window_get_workspace (window);

  if (space)
    name = g_strdup_printf ("%d", wnck_workspace_get_number (space));
  else if (wnck_window_is_pinned (window))
    name = g_strdup ("all");
  else
    name = g_strdup ("none");
  
  g_object_set (GTK_CELL_RENDERER (cell),
                "text", name,
                NULL);

  g_free (name);
}

static void
pid_set_func (GtkTreeViewColumn *tree_column,
              GtkCellRenderer   *cell,
              GtkTreeModel      *model,
              GtkTreeIter       *iter,
              gpointer           data)
{
  WnckWindow *window;
  int pid;
  char *name;
  
  window = get_window (model, iter);
  if (window == NULL)
    return;
  
  pid = wnck_window_get_pid (window);

  if (pid != 0)
    name = g_strdup_printf ("%d", pid);
  else
    name = g_strdup ("not set");
  
  g_object_set (GTK_CELL_RENDERER (cell),
                "text", name,
                NULL);

  g_free (name);
}

static void
shaded_set_func (GtkTreeViewColumn *tree_column,
                 GtkCellRenderer   *cell,
                 GtkTreeModel      *model,
                 GtkTreeIter       *iter,
                 gpointer           data)
{
  WnckWindow *window;

  window = get_window (model, iter);
  if (window == NULL)
    return;
  
  gtk_cell_renderer_toggle_set_active (GTK_CELL_RENDERER_TOGGLE (cell),
                                       wnck_window_is_shaded (window));
}

static void
shaded_toggled_callback (GtkCellRendererToggle *cell,
                         char                  *path_string,
                         gpointer               data)
{
  GtkTreeView *tree_view = GTK_TREE_VIEW (data);
  GtkTreeModel *model = gtk_tree_view_get_model (tree_view);
  GtkTreePath *path = gtk_tree_path_new_from_string (path_string);
  GtkTreeIter iter;
  WnckWindow *window;

  gtk_tree_model_get_iter (model, &iter, path);
  window = get_window (model, &iter);

  if (wnck_window_is_shaded (window))
    wnck_window_unshade (window);
  else
    wnck_window_shade (window);
  
  gtk_tree_path_free (path);
}

static void
minimized_set_func (GtkTreeViewColumn *tree_column,
                    GtkCellRenderer   *cell,
                    GtkTreeModel      *model,
                    GtkTreeIter       *iter,
                    gpointer           data)
{
  WnckWindow *window;

  window = get_window (model, iter);
  if (window == NULL)
    return;

  
  gtk_cell_renderer_toggle_set_active (GTK_CELL_RENDERER_TOGGLE (cell),
                                       wnck_window_is_minimized (window));
}

static void
minimized_toggled_callback (GtkCellRendererToggle *cell,
                            char                  *path_string,
                            gpointer               data)
{
  GtkTreeView *tree_view = GTK_TREE_VIEW (data);
  GtkTreeModel *model = gtk_tree_view_get_model (tree_view);
  GtkTreePath *path = gtk_tree_path_new_from_string (path_string);
  GtkTreeIter iter;
  WnckWindow *window;

  gtk_tree_model_get_iter (model, &iter, path);
  window = get_window (model, &iter);

  if (wnck_window_is_minimized (window))
    /* The toggled callback will only be called in reaction to user
     * button presses or key presses, so gtk_get_current_event_time()
     * should be okay here.
     */
    wnck_window_unminimize (window, gtk_get_current_event_time ());
  else
    wnck_window_minimize (window);
  
  gtk_tree_path_free (path);
}

static void
maximized_set_func (GtkTreeViewColumn *tree_column,
                    GtkCellRenderer   *cell,
                    GtkTreeModel      *model,
                    GtkTreeIter       *iter,
                    gpointer           data)
{
  WnckWindow *window;

  window = get_window (model, iter);
  if (window == NULL)
    return;

  
  gtk_cell_renderer_toggle_set_active (GTK_CELL_RENDERER_TOGGLE (cell),
                                       wnck_window_is_maximized (window));
}

static void
maximized_toggled_callback (GtkCellRendererToggle *cell,
                            char                  *path_string,
                            gpointer               data)
{
  GtkTreeView *tree_view = GTK_TREE_VIEW (data);
  GtkTreeModel *model = gtk_tree_view_get_model (tree_view);
  GtkTreePath *path = gtk_tree_path_new_from_string (path_string);
  GtkTreeIter iter;
  WnckWindow *window;

  gtk_tree_model_get_iter (model, &iter, path);
  window = get_window (model, &iter);

  if (wnck_window_is_maximized (window))
    wnck_window_unmaximize (window);
  else
    wnck_window_maximize (window);
  
  gtk_tree_path_free (path);
}

static void
session_id_set_func (GtkTreeViewColumn *tree_column,
                     GtkCellRenderer   *cell,
                     GtkTreeModel      *model,
                     GtkTreeIter       *iter,
                     gpointer           data)
{
  WnckWindow *window;
  const char *id;
  
  window = get_window (model, iter);
  if (window == NULL)
    return;
  
  id = wnck_window_get_session_id_utf8 (window);

  g_object_set (GTK_CELL_RENDERER (cell),
                "text", id ? id : "not session managed",
                NULL);
}

static gboolean
selection_func (GtkTreeSelection  *selection,
                GtkTreeModel      *model,
                GtkTreePath       *path,
                gboolean           currently_selected,
                gpointer           data)
{
  GtkTreeIter iter;
  WnckWindow *window;
  
  /* Kind of some hack action here. If you try to select a row that's
   * not the active window, we ask the WM to make that window active
   * as a side effect. But we don't actually allow selecting anything
   * that isn't already active. Then, in the active window callback we
   * select the newly-active window
   */
  
  gtk_tree_model_get_iter (model, &iter, path);  

  window = get_window (model, &iter);
  if (window == NULL)
    return FALSE;

  if (currently_selected)
    {
      /* Trying to unselect, not allowed if we are the active window */
      if (wnck_window_is_active (window))
        return FALSE;
      else
        return TRUE;
    }
  else
    {
      if (wnck_window_is_active (window))
        return TRUE;
      else
        {
          /* This should only be called in reaction to user button
           * presses or key presses (I hope), so
           * gtk_get_current_event_time() should be okay here.
           */
          wnck_window_activate (window, gtk_get_current_event_time ());
          return FALSE;
        }
    }
}

static GtkWidget*
create_tree_view (void)
{
  GtkWidget *tree_view;
  GtkCellRenderer *cell_renderer;
  GtkTreeViewColumn *column;
  GtkTreeSelection *selection;
  
  tree_view = gtk_tree_view_new ();

  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (tree_view), TRUE);
  
  /* The icon and title are in the same column, so pack
   * two cell renderers into that column
   */
  column = gtk_tree_view_column_new ();
  gtk_tree_view_column_set_title (column, "Window");

  cell_renderer = gtk_cell_renderer_pixbuf_new ();
  g_object_set (G_OBJECT (cell_renderer),
                "xpad", 2,
                NULL);
  gtk_tree_view_column_pack_start (column,
                                   cell_renderer,
                                   FALSE);
  gtk_tree_view_column_set_cell_data_func (column, cell_renderer,
                                           icon_set_func, NULL, NULL);
  cell_renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_column_pack_start (column,
                                   cell_renderer,
                                   TRUE);
  gtk_tree_view_column_set_cell_data_func (column, cell_renderer,
                                           title_set_func, NULL, NULL);

  gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view),
                               column);
  
  /* Then create a workspace column, only one renderer in this column
   * so we get to use insert_column convenience function
   */
  cell_renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_insert_column_with_data_func (GTK_TREE_VIEW (tree_view),
                                              -1, /* append */
                                              "Workspace",
                                              cell_renderer,
                                              workspace_set_func,
                                              NULL,
                                              NULL);

  /* Process ID */
  cell_renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_insert_column_with_data_func (GTK_TREE_VIEW (tree_view),
                                              -1, /* append */
                                              "PID",
                                              cell_renderer,
                                              pid_set_func,
                                              NULL,
                                              NULL);
  
  /* Shaded checkbox */
  cell_renderer = gtk_cell_renderer_toggle_new ();
  gtk_tree_view_insert_column_with_data_func (GTK_TREE_VIEW (tree_view),
                                              -1, /* append */
                                              "Shaded",
                                              cell_renderer,
                                              shaded_set_func,
                                              NULL,
                                              NULL);
  g_signal_connect (G_OBJECT (cell_renderer), "toggled",
                    G_CALLBACK (shaded_toggled_callback),
                    tree_view);

  /* Minimized checkbox */
  cell_renderer = gtk_cell_renderer_toggle_new ();
  gtk_tree_view_insert_column_with_data_func (GTK_TREE_VIEW (tree_view),
                                              -1, /* append */
                                              "Minimized",
                                              cell_renderer,
                                              minimized_set_func,
                                              NULL,
                                              NULL);
  g_signal_connect (G_OBJECT (cell_renderer), "toggled",
                    G_CALLBACK (minimized_toggled_callback),
                    tree_view);

  /* Maximized checkbox */
  cell_renderer = gtk_cell_renderer_toggle_new ();
  gtk_tree_view_insert_column_with_data_func (GTK_TREE_VIEW (tree_view),
                                              -1, /* append */
                                              "Maximized",
                                              cell_renderer,
                                              maximized_set_func,
                                              NULL,
                                              NULL);
  g_signal_connect (G_OBJECT (cell_renderer), "toggled",
                    G_CALLBACK (maximized_toggled_callback),
                    tree_view);

  /* Session ID */
  cell_renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_insert_column_with_data_func (GTK_TREE_VIEW (tree_view),
                                              -1, /* append */
                                              "Session ID",
                                              cell_renderer,
                                              session_id_set_func,
                                              NULL,
                                              NULL);
  
  /* The selection will track the active window, so we need to
   * handle it with a custom function
   */
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
  gtk_tree_selection_set_mode (selection, GTK_SELECTION_MULTIPLE);
  gtk_tree_selection_set_select_function (selection, selection_func, NULL, NULL);
  return tree_view;
}

static gboolean
do_refill_model (gpointer data)
{
  refill_idle = 0;

  refill_tree_model (global_tree_model,
                     wnck_screen_get (0));

  return FALSE;
}

static void
queue_refill_model (void)
{
  if (refill_idle != 0)
    return;

  /* Don't keep any stale references */
  gtk_list_store_clear (GTK_LIST_STORE (global_tree_model));
  
  refill_idle = g_idle_add (do_refill_model, NULL);
}
