/**
 * @file registry_controller.c
 * @brief Controller layer – GTK signal handlers and timer management.
 *
 * Course:     <Course Name>
 * University: <University Name>
 * Author:     <Your Name>
 * Date:       2026-02-17
 */

#include "registry_controller.h"
#include "registry_model.h"
#include "registry_view.h"
#include "registry_facade.h"

/* ─────────────────────────────────────────────────────────────────
 * Internal helpers
 * ───────────────────────────────────────────────────────────────── */

/** Build a timestamped status string.  Caller must g_free. */
static gchar *make_ready_status (void)
{
    GDateTime *now      = g_date_time_new_now_local ();
    gchar     *time_str = g_date_time_format (now, "%Y-%m-%d %H:%M:%S");
    gchar     *status   = g_strdup_printf ("Ready  –  %s", time_str);
    g_date_time_unref (now);
    g_free (time_str);
    return status;
}

/** Reload the value pane for the currently selected key. */
static void refresh_values (AppState *state)
{
    if (!state->current_hkey) return;

    view_set_status (state, "Loading\xe2\x80\xa6");
    model_load_values (state, state->current_hkey);

    gchar *status = make_ready_status ();
    view_set_status (state, status);
    g_free (status);
}

/* ─────────────────────────────────────────────────────────────────
 * Signal handlers
 * ───────────────────────────────────────────────────────────────── */

/**
 * Fired when the user selects a different key in the tree view.
 * Loads that key's values into the right pane.
 */
static void on_selection_changed (GtkTreeSelection *selection,
                                   gpointer          user_data)
{
    AppState    *state = (AppState *)user_data;
    GtkTreeModel *model;
    GtkTreeIter   iter;

    if (!gtk_tree_selection_get_selected (selection, &model, &iter))
        return;

    HKEY  hkey = NULL;
    gchar *path = NULL;
    gtk_tree_model_get (model, &iter,
                        TREE_COL_HKEY_PTR,  &hkey,
                        TREE_COL_FULL_PATH, &path,
                        -1);

    state->current_hkey = hkey;

    if (path && *path) {
        gchar *status = g_strdup_printf ("Viewing: %s", path);
        view_set_status (state, status);
        g_free (status);
    }
    g_free (path);

    refresh_values (state);
}

/**
 * Fired when the user expands a tree node.
 * Lazily loads subkeys for that node.
 */
static void on_row_expanded (GtkTreeView *tree_view,
                              GtkTreeIter *iter,
                              GtkTreePath *path,
                              gpointer     user_data)
{
    (void)tree_view;
    (void)path;
    AppState *state = (AppState *)user_data;
    model_expand_node (state, iter);
}

/**
 * Auto-refresh timer callback: reload value pane every N seconds.
 */
static gboolean on_auto_refresh_tick (gpointer user_data)
{
    AppState *state = (AppState *)user_data;
    refresh_values (state);
    return G_SOURCE_CONTINUE;
}

/* ─────────────────────────────────────────────────────────────────
 * Public API
 * ───────────────────────────────────────────────────────────────── */

void controller_connect_signals (AppState *state)
{
    g_return_if_fail (state != NULL);

    /* Key-selection signal */
    GtkTreeSelection *sel = gtk_tree_view_get_selection (state->key_tree_view);
    g_signal_connect (sel, "changed",
                      G_CALLBACK (on_selection_changed), state);

    /* Row-expanded signal for lazy loading */
    g_signal_connect (state->key_tree_view, "row-expanded",
                      G_CALLBACK (on_row_expanded), state);
}

void controller_start_auto_refresh (AppState *state)
{
    g_return_if_fail (state != NULL);
    if (state->refresh_timer_id != 0) return; /* Already running */

    state->refresh_timer_id = g_timeout_add_seconds (
        AUTO_REFRESH_INTERVAL_SEC, on_auto_refresh_tick, state);
    view_set_status (state, "Auto-refresh ON");
}

void controller_stop_auto_refresh (AppState *state)
{
    g_return_if_fail (state != NULL);
    if (state->refresh_timer_id == 0) return;

    g_source_remove (state->refresh_timer_id);
    state->refresh_timer_id = 0;
    view_set_status (state, "Auto-refresh OFF");
}

void controller_cleanup (AppState *state)
{
    if (!state) return;

    controller_stop_auto_refresh (state);

    /* The root HKEYs (HKLM, HKCU, …) are predefined constants and must
     * NOT be closed.  Subkeys opened by add_subkey_row() are owned by
     * the TreeStore rows; closing them here would be double-free territory.
     * The OS automatically reclaims handles when the process exits. */
    state->current_hkey = NULL;
}
