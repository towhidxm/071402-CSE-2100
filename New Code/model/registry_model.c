/**
 * @file registry_model.c
 * @brief Model layer – GTK store population from registry data.
 *
 * Course:     <Course Name>
 * University: <University Name>
 * Author:     <Your Name>
 * Date:       2026-02-17
 */

#include "registry_model.h"
#include "registry_facade.h"
#include <string.h>   /* strcmp */

/* ─────────────────────────────────────────────────────────────────
 * Internal helpers
 * ───────────────────────────────────────────────────────────────── */

/** Append a "Loading…" placeholder child under @p parent_iter. */
static void append_placeholder (AppState *state, GtkTreeIter *parent_iter)
{
    GtkTreeIter ph;
    gtk_tree_store_append (state->key_tree_store, &ph, parent_iter);
    gtk_tree_store_set (state->key_tree_store, &ph,
                        TREE_COL_NAME,      "Loading\xe2\x80\xa6", /* UTF-8 ellipsis */
                        TREE_COL_HKEY_PTR,  NULL,
                        TREE_COL_FULL_PATH, "",
                        -1);
}

/**
 * Append one registry subkey row under @p parent_iter.
 * Opens the subkey so future expansions can enumerate its children.
 */
static void append_subkey_row (AppState     *state,
                                GtkTreeIter  *parent_iter,
                                HKEY          parent_hkey,
                                const WCHAR  *name_w)
{
    /* Build full path: grab parent's path, append backslash + new name. */
    gchar *parent_path = NULL;
    gtk_tree_model_get (GTK_TREE_MODEL (state->key_tree_store), parent_iter,
                        TREE_COL_FULL_PATH, &parent_path, -1);

    gchar *name_utf8 = g_utf16_to_utf8 ((const gunichar2 *)name_w,
                                         -1, NULL, NULL, NULL);
    gchar *full_path = g_strdup_printf ("%s\\%s", parent_path, name_utf8);
    g_free (parent_path);

    /* Open the subkey for later use. */
    HKEY child_hkey = NULL;
    registry_open_key (parent_hkey, name_w, &child_hkey);

    /* Insert the row. */
    GtkTreeIter child_iter;
    gtk_tree_store_append (state->key_tree_store, &child_iter, parent_iter);
    gtk_tree_store_set (state->key_tree_store, &child_iter,
                        TREE_COL_NAME,      name_utf8,
                        TREE_COL_HKEY_PTR,  (gpointer)child_hkey,
                        TREE_COL_FULL_PATH, full_path,
                        -1);

    /* Give it a placeholder so the expander arrow appears. */
    append_placeholder (state, &child_iter);

    g_free (name_utf8);
    g_free (full_path);
}

/* ─────────────────────────────────────────────────────────────────
 * Public API
 * ───────────────────────────────────────────────────────────────── */

/** Root keys to display at the top level. */
static const struct { const gchar *name; HKEY hkey; } k_root_keys[] = {
    { "HKEY_CLASSES_ROOT",   HKEY_CLASSES_ROOT   },
    { "HKEY_CURRENT_USER",   HKEY_CURRENT_USER   },
    { "HKEY_LOCAL_MACHINE",  HKEY_LOCAL_MACHINE  },
    { "HKEY_USERS",          HKEY_USERS          },
    { "HKEY_CURRENT_CONFIG", HKEY_CURRENT_CONFIG },
};

void model_populate_root_keys (AppState *state)
{
    g_return_if_fail (state != NULL);

    for (gsize i = 0; i < G_N_ELEMENTS (k_root_keys); i++) {
        GtkTreeIter root_iter;
        gtk_tree_store_append (state->key_tree_store, &root_iter, NULL);
        gtk_tree_store_set (state->key_tree_store, &root_iter,
                            TREE_COL_NAME,      k_root_keys[i].name,
                            TREE_COL_HKEY_PTR,  (gpointer)k_root_keys[i].hkey,
                            TREE_COL_FULL_PATH, k_root_keys[i].name,
                            -1);
        append_placeholder (state, &root_iter);
    }
}

void model_expand_node (AppState *state, GtkTreeIter *parent_iter)
{
    g_return_if_fail (state       != NULL);
    g_return_if_fail (parent_iter != NULL);

    HKEY parent_hkey = NULL;
    gtk_tree_model_get (GTK_TREE_MODEL (state->key_tree_store), parent_iter,
                        TREE_COL_HKEY_PTR, &parent_hkey, -1);

    if (!parent_hkey) return;

    /* Check whether the first child is still the "Loading…" placeholder. */
    GtkTreeIter first_child;
    if (!gtk_tree_model_iter_children (GTK_TREE_MODEL (state->key_tree_store),
                                       &first_child, parent_iter))
        return; /* no children at all – nothing to do */

    gchar *child_name = NULL;
    gtk_tree_model_get (GTK_TREE_MODEL (state->key_tree_store), &first_child,
                        TREE_COL_NAME, &child_name, -1);

    gboolean is_placeholder = (child_name &&
                                strncmp (child_name, "Loading", 7) == 0);
    g_free (child_name);

    if (!is_placeholder) return; /* Already expanded – do not re-populate. */

    /* Remove the placeholder. */
    gtk_tree_store_remove (state->key_tree_store, &first_child);

    /* Enumerate real subkeys via the facade and insert rows. */
    WCHAR **subkeys = registry_enum_subkeys (parent_hkey);
    if (!subkeys) return;

    for (WCHAR **p = subkeys; *p; p++)
        append_subkey_row (state, parent_iter, parent_hkey, *p);

    registry_free_subkey_names (subkeys);
}

void model_load_values (AppState *state, HKEY hkey)
{
    g_return_if_fail (state != NULL);

    gtk_list_store_clear (state->value_list_store);
    if (!hkey) return;

    DWORD          count  = 0;
    RegistryValue *values = registry_enum_values (hkey, &count);
    if (!values) return;

    for (DWORD i = 0; i < count; i++) {
        GtkTreeIter iter;
        gtk_list_store_append (state->value_list_store, &iter);
        gtk_list_store_set (state->value_list_store, &iter,
                            VALUE_COL_NAME,     values[i].name,
                            VALUE_COL_TYPE_STR, values[i].type_str,
                            VALUE_COL_DATA_STR, values[i].data_str,
                            VALUE_COL_TYPE_RAW, values[i].type,
                            VALUE_COL_DATA_RAW, (gpointer)values[i].raw_data,
                            -1);
    }

    registry_free_values (values, count);
}
