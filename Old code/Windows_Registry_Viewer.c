#include <gtk/gtk.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
enum
{
    TREE_COL_NAME,
    TREE_COL_HKEY_PTR,
    TREE_COL_FULL_PATH,
    TREE_N_COLUMNS
};
enum
{
    VALUE_COL_NAME,
    VALUE_COL_TYPE_STR,
    VALUE_COL_DATA_STR,
    VALUE_COL_TYPE_RAW,
    VALUE_COL_DATA_RAW,
    VALUE_N_COLUMNS
};
GtkTreeView *g_key_tree_view = NULL;
GtkTreeStore *g_key_tree_store = NULL;
GtkTreeView *g_value_tree_view = NULL;
GtkListStore *g_value_list_store = NULL;
GtkWidget *g_status_bar = NULL;
GtkWidget *g_refresh_button = NULL;
guint g_refresh_timer_id = 0;
HKEY g_current_selected_key = NULL;
static gchar *get_text_from_iter(GtkTreeModel *model, GtkTreeIter *iter, gint column)
{
    gchar *text;
    gtk_tree_model_get(model, iter, column, &text, -1);
    return text;
    static void add_root_key(const gchar *name, HKEY root_hkey)
    {
        GtkTreeIter iter;
        gtk_tree_store_append(g_key_tree_store, &iter, NULL);
        gtk_tree_store_set(g_key_tree_store, &iter, TREE_COL_NAME, name, TREE_COL_HKEY_PTR, (gpointer)root_hkey, TREE_COL_FULL_PATH, name, -1);
        GtkTreeIter child_iter;
        gtk_tree_store_append(g_key_tree_store, &child_iter, &iter);
        gtk_tree_store_set(g_key_tree_store, &child_iter, TREE_COL_NAME, "Loading...", TREE_COL_HKEY_PTR, NULL, TREE_COL_FULL_PATH, "", -1);
        static void add_subkey_to_tree(GtkTreeIter * parent_iter, HKEY parent_hkey, const WCHAR *subkey_name)
        {
            GtkTreeIter child_iter;
            HKEY hSubKey = NULL;
            LONG lResult;
            gchar *parent_path_gstr = get_text_from_iter(GTK_TREE_MODEL(g_key_tree_store), parent_iter, TREE_COL_FULL_PATH);
            gchar *full_path_gstr = g_strdup_printf("%s\\%ls", parent_path_gstr, subkey_name);
            g_free(parent_path_gstr);
            lResult = RegOpenKeyExW(parent_hkey, subkey_name, 0, KEY_READ, &hSubKey);
            gtk_tree_store_append(g_key_tree_store, &child_iter, parent_iter);
            gtk_tree_store_set(g_key_tree_store, &child_iter, TREE_COL_NAME, g_strdup_printf("%ls", subkey_name), TREE_COL_HKEY_PTR, (gpointer)hSubKey, TREE_COL_FULL_PATH, full_path_gstr, -1);
            GtkTreeIter grandchild_iter;
            gtk_tree_store_append(g_key_tree_store, &grandchild_iter, &child_iter);
            gtk_tree_store_set(g_key_tree_store, &grandchild_iter, TREE_COL_NAME, "Loading...", TREE_COL_HKEY_PTR, NULL, TREE_COL_FULL_PATH, "", -1);
            g_free(full_path_gstr);
        }
        static void on_row_expanded(GtkTreeView * tree_view, GtkTreeIter * iter, GtkTreePath * path, gpointer user_data)
        {
            HKEY parent_hkey;
            gchar *key_name;
            gtk_tree_model_get(GTK_TREE_MODEL(g_key_tree_store), iter, TREE_COL_HKEY_PTR, &parent_hkey, TREE_COL_NAME, &key_name, -1);
            if (parent_hkey == NULL)
            {
                g_print("Error: Parent HKEY is NULL for key: %s\n", key_name);
                g_free(key_name);
                return;
            }
            GtkTreeIter child_iter;
            if (gtk_tree_model_iter_children(GTK_TREE_MODEL(g_key_tree_store), &child_iter, iter))
            {
                gchar *child_name;
                gtk_tree_model_get(GTK_TREE_MODEL(g_key_tree_store), &child_iter, TREE_COL_NAME, &child_name, -1);
                if (g_strcmp0(child_name, "Loading...") != 0)
                {
                    g_free(child_name);
                    g_free(key_name);
                    return;
                }
                g_free(child_name);
                gtk_tree_store_remove(g_key_tree_store, &child_iter);
            }
            WCHAR subkey_name[MAX_PATH];
            DWORD subkey_name_len = MAX_PATH;
            DWORD index = 0;
            FILETIME ftLastWriteTime;
            LONG lResult;
            while (TRUE)
            {
                subkey_name_len = MAX_PATH;
                lResult = RegEnumKeyExW(parent_hkey, index, subkey_name, &subkey_name_len, NULL, NULL, NULL, &ftLastWriteTime);
                if (lResult == ERROR_SUCCESS)
                {
                    add_subkey_to_tree(iter, parent_hkey, subkey_name);
                    index++;
                }
                else if (lResult == ERROR_NO_MORE_ITEMS)
                {
                    break;
                }
                else
                {
                    g_print("Error enumerating subkeys for %s: %ld\n", key_name, lResult);
                    break;
                }
            }
            g_free(key_name);
        }
        static void update_status_bar(const gchar *message)
        {
            if (g_status_bar)
            {
                gtk_statusbar_pop(GTK_STATUSBAR(g_status_bar), 0);
                gtk_statusbar_push(GTK_STATUSBAR(g_status_bar), 0, message);
            }
        }
        static gchar *get_current_time_string(void)
        {
            GDateTime *now = g_date_time_new_now_local();
            gchar *time_str = g_date_time_format(now, "%Y-%m-%d %H:%M:%S");
            g_date_time_unref(now);
            return time_str;
        }
        static void display_key_values(HKEY hKey)
        {
            gtk_list_store_clear(g_value_list_store);
            if (hKey == NULL)
            {
                g_print("Attempted to display values for a NULL HKEY.\n");
                update_status_bar("Error: No key selected");
                return;
            }
            update_status_bar("Loading...");
            WCHAR value_name[MAX_PATH];
            DWORD value_name_len;
            DWORD value_type;
            BYTE *value_data = NULL;
            DWORD value_data_len;
            DWORD index = 0;
            LONG lResult;
            while (TRUE)
            {
                value_name_len = MAX_PATH;
                value_data_len = 0;
                lResult = RegEnumValueW(hKey, index, value_name, &value_name_len, NULL, &value_type, NULL, &value_data_len);
                if (lResult == ERROR_SUCCESS || lResult == ERROR_MORE_DATA)
                {
                    if (value_data_len > 0)
                    {
                        value_data = (BYTE *)malloc(value_data_len);
                        if (value_data == NULL)
                        {
                            g_print("Memory allocation failed for value data.\n");
                            goto next_item;
                        }
                    }
                    else
                    {
                        value_data = NULL;
                    }
                    value_name_len = MAX_PATH;
                    l
                        lResult = RegEnumValueW(hKey, index, value_name, &value_name_len, NULL, &value_type, value_data, &value_data_len);
                    if (lResult == ERROR_SUCCESS)
                    {
                        GtkTreeIter iter;
                        gchar *type_str = NULL;
                        gchar *data_str = NULL;
                        switch (value_type)
                        {
                        case REG_SZ:
                        case REG_EXPAND_SZ:
                            type_str = g_strdup_printf("%s", (value_type == REG_SZ) ? "REG_SZ" : "REG_EXPAND_SZ");
                            data_str = g_strdup_printf("%ls", (WCHAR *)value_data);
                            break;
                        case REG_DWORD:
                            type_str = g_strdup("REG_DWORD");
                            if (value_data && value_data_len >= sizeof(DWORD))
                            {
                                data_str = g_strdup_printf("0x%08X (%lu)", *(DWORD *)value_data, *(DWORD *)value_data);
                            }
                            else
                            {
                                data_str = g_strdup("(empty)");
                            }
                            break;
                        case REG_QWORD:
                            type_str = g_strdup("REG_QWORD");
                            if (value_data && value_data_len >= sizeof(ULONGLONG))
                            {
                                data_str = g_strdup_printf("0x%016llX (%llu)", *(ULONGLONG *)value_data, *(ULONGLONG *)value_data);
                            }
                            else
                            {
                                data_str = g_strdup("(empty)");
                            }
                            break;
                        case REG_BINARY:
                            type_str = g_strdup("REG_BINARY");
                            if (value_data_len > 0)
                            {
                                GString *hex_str = g_string_new("");
                                for (DWORD i = 0; i < value_data_len; i++)
                                {
                                    g_string_append_printf(hex_str, "%02X ", value_data[i]);
                                }
                                data_str = g_string_free(hex_str, FALSE);
                            }
                            else
                            {
                                data_str = g_strdup("(zero-length binary value)");
                            }
                            break;
                        default:
                            type_str = g_strdup_printf("Unknown Type (%lu)", value_type);
                            data_str = g_strdup_printf("Binary Data (Length: %lu)", value_data_len);
                            break;
                        }
                        gtk_list_store_append(g_value_list_store, &iter);
                        gtk_list_store_set(g_value_list_store, &iter, VALUE_COL_NAME, g_strdup_printf("%ls", value_name), VALUE_COL_TYPE_STR, type_str ? type_str : g_strdup(""), VALUE_COL_DATA_STR, data_str ? data_str : g_strdup(""), VALUE_COL_TYPE_RAW, value_type, VALUE_COL_DATA_RAW, value_data, -1);
                        g_free(type_str);
                        g_free(data_str);
                    }
                    else
                    {
                        g_print("Error getting data for value %ls: %ld\n", value_name, lResult);
                    }
                    if (value_data)
                    {
                        free(value_data);
                        value_data = NULL;
                    }
                }
                else if (lResult == ERROR_NO_MORE_ITEMS)
                {
                    break;
                }
                else
                {
                    g_print("Error enumerating values: %ld\n", lResult);
                    break;
                }
            next_item:
                index++;
            }
            gchar *time_str = get_current_time_string();
            gchar *status_msg = g_strdup_printf("Ready - %s", time_str);
            update_status_bar(status_msg);
            g_free(status_msg);
            g_free(time_str);
        }
        static void on_key_tree_selection_changed(GtkTreeSelection * selection, gpointer user_data)
        {
            GtkTreeModel *model;
            GtkTreeIter iter;
            if (gtk_tree_selection_get_selected(selection, &model, &iter))
            {
                HKEY hKey;
                gchar *key_name;
                gchar *full_path;

                gtk_tree_model_get(model, &iter, TREE_COL_HKEY_PTR, &hKey, TREE_COL_NAME, &key_name, TREE_COL_FULL_PATH, &full_path, -1);
                g_print("Selected Key: %s (Path: %s, HKEY: %p)\n", key_name, full_path, hKey);
                g_current_selected_key = hKey;
                display_key_values(hKey);
                g_free(key_name);
                g_free(full_path);
            }
        }
        static gboolean auto_refresh_callback(gpointer user_data)
        {
            if (g_current_selected_key != NULL)
            {
                g_print("Auto-refreshing registry values...\n");
                display_key_values(g_current_selected_key);
            }
            return TRUE;
        }
        static void on_refresh_button_clicked(GtkButton * button, gpointer user_data)
        {
            if (g_current_selected_key != NULL)
            {
                g_print("Manual refresh triggered...\n");
                display_key_values(g_current_selected_key);
            }
            else
            {
                update_status_bar("No key selected for refresh");
            }
        }
        static void start_auto_refresh(void)
        {
            if (g_refresh_timer_id == 0)
            {
                g_refresh_timer_id = g_timeout_add_seconds(5, auto_refresh_callback, NULL);
                update_status_bar("Auto-refresh ON");
            }
        }
        static void stop_auto_refresh(void)
        {
            if (g_refresh_timer_id != 0)
            {
                g_source_remove(g_refresh_timer_id);
                g_refresh_timer_id = 0;
                update_status_bar("Auto-refresh OFF");
            }
        }
        static void toggle_auto_refresh(GtkToggleButton * button, gpointer user_data)
        {
            if (gtk_toggle_button_get_active(button))
            {
                start_auto_refresh();
            }
            else
            {
                stop_auto_refresh();
            }
        }
        static void show_safety_notice(GtkWindow * parent)
        {
            GtkWidget *dialog;
            GtkWidget *content_area;
            GtkWidget *label;
            dialog = gtk_dialog_new_with_buttons("⚠️ SAFETY NOTICE - Registry Viewer", parent, GTK_DIALOG_MODAL, "I Understand", GTK_RESPONSE_ACCEPT, NULL);
            content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
            label = gtk_label_new(
                "READ-ONLY MODE ACTIVATED\n\n"
                "This Registry Viewer operates in SAFE READ-ONLY mode:\n\n"
                "You can VIEW registry values\n"
                "You can REFRESH data in real-time\n"
                "You CANNOT edit registry values\n"
                "You CANNOT delete registry keys\n"
                "You CANNOT modify system settings\n\n"
                "This prevents accidental system damage.\n"
                "Your PC/Laptop is protected from registry corruption.");
            gtk_label_set_wrap(GTK_LABEL(label), TRUE);
            gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_CENTER);
            gtk_widget_set_margin_top(label, 20);
            gtk_widget_set_margin_bottom(label, 20);
            gtk_widget_set_margin_start(label, 20);
            gtk_widget_set_margin_end(label, 20);
            gtk_box_append(GTK_BOX(content_area), label);
            gtk_widget_set_visible(dialog, TRUE);
            g_signal_connect(dialog, "response", G_CALLBACK(gtk_window_destroy), NULL);
            gtk_window_present(GTK_WINDOW(dialog));
        }
        static GtkWidget *create_key_tree_view(void)
        {
            GtkWidget *tree_view;
            GtkCellRenderer *renderer;
            GtkTreeViewColumn *column;
            g_key_tree_store = gtk_tree_store_new(TREE_N_COLUMNS, G_TYPE_STRING, G_TYPE_POINTER, G_TYPE_STRING);
            tree_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(g_key_tree_store));
            g_object_unref(g_key_tree_store);
            g_key_tree_view = GTK_TREE_VIEW(tree_view);
            gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(tree_view), FALSE);
            gtk_tree_view_set_enable_tree_lines(GTK_TREE_VIEW(tree_view), TRUE);
            gtk_tree_view_set_show_expanders(GTK_TREE_VIEW(tree_view), TRUE);
            renderer = gtk_cell_renderer_text_new();
            column = gtk_tree_view_column_new_with_attributes("Key", renderer, "text", TREE_COL_NAME, NULL);
            gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), column);
            add_root_key("HKEY_CLASSES_ROOT", HKEY_CLASSES_ROOT);
            add_root_key("HKEY_CURRENT_USER", HKEY_CURRENT_USER);
            add_root_key("HKEY_LOCAL_MACHINE", HKEY_LOCAL_MACHINE);
            add_root_key("HKEY_USERS", HKEY_USERS);
            add_root_key("HKEY_CURRENT_CONFIG", HKEY_CURRENT_CONFIG);
            GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree_view));
            g_signal_connect(selection, "changed", G_CALLBACK(on_key_tree_selection_changed), NULL);
            g_signal_connect(tree_view, "row-expanded", G_CALLBACK(on_row_expanded), NULL);
            return gtk_scrolled_window_new();
        }
        static GtkWidget *create_value_list_view(void)
        {
            GtkWidget *tree_view;
            GtkCellRenderer *renderer;
            GtkTreeViewColumn *column;
            g_value_list_store = gtk_list_store_new(VALUE_N_COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_UINT, G_TYPE_POINTER);
            tree_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(g_value_list_store));
            g_object_unref(g_value_list_store);
            g_value_tree_view = GTK_TREE_VIEW(tree_view);
            gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(tree_view), TRUE);
            renderer = gtk_cell_renderer_text_new();
            column = gtk_tree_view_column_new_with_attributes("Name", renderer, "text", VALUE_COL_NAME, NULL);
            gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), column);
            gtk_tree_view_column_set_resizable(column, TRUE);
            gtk_tree_view_column_set_min_width(column, 150);
            renderer = gtk_cell_renderer_text_new();
            column = gtk_tree_view_column_new_with_attributes("Type", renderer, "text", VALUE_COL_TYPE_STR, NULL);
            gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), column);
            gtk_tree_view_column_set_resizable(column, TRUE);
            gtk_tree_view_column_set_min_width(column, 100);
            renderer = gtk_cell_renderer_text_new();
            column = gtk_tree_view_column_new_with_attributes("Data", renderer, "text", VALUE_COL_DATA_STR, NULL);
            gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), column);
            gtk_tree_view_column_set_resizable(column, TRUE);
            gtk_tree_view_column_set_min_width(column, 300);
            return gtk_scrolled_window_new();
        }
        static void activate(GtkApplication * app, gpointer user_data)
        {
            GtkWidget *window;
            GtkWidget *main_box;
            GtkWidget *toolbar;
            GtkWidget *paned;
            GtkWidget *left_scrolled_window;
            GtkWidget *right_scrolled_window;
            window = gtk_application_window_new(app);
            gtk_window_set_title(GTK_WINDOW(window), "Windows Registry Viewer ");
            gtk_window_set_default_size(GTK_WINDOW(window), 1024, 768);
            main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
            gtk_window_set_child(GTK_WINDOW(window), main_box);
            toolbar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
            gtk_widget_set_margin_start(toolbar, 5);
            gtk_widget_set_margin_end(toolbar, 5);
            gtk_widget_set_margin_top(toolbar, 5);
            gtk_widget_set_margin_bottom(toolbar, 5);
            GtkWidget *separator = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
            gtk_box_append(GTK_BOX(toolbar), separator);
            GtkWidget *safety_label = gtk_label_new(" READ-ONLY MODE ");
            gtk_widget_set_tooltip_text(safety_label, "This application is in read-only mode for safety. Registry modifications are disabled.");
            gtk_widget_add_css_class(safety_label, "warning");
            gtk_widget_set_halign(safety_label, GTK_ALIGN_START);
            gtk_box_append(GTK_BOX(toolbar), safety_label);
            GtkWidget *separator2 = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
            gtk_box_append(GTK_BOX(toolbar), separator2);
            GtkWidget *status_label = gtk_label_new("Status: Ready (Read-Only)");
            gtk_widget_set_halign(status_label, GTK_ALIGN_START);
            gtk_box_append(GTK_BOX(toolbar), status_label);
            gtk_box_append(GTK_BOX(main_box), toolbar);
            paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
            gtk_box_append(GTK_BOX(main_box), paned);
            gtk_widget_set_vexpand(paned, TRUE);
            left_scrolled_window = create_key_tree_view();
            gtk_paned_set_start_child(GTK_PANED(paned), left_scrolled_window);
            gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(left_scrolled_window), GTK_WIDGET(g_key_tree_view));
            gtk_widget_set_hexpand(left_scrolled_window, TRUE);
            gtk_widget_set_vexpand(left_scrolled_window, TRUE);
            right_scrolled_window = create_value_list_view();
            gtk_paned_set_end_child(GTK_PANED(paned), right_scrolled_window);
            gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(right_scrolled_window), GTK_WIDGET(g_value_tree_view));
            gtk_widget_set_hexpand(right_scrolled_window, TRUE);
            gtk_widget_set_vexpand(right_scrolled_window, TRUE);
            gtk_paned_set_position(GTK_PANED(paned), 300);
            g_status_bar = gtk_statusbar_new();
            gtk_box_append(GTK_BOX(main_box), g_status_bar);
            update_status_bar("Ready");
            gtk_widget_set_visible(window, TRUE);
        }
        static void cleanup_resources(void)
        {
            if (g_refresh_timer_id != 0)
            {
                g_source_remove(g_refresh_timer_id);
                g_refresh_timer_id = 0;
            }
            if (g_current_selected_key != NULL)
            {
                RegCloseKey(g_current_selected_key);
                g_current_selected_key = NULL;
            }
        }
        int main(int argc, char **argv)
        {
            GtkApplication *app;
            int status;
            app = gtk_application_new("org.example.RegistryViewer", G_APPLICATION_DEFAULT_FLAGS);
            g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
            status = g_application_run(G_APPLICATION(app), argc, argv);
            cleanup_resources();
            g_object_unref(app);
            return status;
        }