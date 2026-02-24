/**
 * @file registry_facade.h
 * @brief Facade for all Windows Registry read operations.
 *
 * Provides a clean, safe (read-only) interface over the raw Win32
 * Registry API. No writes, no deletes – ever.
 *
 * Course:     <Course Name>
 * University: <University Name>
 * Author:     <Your Name>
 * Date:       2026-02-17
 */

#ifndef REGISTRY_FACADE_H
#define REGISTRY_FACADE_H

#include <windows.h>
#include <glib.h>

/* ─────────────────────────────────────────────
 * Value descriptor (owned by caller, free with
 * registry_value_free())
 * ───────────────────────────────────────────── */
typedef struct RegistryValue {
    gchar  *name;       /**< Value name (UTF-8)                        */
    DWORD   type;       /**< Raw REG_* constant                        */
    gchar  *type_str;   /**< Human-readable type string                */
    gchar  *data_str;   /**< Human-readable data string                */
    BYTE   *raw_data;   /**< Raw bytes; may be NULL for empty values   */
    DWORD   raw_size;   /**< Byte length of raw_data                   */
} RegistryValue;

/* ─────────────────────────────────────────────
 * Public API
 * ───────────────────────────────────────────── */

/**
 * Open a subkey of @p parent in read-only mode.
 *
 * @param parent     An already-open HKEY (or a predefined root HKEY).
 * @param subkey_w   Wide-char subkey name.
 * @param[out] out   Receives the opened handle on success.
 * @return           ERROR_SUCCESS or a Win32 error code.
 */
LONG registry_open_key (HKEY parent, const WCHAR *subkey_w, HKEY *out);

/**
 * Enumerate all direct subkey names under @p hkey.
 *
 * @param hkey   An open, readable HKEY.
 * @return       NULL-terminated array of wide-char strings.
 *               Caller must free each element and the array itself,
 *               or use registry_free_subkey_names().
 */
WCHAR **registry_enum_subkeys (HKEY hkey);

/**
 * Free the array returned by registry_enum_subkeys().
 */
void registry_free_subkey_names (WCHAR **names);

/**
 * Enumerate all values under @p hkey.
 *
 * @param hkey       An open, readable HKEY.
 * @param[out] count Number of values returned.
 * @return           Heap-allocated array of RegistryValue structs,
 *                   or NULL on error. Free with registry_free_values().
 */
RegistryValue *registry_enum_values (HKEY hkey, DWORD *count);

/**
 * Free the array returned by registry_enum_values().
 */
void registry_free_values (RegistryValue *values, DWORD count);

/**
 * Close a handle obtained via registry_open_key().
 * Safe to call with NULL.
 */
void registry_close_key (HKEY hkey);

#endif /* REGISTRY_FACADE_H */
