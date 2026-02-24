# TECHNICAL DESIGN DOCUMENT
## Windows Registry Viewer
### MVC + Facade Architecture

---

| Field        | Detail                                  |
|--------------|-----------------------------------------|
| Project      | Safe Read-Only Windows Registry Viewer  |
| Language     | C11 (GTK 4 + Win32 API)                 |
| Architecture | MVC + Facade Pattern                    |
| Version      | 1.0.0                                   |
| Date         | 2026-02-17                              |
| Author       | \<TOWHID AL MAHMUD & ABIR KHAN SIAM\>                           |
| Course       | \<0714 02 CSE 2100\>                         |
| University   | \<Khulna University\>                     |

> *CONFIDENTIAL – FOR ACADEMIC USE ONLY*

---

## Table of Contents

1. [System Architecture](#1-system-architecture)
2. [Module Breakdown](#2-module-breakdown)
3. [Data Structure Design](#3-data-structure-design)
4. [Registry Tree Structure Management](#4-registry-tree-structure-management)
5. [Best Programming Practices](#5-best-programming-practices)
6. [Design Patterns](#6-design-patterns)
7. [Build Configuration & Testing](#7-build-configuration--testing)

---

## 1. System Architecture

### 1.1 High-Level Architecture Diagram

The application follows a layered MVC + Facade architecture. Each layer has a single, clearly defined responsibility, communicating only with the layer directly adjacent to it.

```
┌─────────────────────────────────────────────────────────┐
│                    USER INTERACTION                     │
│              (Mouse clicks, keyboard input)             │
└───────────────────────────┬─────────────────────────────┘
                            │
┌───────────────────────────▼─────────────────────────────┐
│                 CONTROLLER LAYER                        │
│         registry_controller.c / registry_view.c        │
│  • GTK signal handlers (on_selection_changed, etc.)     │
│  • Auto-refresh timer management                        │
│  • Widget construction & layout (View sub-layer)        │
└────────────┬──────────────────────────┬─────────────────┘
             │                          │
┌────────────▼────────────┐  ┌──────────▼─────────────────┐
│      MODEL LAYER        │  │       VIEW LAYER           │
│  registry_model.c       │  │   registry_view.c          │
│  • Populate GtkTreeStore│  │   • Build GTK widgets      │
│  • Populate GtkListStore│  │   • Status bar updates     │
│  • Lazy expansion logic │  │   • Safety notice dialog   │
└────────────┬────────────┘  └────────────────────────────┘
             │
┌────────────▼────────────────────────────────────────────┐
│                  FACADE LAYER                           │
│               registry_facade.c                        │
│  • registry_open_key()    • registry_enum_subkeys()    │
│  • registry_enum_values() • registry_close_key()       │
│  • All Win32 RegXxx() calls isolated here              │
└────────────┬────────────────────────────────────────────┘
             │
┌────────────▼────────────────────────────────────────────┐
│              WINDOWS REGISTRY API (Win32)               │
│    RegOpenKeyExW  RegEnumKeyExW  RegEnumValueW etc.     │
│                   (READ-ONLY ACCESS)                   │
└─────────────────────────────────────────────────────────┘
```

---

### 1.2 File Structure

```
RegistryViewer/
├── include/                  ← Shared headers (interface contracts)
│   ├── registry_types.h      → AppState struct, enums, constants
│   ├── registry_facade.h     → Win32 wrapper interface
│   ├── registry_model.h      → GTK store population interface
│   ├── registry_view.h       → Widget builder interface
│   └── registry_controller.h → Signal handler interface
│
├── src/
│   ├── main.c               → Entry point, MVC wiring
│   └── registry_facade.c    → ALL Win32 Registry calls
│
├── model/
│   └── registry_model.c     → Facade data → GTK stores
│
├── ui/
│   └── registry_view.c      → GTK widget construction only
│
├── controller/
│   └── registry_controller.c → GTK signals & timer logic
│
└── CMakeLists.txt           → CMake build configuration
```

---

### 1.3 Layer Communication Flow

Data and control flow strictly in one direction per operation, preventing circular dependencies:

```
KEY SELECTION EVENT:
  User clicks tree row
      → GTK fires 'selection::changed' signal
      → Controller: on_selection_changed()
      → Model:      model_load_values(state, hkey)
      → Facade:     registry_enum_values(hkey, &count)
      → Win32:      RegEnumValueW()
      ← Facade returns RegistryValue[]
      ← Model populates GtkListStore rows
      ← GTK re-renders the value pane automatically

TREE EXPANSION EVENT:
  User clicks expander arrow
      → GTK fires 'row-expanded' signal
      → Controller: on_row_expanded()
      → Model:      model_expand_node(state, iter)
      → Facade:     registry_enum_subkeys(hkey)
      → Win32:      RegEnumKeyExW()
      ← Facade returns WCHAR*[]
      ← Model inserts child rows into GtkTreeStore
```

---

## 2. Module Breakdown

### 2.1 GUI Module (registry_view.c / registry_view.h)

The GUI module is responsible exclusively for constructing and laying out GTK widgets. It contains zero business logic, zero Win32 calls, and zero registry data — keeping it purely declarative.

| Component       | GTK Widget                      | Purpose                                         |
|-----------------|---------------------------------|-------------------------------------------------|
| Main Window     | GtkApplicationWindow            | Root container, 1100×750 px default             |
| Toolbar         | GtkBox (horizontal)             | Read-only badge + status hint                   |
| Split Pane      | GtkPaned                        | Adjustable left/right divider (default 310 px)  |
| Key Tree View   | GtkTreeView + GtkTreeStore      | Hierarchical display of registry keys           |
| Value List View | GtkTreeView + GtkListStore      | Tabular display of values for selected key      |
| Status Bar      | GtkStatusbar                    | One-line timestamped status messages            |
| Safety Dialog   | GtkDialog (modal)               | Read-only safety notice on first launch         |

#### GUI Column Layout

| Column | Model Index        | Min Width | Data Shown                   |
|--------|--------------------|-----------|------------------------------|
| Name   | VALUE_COL_NAME     | 150 px    | Registry value name (UTF-8)  |
| Type   | VALUE_COL_TYPE_STR | 130 px    | e.g. REG_SZ, REG_DWORD       |
| Data   | VALUE_COL_DATA_STR | 300 px    | Human-readable value content |

---

### 2.2 Registry Access Module (registry_facade.c / registry_facade.h)

The Facade is the only file in the project that touches the Win32 Registry API. This centralisation means: auditing safety is trivial, swapping backends requires changing one file only, and the rest of the codebase stays platform-agnostic.

| Function                     | Win32 Calls Used                        | Description                          |
|------------------------------|-----------------------------------------|--------------------------------------|
| `registry_open_key()`        | `RegOpenKeyExW(KEY_READ)`               | Open subkey read-only; returns HKEY  |
| `registry_enum_subkeys()`    | `RegEnumKeyExW()`                       | List all direct child key names      |
| `registry_enum_values()`     | `RegQueryInfoKeyW()` + `RegEnumValueW()`| Return typed RegistryValue array     |
| `registry_close_key()`       | `RegCloseKey()`                         | Safe close; NULL-tolerant            |
| `registry_free_subkey_names()` | —                                     | Free WCHAR\*\* from enum_subkeys     |
| `registry_free_values()`     | —                                       | Free RegistryValue[] from enum_values|

> **IMPORTANT:** The access flag `KEY_READ` is the only access mode used throughout the entire codebase. `KEY_WRITE`, `KEY_ALL_ACCESS`, and `KEY_SET_VALUE` are never requested, making write operations physically impossible at the OS level.

---

### 2.3 File Export Module (Future Extension)

The current version is a viewer only. A File Export module can be added cleanly in a future iteration without touching existing layers. The recommended design is:

| Export Format | Suggested Library       | Output Description                          |
|---------------|-------------------------|---------------------------------------------|
| CSV           | Standard stdio (fprintf)| Name, Type, Data columns — one value per row|
| JSON          | cJSON or jansson        | Nested key/value tree structure             |
| REG           | Custom serialiser       | Windows .reg import-compatible format       |
| HTML          | Custom string builder   | Self-contained browser-viewable report      |

Because the Facade already returns a clean `RegistryValue[]` array, the export module only needs to iterate that array and write to a file — no Win32 interaction needed.

---

### 2.4 Error Handling Module

Error handling follows a consistent three-tier strategy across all layers:

| Layer      | Strategy                                                        | Example                                             |
|------------|-----------------------------------------------------------------|-----------------------------------------------------|
| Facade     | Return Win32 LONG error codes; log to stderr via fprintf        | `ERROR_ACCESS_DENIED` → log + return NULL           |
| Model      | Defensive NULL checks with `g_return_if_fail` / `g_return_val_if_fail` | NULL HKEY → early return, no crash       |
| Controller | Status bar feedback to user; no crash-on-error policy          | No key selected → `view_set_status("No key selected")` |
| Memory     | Every malloc/g_new0 paired with matching free/g_free            | `registry_free_values()` cleans all fields          |

```c
/* Facade error handling pattern */
WCHAR **registry_enum_subkeys(HKEY hkey) {
    if (rc == ERROR_NO_MORE_ITEMS) break;          // normal exit
    if (rc != ERROR_SUCCESS) {
        fprintf(stderr, "[facade] error rc=%ld\n", rc);
        break;                                     // graceful degradation
    }
}

/* Model defensive check pattern */
void model_expand_node(AppState *state, GtkTreeIter *iter) {
    g_return_if_fail(state != NULL);
    g_return_if_fail(iter  != NULL);
    // ... safe to proceed
}
```

---

## 3. Data Structure Design

### 3.1 AppState — Central Application State

`AppState` replaces all global variables from the original single-file implementation. It is stack-allocated in `main()` and passed by pointer to every layer, making the application fully re-entrant and easier to unit-test.

```c
typedef struct AppState {
    /* ── Models (GTK data stores) ───────────────── */
    GtkTreeStore  *key_tree_store;   // Left pane: hierarchical keys
    GtkListStore  *value_list_store; // Right pane: flat value list

    /* ── View widgets ───────────────────────────── */
    GtkTreeView   *key_tree_view;    // Left pane widget
    GtkTreeView   *value_tree_view;  // Right pane widget
    GtkWidget     *status_bar;       // Bottom status bar
    GtkWidget     *main_window;      // Top-level window

    /* ── Controller state ───────────────────────── */
    HKEY           current_hkey;     // Currently displayed key
    guint          refresh_timer_id; // 0 = timer not running
} AppState;
```

---

### 3.2 RegistryValue — Value Descriptor

The `RegistryValue` struct carries both raw and pre-formatted representations of a single registry value entry, eliminating repeated formatting work in the rendering path.

```c
typedef struct RegistryValue {
    gchar  *name;       // Value name in UTF-8 (g_free when done)
    DWORD   type;       // Raw REG_SZ / REG_DWORD / REG_BINARY / ...
    gchar  *type_str;   // Human-readable: "REG_SZ", "REG_DWORD", ...
    gchar  *data_str;   // Formatted: "0x0000001A (26)" for DWORD
    BYTE   *raw_data;   // Original bytes from RegEnumValueW()
    DWORD   raw_size;   // Byte length of raw_data
} RegistryValue;
```

---

### 3.3 GTK Tree Column Enums

Integer column indices for the two GTK stores are defined as typed enums in `registry_types.h`, preventing magic-number bugs:

| Enum                 | Value | GType          | Content                          |
|----------------------|-------|----------------|----------------------------------|
| `TREE_COL_NAME`      | 0     | G_TYPE_STRING  | Display name of the key          |
| `TREE_COL_HKEY_PTR`  | 1     | G_TYPE_POINTER | HKEY handle (opened with KEY_READ)|
| `TREE_COL_FULL_PATH` | 2     | G_TYPE_STRING  | Full path e.g. HKLM\\SOFTWARE\\...|
| `VALUE_COL_NAME`     | 0     | G_TYPE_STRING  | Value name                       |
| `VALUE_COL_TYPE_STR` | 1     | G_TYPE_STRING  | Human-readable type              |
| `VALUE_COL_DATA_STR` | 2     | G_TYPE_STRING  | Human-readable data              |
| `VALUE_COL_TYPE_RAW` | 3     | G_TYPE_UINT    | Raw DWORD type constant          |
| `VALUE_COL_DATA_RAW` | 4     | G_TYPE_POINTER | Raw byte pointer (future use)    |

---

### 3.4 Supported REG_* Types

| Registry Type  | DWORD Value | Formatted Output Example              |
|----------------|-------------|---------------------------------------|
| REG_SZ         | 1           | Hello, World!                         |
| REG_EXPAND_SZ  | 2           | %SystemRoot%\system32                 |
| REG_BINARY     | 3           | DE AD BE EF 00 1A                     |
| REG_DWORD      | 4           | 0x00000001  (1)                       |
| REG_QWORD      | 11          | 0x00000000FFFFFFFF  (4294967295)      |
| REG_MULTI_SZ   | 7           | Value1  \|  Value2  \|  Value3        |
| REG_NONE       | 0           | (empty)                               |
| Unknown        | —           | XX XX XX … (hex dump)                 |

---

## 4. Registry Tree Structure Management

### 4.1 Lazy Loading Strategy

Windows registry hives can contain millions of subkeys. Loading the entire tree upfront would cause a multi-second freeze on startup. The application uses a lazy expansion pattern: every node starts with a single placeholder child, which is replaced with real data only when the user actually expands that node.

```
STARTUP:                     USER EXPANDS HKEY_LOCAL_MACHINE:

▼ HKEY_CLASSES_ROOT          ▼ HKEY_CLASSES_ROOT
  └── Loading…                 └── Loading…
▶ HKEY_LOCAL_MACHINE         ▼ HKEY_LOCAL_MACHINE   ← expanded
  └── Loading…                 ├── BCD00000000
▶ HKEY_CURRENT_USER            │     └── Loading…
  └── Loading…                 ├── HARDWARE
▶ HKEY_USERS                   │     └── Loading…
  └── Loading…                 ├── SAM
▶ HKEY_CURRENT_CONFIG          │     └── Loading…
  └── Loading…                 └── SOFTWARE
                                     └── Loading…
```

---

### 4.2 Expansion Algorithm (model_expand_node)

```c
void model_expand_node(AppState *state, GtkTreeIter *parent_iter) {
    // 1. Guard: abort if HKEY is NULL (placeholder row or error)
    HKEY parent_hkey = NULL;
    gtk_tree_model_get(..., TREE_COL_HKEY_PTR, &parent_hkey, -1);
    if (!parent_hkey) return;

    // 2. Check first child: is it still the placeholder?
    GtkTreeIter first_child;
    gtk_tree_model_iter_children(..., &first_child, parent_iter);
    gchar *name = NULL;
    gtk_tree_model_get(..., &first_child, TREE_COL_NAME, &name, -1);
    if (strncmp(name, "Loading", 7) != 0) return; // Already real

    // 3. Remove placeholder
    gtk_tree_store_remove(state->key_tree_store, &first_child);

    // 4. Enumerate via Facade and insert real rows
    WCHAR **subkeys = registry_enum_subkeys(parent_hkey);
    for (WCHAR **p = subkeys; *p; p++)
        append_subkey_row(state, parent_iter, parent_hkey, *p);
    registry_free_subkey_names(subkeys);
}
```

---

### 4.3 HKEY Lifetime Management

Registry key handles (HKEY) have a specific ownership model that must be carefully observed to avoid handle leaks or double-closes:

| HKEY Type                    | Owner      | Open Location          | Close Location              |
|------------------------------|------------|------------------------|-----------------------------|
| Predefined root (HKLM, HKCU…)| Windows OS | Never opened           | NEVER closed — OS constant  |
| Subkey HKEY                  | Tree row   | `append_subkey_row()`  | OS reclaims on process exit |
| current_hkey                 | AppState   | `on_selection_changed()`| `controller_cleanup()`     |

> **Note:** Closing predefined root handles (`HKEY_LOCAL_MACHINE` etc.) is an error. The original implementation risked this; the refactored version correctly identifies and skips them in cleanup.

---

### 4.4 UTF-16 to UTF-8 Conversion

All Windows Registry API functions return UTF-16 (wide-char) strings. GTK requires UTF-8. The conversion is centralised in the Facade using a single helper function, ensuring consistent handling throughout:

```c
/* Centralised in registry_facade.c — never duplicated elsewhere */
static gchar *wchar_to_utf8(const WCHAR *ws) {
    if (!ws) return g_strdup("");
    return g_utf16_to_utf8((const gunichar2 *)ws,
                           -1, NULL, NULL, NULL);
}

/* Usage in value formatting */
case REG_SZ:
    data_str = wchar_to_utf8((const WCHAR *)value_data);
    break;
```

---

## 5. Best Programming Practices

### 5.1 Single Responsibility Principle

Each source file has exactly one job. This is enforced structurally: the View file cannot include `registry_facade.h` (no Win32), and the Facade cannot include `registry_view.h` (no GTK). Violations would cause compile errors.

| File                   | Only Allowed To…                                               |
|------------------------|----------------------------------------------------------------|
| `registry_facade.c`    | Call Win32 RegXxx() functions; allocate/free raw data          |
| `registry_model.c`     | Call Facade functions + GTK store mutation functions           |
| `registry_view.c`      | Call GTK widget creation functions + `view_set_status()`       |
| `registry_controller.c`| Connect GTK signals; call Model + View functions; manage timer |
| `main.c`               | Create AppState; call View, Model, Controller init functions; run event loop |

---

### 5.2 Memory Management Rules

- Every function that allocates memory documents the requirement in its header comment ("caller must free with…")
- GLib allocations use `g_new0` / `g_free`; C allocations use `malloc` / `free` — never mixed
- All struct fields in `RegistryValue` are freed by `registry_free_values()` — never individually
- GTK stores own their string copies; caller strings are freed after `gtk_tree_store_set()` or `gtk_list_store_set()`
- `g_return_if_fail` / `g_return_val_if_fail` guard every public function against NULL pointer crashes

---

### 5.3 Naming Conventions

| Category         | Convention          | Example                                       |
|------------------|---------------------|-----------------------------------------------|
| Global constants | UPPER_SNAKE_CASE    | `AUTO_REFRESH_INTERVAL_SEC`                   |
| Type definitions | PascalCase          | `AppState`, `RegistryValue`                   |
| Enum values      | PREFIX_UPPER_SNAKE  | `TREE_COL_NAME`, `VALUE_N_COLUMNS`            |
| Public functions | `module_verb_noun()`| `registry_enum_values()`, `model_load_values()`|
| Static helpers   | `verb_noun()`       | `wchar_to_utf8()`, `bytes_to_hex()`           |
| GTK callbacks    | `on_event_name()`   | `on_selection_changed()`, `on_row_expanded()` |
| Local variables  | snake_case          | `parent_hkey`, `child_iter`                   |

---

### 5.4 Defensive Programming

```c
/* Rule 1: Always validate function arguments */
LONG registry_open_key(HKEY parent, const WCHAR *subkey_w, HKEY *out) {
    g_return_val_if_fail(out != NULL, ERROR_INVALID_PARAMETER);
    *out = NULL;  // Always initialise OUT params before use
    return RegOpenKeyExW(parent, subkey_w, 0, KEY_READ, out);
}

/* Rule 2: NULL-safe cleanup functions */
void registry_close_key(HKEY hkey) {
    if (hkey) RegCloseKey(hkey);  // Safe to call with NULL
}

/* Rule 3: Reset buffer sizes before every RegEnum call */
while (TRUE) {
    name_len = MAX_PATH;  // ← MUST reset each iteration
    data_len = max_data_len + 2;
    rc = RegEnumValueW(hkey, index, name_buf, &name_len, ...);
}
```

---

### 5.5 Code Documentation Standard

All public functions use Doxygen-style comments with `@brief`, `@param`, and `@return` tags. Internal (static) helpers use a single-line comment explaining their purpose. This allows documentation to be auto-generated with doxygen without any configuration changes.

```c
/**
 * Enumerate all values under @p hkey.
 *
 * @param hkey        An open, readable HKEY.
 * @param[out] count  Number of values returned.
 * @return            Heap-allocated array of RegistryValue structs,
 *                    or NULL on error. Free with registry_free_values().
 */
RegistryValue *registry_enum_values(HKEY hkey, DWORD *count);
```

---

## 6. Design Patterns

### 6.1 Facade Pattern

The Facade pattern provides a simplified interface over a complex subsystem. Here, the Win32 Registry API (with its many error codes, buffer-length conventions, and UTF-16 quirks) is hidden behind six clean, safe functions.

```c
/* WITHOUT Facade (original code, spread throughout the file): */
RegOpenKeyExW(parent, subkey, 0, KEY_READ, &hSubKey);  // repeated
subkey_name_len = MAX_PATH;                             // easy to forget reset
lResult = RegEnumKeyExW(hkey, i, name, &name_len, ...);
if (lResult != ERROR_SUCCESS && lResult != ERROR_NO_MORE_ITEMS) { ... }

/* WITH Facade (clean call sites in model.c): */
WCHAR **subkeys = registry_enum_subkeys(parent_hkey);
for (WCHAR **p = subkeys; *p; p++)
    append_subkey_row(state, parent_iter, parent_hkey, *p);
registry_free_subkey_names(subkeys);  // one ownership rule
```

---

### 6.2 MVC (Model-View-Controller) Pattern

The application uses a strict MVC separation implemented across three physical directories:

| MVC Role   | File                   | What It Knows About                                           |
|------------|------------------------|---------------------------------------------------------------|
| Model      | `registry_model.c`     | GTK stores + Facade data structures; nothing about widgets    |
| View       | `registry_view.c`      | GTK widget API only; nothing about registry data              |
| Controller | `registry_controller.c`| GTK signals; delegates to Model and View; owns AppState timer |

---

### 6.3 Lazy Loading / Virtual Proxy Pattern

The tree view uses a Virtual Proxy pattern for performance: a lightweight placeholder child ("Loading…") stands in for the real subkeys until the node is actually expanded by the user. This avoids loading the entire registry tree (potentially millions of keys) on startup.

| Scenario              | Keys Loaded into Memory                               |
|-----------------------|-------------------------------------------------------|
| App startup           | 5 root nodes only (HKCR, HKCU, HKLM, HKU, HKCC)      |
| User expands HKLM     | ~15 direct children of HKLM                           |
| User expands SOFTWARE | ~200–800 direct children of SOFTWARE                  |
| Without lazy loading  | Potentially 1,000,000+ keys at startup — unacceptable |

---

### 6.4 Observer Pattern (via GTK Signals)

GTK's signal/callback system is a built-in implementation of the Observer pattern. The Controller subscribes to model change events and UI events without the View or Model knowing about each other:

```c
/* Controller subscribes (observe) — View and Model never talk directly */
GtkTreeSelection *sel = gtk_tree_view_get_selection(state->key_tree_view);
g_signal_connect(sel,  "changed",      G_CALLBACK(on_selection_changed), state);
g_signal_connect(tree, "row-expanded", G_CALLBACK(on_row_expanded),      state);

/* When GtkTreeSelection fires 'changed', Controller calls Model: */
static void on_selection_changed(GtkTreeSelection *sel, gpointer user_data) {
    AppState *state = (AppState *)user_data;
    // ... get hkey from selection ...
    model_load_values(state, hkey);  // Model updates GtkListStore
    // GTK automatically redraws the view — Controller never touches the view
}
```

---

### 6.5 Template Method Pattern (Registry Value Formatting)

The `format_value_data()` function in the Facade implements a Template Method: the algorithm skeleton is fixed (switch on type → format → return string), but the concrete formatting step varies by type. Adding a new `REG_*` type only requires adding one new case:

```c
static gchar *format_value_data(DWORD type, const BYTE *data, DWORD len) {
    if (!data || len == 0) return g_strdup("(empty)");  // invariant

    switch (type) {                      // ← vary by type
        case REG_SZ:       return wchar_to_utf8(...);
        case REG_DWORD:    return g_strdup_printf("0x%08X (%lu)", ...);
        case REG_QWORD:    return g_strdup_printf("0x%016llX (%llu)", ...);
        case REG_BINARY:   return bytes_to_hex(data, len);
        case REG_MULTI_SZ: /* join with  |  separator */ ...
        default:           return bytes_to_hex(data, len);  // safe fallback
    }
}
```

---

### 6.6 Pattern Summary

| Pattern         | Where Applied          | Benefit                                               |
|-----------------|------------------------|-------------------------------------------------------|
| Facade          | `registry_facade.c`    | Isolates Win32 complexity; single audit point for safety |
| MVC             | `model/` `ui/` `controller/` | Separation of concerns; each layer independently testable |
| Virtual Proxy   | Lazy tree expansion    | Prevents loading millions of keys at startup          |
| Observer        | GTK `g_signal_connect()` | Decouples event sources from event handlers         |
| Template Method | `format_value_data()`  | Extends formatting to new types without modifying callers |

---

## 7. Build Configuration & Testing

### 7.1 CMake Build

```bash
# Prerequisites: GTK4, CMake ≥ 3.20, C11 compiler
mkdir build && cd build
cmake ..
cmake --build .

# Run
.\RegistryViewer.exe
```

---

### 7.2 Compiler Warnings

The `CMakeLists.txt` enables maximum warning levels. The project must compile with zero warnings:

```bash
# GCC / Clang
-Wall -Wextra -Wpedantic -Werror

# MSVC
/W4 /WX
```

---

### 7.3 Safety Verification Checklist

| Check                        | How to Verify                                                               |
|------------------------------|-----------------------------------------------------------------------------|
| No KEY_WRITE anywhere        | `grep -r "KEY_WRITE" src/ model/ ui/ controller/` → must return nothing     |
| No RegSetValue anywhere      | `grep -r "RegSetValue" .` → must return nothing                             |
| No RegDeleteKey anywhere     | `grep -r "RegDeleteKey" .` → must return nothing                            |
| All allocs have matching frees | Run with Valgrind or Dr. Memory; check for leak reports                   |
| Facade is only Win32 caller  | `grep -r "#include <windows.h>"` → should appear in facade only            |

---

