/**
 * @file registry_facade.c
 * @brief Implementation of the read-only Windows Registry Facade.
 *
 * All Win32 Registry API calls are centralised here.  The rest of
 * the application never calls RegXxx() functions directly, making
 * the blast-radius of any API change minimal.
 *
 * Course:     <Course Name>
 * University: <University Name>
 * Author:     <Your Name>
 * Date:       2026-02-17
 */

#include "registry_facade.h"
#include <stdio.h>   /* fprintf, stderr */
#include <stdlib.h>  /* malloc, free    */

/* ─────────────────────────────────────────────────────────────────
 * Internal helpers
 * ───────────────────────────────────────────────────────────────── */

/** Convert a wide-char string to a new UTF-8 GLib string. */
static gchar *wchar_to_utf8 (const WCHAR *ws)
{
    if (!ws) return g_strdup ("");
    return g_utf16_to_utf8 ((const gunichar2 *)ws, -1, NULL, NULL, NULL);
}

/** Format raw registry bytes as "XX XX XX …" hex string. */
static gchar *bytes_to_hex (const BYTE *data, DWORD len)
{
    GString *buf = g_string_sized_new (len * 3 + 1);
    for (DWORD i = 0; i < len; i++)
        g_string_append_printf (buf, "%02X ", data[i]);
    /* Remove trailing space */
    if (buf->len > 0)
        g_string_truncate (buf, buf->len - 1);
    return g_string_free (buf, FALSE);
}

/** Build a human-readable type string for a REG_* constant. */
static const gchar *type_to_string (DWORD type)
{
    switch (type) {
        case REG_SZ:        return "REG_SZ";
        case REG_EXPAND_SZ: return "REG_EXPAND_SZ";
        case REG_BINARY:    return "REG_BINARY";
        case REG_DWORD:     return "REG_DWORD";
        case REG_QWORD:     return "REG_QWORD";
        case REG_MULTI_SZ:  return "REG_MULTI_SZ";
        case REG_NONE:      return "REG_NONE";
        default:            return "REG_UNKNOWN";
    }
}

/** Convert raw registry bytes to a printable data string. */
static gchar *format_value_data (DWORD type, const BYTE *data, DWORD len)
{
    if (!data || len == 0)
        return g_strdup ("(empty)");

    switch (type) {
        case REG_SZ:
        case REG_EXPAND_SZ:
            return wchar_to_utf8 ((const WCHAR *)data);

        case REG_DWORD:
            if (len >= sizeof (DWORD)) {
                DWORD v = *(const DWORD *)data;
                return g_strdup_printf ("0x%08X  (%lu)", v, (unsigned long)v);
            }
            return g_strdup ("(invalid DWORD)");

        case REG_QWORD:
            if (len >= sizeof (ULONGLONG)) {
                ULONGLONG v = *(const ULONGLONG *)data;
                return g_strdup_printf ("0x%016llX  (%llu)", v, v);
            }
            return g_strdup ("(invalid QWORD)");

        case REG_BINARY:
            return bytes_to_hex (data, len);

        case REG_MULTI_SZ: {
            GString *buf = g_string_new ("");
            const WCHAR *p = (const WCHAR *)data;
            while (*p) {
                gchar *s = wchar_to_utf8 (p);
                g_string_append (buf, s);
                g_string_append (buf, "  |  ");
                g_free (s);
                p += wcslen (p) + 1;
            }
            return g_string_free (buf, FALSE);
        }

        default:
            return bytes_to_hex (data, len);
    }
}

/* ─────────────────────────────────────────────────────────────────
 * Public API implementation
 * ───────────────────────────────────────────────────────────────── */

LONG registry_open_key (HKEY parent, const WCHAR *subkey_w, HKEY *out)
{
    g_return_val_if_fail (out != NULL, ERROR_INVALID_PARAMETER);
    *out = NULL;
    return RegOpenKeyExW (parent, subkey_w, 0, KEY_READ, out);
}

WCHAR **registry_enum_subkeys (HKEY hkey)
{
    g_return_val_if_fail (hkey != NULL, NULL);

    /* Pre-allocate a growable GPtrArray, then transfer to a plain array. */
    GPtrArray *arr = g_ptr_array_new ();
    WCHAR     name[MAX_PATH];
    DWORD     name_len;
    DWORD     index = 0;
    LONG      rc;

    for (;;) {
        name_len = MAX_PATH;
        rc = RegEnumKeyExW (hkey, index, name, &name_len,
                            NULL, NULL, NULL, NULL);
        if (rc == ERROR_NO_MORE_ITEMS) break;
        if (rc != ERROR_SUCCESS) {
            fprintf (stderr, "[facade] RegEnumKeyExW index=%lu rc=%ld\n",
                     (unsigned long)index, (long)rc);
            break;
        }
        g_ptr_array_add (arr, _wcsdup (name)); /* caller-owned copy */
        index++;
    }

    g_ptr_array_add (arr, NULL); /* NULL sentinel */
    return (WCHAR **)g_ptr_array_free (arr, FALSE);
}

void registry_free_subkey_names (WCHAR **names)
{
    if (!names) return;
    for (WCHAR **p = names; *p; p++)
        free (*p); /* allocated with _wcsdup / malloc */
    g_free (names);
}

RegistryValue *registry_enum_values (HKEY hkey, DWORD *count)
{
    g_return_val_if_fail (hkey  != NULL, NULL);
    g_return_val_if_fail (count != NULL, NULL);
    *count = 0;

    /* Query how many values exist and the max name / data sizes. */
    DWORD num_values   = 0;
    DWORD max_name_len = 0;
    DWORD max_data_len = 0;
    LONG  rc = RegQueryInfoKeyW (hkey, NULL, NULL, NULL, NULL, NULL, NULL,
                                 &num_values, &max_name_len,
                                 &max_data_len, NULL, NULL);
    if (rc != ERROR_SUCCESS || num_values == 0)
        return NULL;

    RegistryValue *values = g_new0 (RegistryValue, num_values);
    WCHAR *name_buf       = g_new0 (WCHAR, max_name_len + 2);
    BYTE  *data_buf       = g_malloc0 (max_data_len + 4);
    DWORD  filled         = 0;

    for (DWORD i = 0; i < num_values; i++) {
        DWORD name_len = max_name_len + 1;
        DWORD data_len = max_data_len + 2;
        DWORD type     = 0;

        rc = RegEnumValueW (hkey, i,
                            name_buf, &name_len,
                            NULL, &type,
                            data_buf, &data_len);
        if (rc != ERROR_SUCCESS) {
            fprintf (stderr, "[facade] RegEnumValueW index=%lu rc=%ld\n",
                     (unsigned long)i, (long)rc);
            continue;
        }

        RegistryValue *v = &values[filled++];
        v->name     = wchar_to_utf8 (name_buf);
        v->type     = type;
        v->type_str = g_strdup (type_to_string (type));
        v->data_str = format_value_data (type, data_buf, data_len);

        if (data_len > 0) {
            v->raw_data = g_memdup2 (data_buf, data_len);
            v->raw_size = data_len;
        }
    }

    g_free (name_buf);
    g_free (data_buf);
    *count = filled;
    return values;
}

void registry_free_values (RegistryValue *values, DWORD count)
{
    if (!values) return;
    for (DWORD i = 0; i < count; i++) {
        g_free (values[i].name);
        g_free (values[i].type_str);
        g_free (values[i].data_str);
        g_free (values[i].raw_data);
    }
    g_free (values);
}

void registry_close_key (HKEY hkey)
{
    if (hkey) RegCloseKey (hkey);
}
