/**
 * @file registry_view.c
 * @brief View layer – GTK widget construction and status helpers.
 *
 * Course:     <Course Name>
 * University: <University Name>
 * Author:     <Your Name>
 * Date:       2026-02-17
 */

#include "registry_view.h"
#include "registry_types.h"

/* ─────────────────────────────────────────────────────────────────
 * Internal: Key Tree View (left pane)
 * ───────────────────────────────────────────────────────────────── */

static GtkWidget *build_key_tree_view (AppState *state)
{
    /* Create TreeStore */
    state->key_tree_store = gtk_tree_store_new (
        TREE_N_COLUMNS,
        G_TYPE_STRING,   /* TREE_COL_NAME      */
        G_TYPE_POINTER,  /* TREE_COL_HKEY_PTR  */
        G_TYPE_STRING    /* TREE_COL_FULL_PATH */
    );

    GtkWidget *tv = gtk_tree_view_new_with_model (
        GTK_TREE_MODEL (state->key_tree_store));
    g_object_unref (state->key_tree_store); /* TreeView now owns the model */

    state->key_tree_view = GTK_TREE_VIEW (tv);

    gtk_tree_view_set_headers_visible  (GTK_TREE_VIEW (tv), FALSE);
    gtk_tree_view_set_enable_tree_lines(GTK_TREE_VIEW (tv), TRUE);
    gtk_tree_view_set_show_expanders   (GTK_TREE_VIEW (tv), TRUE);

    GtkCellRenderer    *renderer = gtk_cell_renderer_text_new ();
    GtkTreeViewColumn  *col      = gtk_tree_view_column_new_with_attributes (
        "Key", renderer, "text", TREE_COL_NAME, NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (tv), col);

    /* Wrap in a scrolled window */
    GtkWidget *sw = gtk_scrolled_window_new ();
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
                                    GTK_POLICY_AUTOMATIC,
                                    GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (sw), tv);
    gtk_widget_set_hexpand (sw, TRUE);
    gtk_widget_set_vexpand (sw, TRUE);
    return sw;
}

/* ─────────────────────────────────────────────────────────────────
 * Internal: Value List View (right pane)
 * ───────────────────────────────────────────────────────────────── */

static GtkWidget *build_value_list_view (AppState *state)
{
    state->value_list_store = gtk_list_store_new (
        VALUE_N_COLUMNS,
        G_TYPE_STRING,   /* VALUE_COL_NAME     */
        G_TYPE_STRING,   /* VALUE_COL_TYPE_STR */
        G_TYPE_STRING,   /* VALUE_COL_DATA_STR */
        G_TYPE_UINT,     /* VALUE_COL_TYPE_RAW */
        G_TYPE_POINTER   /* VALUE_COL_DATA_RAW */
    );

    GtkWidget *tv = gtk_tree_view_new_with_model (
        GTK_TREE_MODEL (state->value_list_store));
    g_object_unref (state->value_list_store);

    state->value_tree_view = GTK_TREE_VIEW (tv);
    gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (tv), TRUE);

    /* Helper macro for adding columns */
#define ADD_COL(label, col_id, min_w)                                      \
    do {                                                                    \
        GtkCellRenderer   *_r = gtk_cell_renderer_text_new ();             \
        GtkTreeViewColumn *_c = gtk_tree_view_column_new_with_attributes ( \
            label, _r, "text", col_id, NULL);                              \
        gtk_tree_view_column_set_resizable (_c, TRUE);                     \
        gtk_tree_view_column_set_min_width (_c, min_w);                    \
        gtk_tree_view_append_column (GTK_TREE_VIEW (tv), _c);              \
    } while (0)

    ADD_COL ("Name", VALUE_COL_NAME,     150);
    ADD_COL ("Type", VALUE_COL_TYPE_STR, 130);
    ADD_COL ("Data", VALUE_COL_DATA_STR, 300);
#undef ADD_COL

    GtkWidget *sw = gtk_scrolled_window_new ();
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
                                    GTK_POLICY_AUTOMATIC,
                                    GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (sw), tv);
    gtk_widget_set_hexpand (sw, TRUE);
    gtk_widget_set_vexpand (sw, TRUE);
    return sw;
}

/* ─────────────────────────────────────────────────────────────────
 * Internal: Toolbar
 * ───────────────────────────────────────────────────────────────── */

