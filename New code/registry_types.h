/**
 * @file registry_types.h
 * @brief Shared type definitions, enums, and constants
 *
 * Course:     <Course Name>
 * University: <University Name>
 * Author:     <Your Name>
 * Date:       2026-02-17
 */

#ifndef REGISTRY_TYPES_H
#define REGISTRY_TYPES_H

#include <gtk/gtk.h>
#include <windows.h>

/* ─────────────────────────────────────────────
 * Application metadata
 * ───────────────────────────────────────────── */
#define APP_ID          "org.example.RegistryViewer"
#define APP_TITLE       "Windows Registry Viewer"
#define APP_VERSION     "1.0.0"
#define AUTO_REFRESH_INTERVAL_SEC 5

/* ─────────────────────────────────────────────
 * TreeStore column indices  (Key tree, left pane)
 * ───────────────────────────────────────────── */
typedef enum {
    TREE_COL_NAME      = 0,   /**< Display name of the registry key  */
    TREE_COL_HKEY_PTR  = 1,   /**< Opened HKEY handle (GType pointer)*/
    TREE_COL_FULL_PATH = 2,   /**< Full path string, e.g. HKLM\\SW   */
    TREE_N_COLUMNS     = 3
} TreeColumn;

/* ─────────────────────────────────────────────
 * ListStore column indices  (Value list, right pane)
 * ───────────────────────────────────────────── */
typedef enum {
    VALUE_COL_NAME     = 0,   /**< Value name                        */
    VALUE_COL_TYPE_STR = 1,   /**< Human-readable type, e.g. REG_SZ  */
    VALUE_COL_DATA_STR = 2,   /**< Human-readable data string        */
    VALUE_COL_TYPE_RAW = 3,   /**< Raw DWORD type (internal use)     */
    VALUE_COL_DATA_RAW = 4,   /**< Raw data pointer (internal use)   */
    VALUE_N_COLUMNS    = 5
} ValueColumn;

/* ─────────────────────────────────────────────
 * Application-wide state (passed via user_data)
 * ───────────────────────────────────────────── */
typedef struct AppState {
    /* Models */
    GtkTreeStore  *key_tree_store;
    GtkListStore  *value_list_store;

    /* View widgets */
    GtkTreeView   *key_tree_view;
    GtkTreeView   *value_tree_view;
    GtkWidget     *status_bar;
    GtkWidget     *main_window;

    /* Controller state */
    HKEY           current_hkey;     /**< Currently viewed registry key  */
    guint          refresh_timer_id; /**< 0 when timer is not running    */
} AppState;

#endif /* REGISTRY_TYPES_H */