static GtkWidget *build_toolbar (void)
{
    GtkWidget *bar = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_widget_set_margin_start  (bar, 6);
    gtk_widget_set_margin_end    (bar, 6);
    gtk_widget_set_margin_top    (bar, 4);
    gtk_widget_set_margin_bottom (bar, 4);

    /* Read-only badge */
    GtkWidget *badge = gtk_label_new ("\xf0\x9f\x94\x92  READ-ONLY MODE");
    gtk_widget_set_tooltip_text (badge,
        "Registry modifications are disabled.\n"
        "Your system is protected from accidental changes.");
    gtk_widget_add_css_class (badge, "warning");
    gtk_box_append (GTK_BOX (bar), badge);

    gtk_box_append (GTK_BOX (bar),
                    gtk_separator_new (GTK_ORIENTATION_VERTICAL));

    GtkWidget *hint = gtk_label_new ("Select a key to view its values.");
    gtk_widget_set_halign (hint, GTK_ALIGN_START);
    gtk_widget_set_hexpand (hint, TRUE);
    gtk_box_append (GTK_BOX (bar), hint);

    return bar;
}

/* ─────────────────────────────────────────────────────────────────
 * Public API
 * ───────────────────────────────────────────────────────────────── */

void view_build_main_window (GtkApplication *app, AppState *state)
{
    g_return_if_fail (app   != NULL);
    g_return_if_fail (state != NULL);

    /* Main window */
    GtkWidget *window = gtk_application_window_new (app);
    state->main_window = window;
    gtk_window_set_title        (GTK_WINDOW (window), APP_TITLE);
    gtk_window_set_default_size (GTK_WINDOW (window), 1100, 750);

    /* Root container */
    GtkWidget *root_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    gtk_window_set_child (GTK_WINDOW (window), root_box);

    /* Toolbar */
    gtk_box_append (GTK_BOX (root_box), build_toolbar ());
    gtk_box_append (GTK_BOX (root_box),
                    gtk_separator_new (GTK_ORIENTATION_HORIZONTAL));

    /* Paned split */
    GtkWidget *paned = gtk_paned_new (GTK_ORIENTATION_HORIZONTAL);
    gtk_widget_set_vexpand (paned, TRUE);
    gtk_box_append (GTK_BOX (root_box), paned);

    GtkWidget *left_sw  = build_key_tree_view  (state);
    GtkWidget *right_sw = build_value_list_view (state);

    gtk_paned_set_start_child (GTK_PANED (paned), left_sw);
    gtk_paned_set_end_child   (GTK_PANED (paned), right_sw);
    gtk_paned_set_position    (GTK_PANED (paned), 310);

    /* Status bar */
    state->status_bar = gtk_statusbar_new ();
    gtk_box_append (GTK_BOX (root_box), state->status_bar);

    gtk_widget_set_visible (window, TRUE);
}

void view_set_status (AppState *state, const gchar *message)
{
    g_return_if_fail (state   != NULL);
    g_return_if_fail (message != NULL);

    GtkStatusbar *sb = GTK_STATUSBAR (state->status_bar);
    gtk_statusbar_pop  (sb, 0);
    gtk_statusbar_push (sb, 0, message);
}

void view_show_safety_notice (GtkWindow *parent)
{
    GtkWidget *dialog = gtk_dialog_new_with_buttons (
        "\xe2\x9a\xa0\xef\xb8\x8f  Safety Notice – Registry Viewer",
        parent,
        GTK_DIALOG_MODAL,
        "I Understand",
        GTK_RESPONSE_ACCEPT,
        NULL
    );

    GtkWidget *content = gtk_dialog_get_content_area (GTK_DIALOG (dialog));

    GtkWidget *label = gtk_label_new (
        "\xf0\x9f\x94\x92 READ-ONLY MODE ACTIVE\n\n"
        "This viewer operates in safe read-only mode:\n\n"
        "  \xe2\x9c\x85  View registry keys and values\n"
        "  \xe2\x9c\x85  Refresh data in real-time\n"
        "  \xe2\x9d\x8c  Edit values (disabled)\n"
        "  \xe2\x9d\x8c  Delete keys (disabled)\n"
        "  \xe2\x9d\x8c  Modify system settings (disabled)\n\n"
        "Your system is protected from accidental changes."
    );
    gtk_label_set_wrap      (GTK_LABEL (label), TRUE);
    gtk_label_set_justify   (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
    gtk_widget_set_margin_top    (label, 20);
    gtk_widget_set_margin_bottom (label, 20);
    gtk_widget_set_margin_start  (label, 24);
    gtk_widget_set_margin_end    (label, 24);
    gtk_box_append (GTK_BOX (content), label);

    g_signal_connect (dialog, "response", G_CALLBACK (gtk_window_destroy), NULL);
    gtk_window_present (GTK_WINDOW (dialog));
}
