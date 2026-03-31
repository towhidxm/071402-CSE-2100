## Windows Registry Viewer — GTK4 Implementation

### Course Metadata
- **Course Code**: 0714 02 CSE 2100  
- **Course Title**: Advanced Programming Laboratory  
- **Project**: Windows Registry Viewer — Read‑Only Refactoring Journey  
- **Students**: Sakib08 & AbuSaeed22  
- **Session**: 0714‑02  
- **Date**: February 2026  

---

## Part A — Project Overview

### 1. Introduction

This project is a **Windows Registry Viewer** application built from scratch using **GTK4** and **C**.  
It provides a modern, safe, **read‑only** graphical interface to browse the Windows Registry in a structured way.

**What This Application Does**

- **Browse Registry Hives**: `HKEY_CLASSES_ROOT`, `HKEY_CURRENT_USER`, `HKEY_LOCAL_MACHINE`, `HKEY_USERS`, `HKEY_CURRENT_CONFIG`
- **Lazy Loading of Keys**: Expands subkeys on demand for performance
- **Value Inspection**: Shows value name, type, and formatted data
- **Type‑Aware Formatting**:
  - `REG_SZ`, `REG_EXPAND_SZ`, `REG_MULTI_SZ` → UTF‑8 strings
  - `REG_DWORD`, `REG_QWORD` → hex + decimal
  - `REG_BINARY` and others → hex dump
- **Status Bar Feedback**: Shows current path and timestamps
- **Auto‑Refresh Support (Controller API)**: Hooks provided to periodically reload values
- **Safety First**: **No write / delete operations** implemented (read‑only façade)

The design mirrors the **Windows Event Viewer refactoring** you described, but targets the **Windows Registry** instead of Event Logs and is tailored to the course’s **architecture and MVC focus**.

### 2. Design Goals

- **Read‑Only Safety**: All Registry access is done through a façade that **never** calls write APIs.
- **Layered Architecture**: Clear separation between **View (GTK widgets)**, **Model (GTK stores)**, and **Controller (signals + timers)**.
- **Win32 Isolation**: All `Reg*` calls live in a single façade module.
- **Course‑Friendly Style**: Professional naming, comments, and modularisation suitable for an Advanced Programming Laboratory report.

### 3. Technology Stack

- **Language**: C (C11)
- **GUI Framework**: GTK4
- **Windows API**: Win32 Registry API (`windows.h`, `RegOpenKeyExW`, `RegEnumKeyExW`, `RegEnumValueW`, `RegQueryInfoKeyW`, `RegCloseKey`, etc.)
- **Build System**: GNU Make
- **Compiler**: GCC (MinGW‑w64, 64‑bit)
- **Platform**: Windows 10 / 11 (64‑bit) via **MSYS2 MinGW‑w64**

---

## Part B — Repository Structure

### 4. Folder Layout

```text
.
├── src/
│   ├── main.c               # Application entry point, wires MVC together
│   └── registry_facade.c    # Read‑only Win32 Registry façade
├── include/
│   ├── registry_types.h     # Shared types, enums, AppState
│   ├── registry_view.h      # View interface (GTK widgets)
│   ├── registry_model.h     # Model interface (GTK stores)
│   ├── registry_controller.h# Controller interface (signals, timers)
│   └── registry_facade.h    # Facade interface for registry operations
├── ui/
│   └── registry_view.c      # Implementation of GTK UI (tree, list, toolbar)
├── model/
│   └── registry_model.c     # GTK TreeStore/ListStore population logic
├── controller/
│   └── registry_controller.c# Signal handlers + auto‑refresh
├── build/
│   ├── obj/                 # Object files (generated)
│   └── bin/                 # Final executable (generated)
├── Makefile                 # Build configuration (MSYS2 + GTK4)
├── README.md                # This documentation
└── LICENSE                  # MIT License
```

**Architecture Mapping**

- **Presentation Layer**: `ui/registry_view.c`
- **Application Logic Layer**: `controller/registry_controller.c`
- **Data / Model Layer**: `model/registry_model.c`, `include/registry_types.h`
- **Platform / Utility Layer**: `src/registry_facade.c`, `include/registry_facade.h`

---

## Part C — Build & Usage

### 5. Build Requirements

**System Requirements**

- **OS**: Windows 10 / 11 (64‑bit)
- **RAM**: 2 GB minimum (4 GB recommended)
- **Disk**: ~500 MB for MSYS2 + dependencies

**Required Software**

- **MSYS2** (64‑bit) – `https://www.msys2.org/`
- **GCC / MinGW‑w64**
- **GTK4** development packages
- **pkg-config**

### 6. MSYS2 Setup

1. Install MSYS2 from `https://www.msys2.org/`.
2. Open **MSYS2 MinGW 64‑bit** terminal.
3. Update the environment:

```bash
pacman -Syu
# if terminal closes, reopen and run:
pacman -Su
```

4. Install toolchain and libraries:

```bash
pacman -S mingw-w64-x86_64-gcc
pacman -S mingw-w64-x86_64-make
pacman -S make
pacman -S mingw-w64-x86_64-gtk4
pacman -S mingw-w64-x86_64-pkg-config
```

5. Verify installation:

```bash
gcc --version
pkg-config --modversion gtk4
pkg-config --cflags gtk4
```

---

### 7. Compilation Instructions

1. Open **MSYS2 MinGW 64‑bit**.
2. Change to the project directory:

```bash
cd /c/Users/towhi/OneDrive/Desktop/"New Code"
```

3. Build:

```bash
make
```

**What the Makefile does**

- Compiles all `.c` sources under `src/`, `ui/`, `model/`, `controller/`
- Places object files in `build/obj`
- Links them into `build/bin/registry_viewer.exe`

**Other useful targets**

```bash
make clean      # Remove build/ directory
make rebuild    # Clean and rebuild
make run        # Build and run the application
make help       # Show list of targets
```

---

### 8. Running the Application

From **MSYS2 MinGW 64‑bit**:

```bash
make run
# or
./build/bin/registry_viewer.exe
```

From **Windows Explorer**:

- Navigate to `build/bin/`
- Double‑click `registry_viewer.exe`

**Notes**

- The application is read‑only; it does **not** modify the Registry.
- No administrator rights are strictly required for basic viewing of user‑level keys, but some hives/keys may still be protected by the OS.

---

## Part D — Architecture & Design

### 9. MVC / Layered Design

- **Model (`model/registry_model.c`)**
  - Owns the **GTK TreeStore** for keys and **ListStore** for values.
  - Knows how to populate:
    - Root nodes for the five standard hives
    - Subkeys under a given key (lazy loading with a `"Loading…"` placeholder)
    - Value rows (name, type, data) for a selected key
  - Calls only the **Registry Facade**, never raw `Reg*` APIs.

- **View (`ui/registry_view.c`)**
  - Builds the GTK4 widget tree:
    - Left pane: key tree (`GtkTreeView` + `GtkTreeStore`)
    - Right pane: value list (`GtkTreeView` + `GtkListStore`)
    - Toolbar with **READ‑ONLY** badge
    - Status bar
  - Exposes helper `view_set_status()` and `view_show_safety_notice()`.
  - No Registry logic or Win32 calls.

- **Controller (`controller/registry_controller.c`)**
  - Connects GTK signals:
    - Tree selection changed → load values for selected key
    - Row expanded → lazy‑load subkeys
  - Provides hooks for auto‑refresh using `g_timeout_add_seconds`.
  - Manages `AppState.current_hkey` and updates the status bar.

- **Facade (`src/registry_facade.c`)**
  - Centralises all **Win32 Registry** API usage:
    - `registry_open_key`
    - `registry_enum_subkeys`
    - `registry_enum_values`
    - `registry_close_key`
  - Converts wide strings to UTF‑8 and formats raw bytes into readable text.
  - Guarantees read‑only access; no `RegSetValue*`, `RegDelete*`, or similar calls.

- **Entry Point (`src/main.c`)**
  - Creates `GtkApplication`
  - Wires **View → Model → Controller**
  - Starts GTK main loop and performs final cleanup.

This matches the **“facade + MVC + layered architecture”** style in your Windows Event Viewer README, but scoped to a smaller, focused educational project.

---

## Part E — Troubleshooting

### 10. Common Build Issues

- **`gtk/gtk.h: No such file or directory`**
  - GTK4 not installed or wrong shell.
  - Use **MSYS2 MinGW 64‑bit**, run `pacman -S mingw-w64-x86_64-gtk4` and check `pkg-config --cflags gtk4`.

- **`pkg-config: command not found`**
  - Install via `pacman -S mingw-w64-x86_64-pkg-config`.

- **Linker error `undefined reference` to GTK functions**
  - Ensure `pkg-config --libs gtk4` prints a list of libraries.
  - Make sure you are using `make` from the **MinGW 64‑bit** environment.

### 11. Runtime Issues

- **App opens but tree is empty**
  - Check that `model_populate_root_keys()` is being called (see `main.c`).
  - Verify that `windows.h` and Registry APIs are available (Windows 10/11).

- **Some keys show no values**
  - Many Registry keys legitimately have no values; this is expected.
  - Protected keys may still be hidden by the OS ACLs.

---

## Part F — How This Demonstrates Course Outcomes

- **Advanced C Programming**
  - Modular header/source organisation
  - Safe memory management with GLib/GTK
  - Use of Win32 handles and error codes

- **GTK4 GUI Development**
  - Custom layout with split panes, list/tree views, and status bar
  - Signal handling and event‑driven programming

- **Windows API Integration**
  - Practical use of Registry APIs with Unicode handling
  - Conversion between `WCHAR*` and UTF‑8 using GLib

- **Software Architecture**
  - Clear separation of **View**, **Model**, **Controller**, and **Platform** layers
  - Central façade to minimise coupling with Win32 APIs

---

## Part G — AI Prompts Used During Development

The following prompts were used (or can be reused) to guide analysis, modularisation, and documentation for this Registry Viewer project.

### Prompt 1 — Initial Architecture Audit

```text
Analyze this C + GTK4 Windows Registry Viewer codebase.
Read all source and header files and list:
(1) module responsibilities,
(2) public APIs,
(3) shared state usage,
(4) GTK signal flow,
(5) Win32 Registry API usage.
Then provide a layered architecture summary and identify any coupling risks.
```

### Prompt 2 — MVC Boundary Validation

```text
Check whether this project properly follows MVC.
Verify that:
- View contains only GTK widget construction and display helpers,
- Model contains data loading and store population logic,
- Controller handles signals and orchestration only,
- Win32 Reg* calls are isolated behind a facade.
Report violations with concrete file-level suggestions.
```

### Prompt 3 — Read-Only Safety Review

```text
Audit the code for read-only safety.
Confirm that no write/delete Registry APIs are used
(e.g., RegSetValueEx, RegDeleteKey, RegDeleteValue).
If any unsafe calls exist, suggest safe read-only alternatives.
```

### Prompt 4 — Makefile Generation

```text
Create a production-ready Makefile for this C GTK4 project on MSYS2 MinGW-w64.
Requirements:
- Build all .c files from src/, ui/, model/, controller/
- Output object files to build/obj/
- Output executable to build/bin/registry_viewer.exe
- Use pkg-config for gtk4 cflags/libs
- Link required Windows libraries
- Provide targets: make, run, clean, rebuild, help
```

### Prompt 5 — README Authoring

```text
Write a complete README for a Windows Registry Viewer project
in the style of an Advanced Programming Laboratory report.
Include:
project overview, feature list, architecture section, build/setup steps,
run instructions, troubleshooting, and course outcome mapping.
Keep it professional and submission-ready.
```

### Prompt 6 — Code Quality Refactor

```text
Refactor this codebase for better maintainability without changing behavior.
Focus on:
- smaller functions,
- clearer naming,
- safer memory handling,
- consistent comments and formatting.
Do not introduce registry write capabilities.
```

---

## Part H — License & Contact

- **License**: MIT (see `LICENSE` file)
- **Authors**: Sakib08 & AbuSaeed22  
- **Course**: CSE 2100 – Advanced Programming Laboratory  
- **Session**: 0714‑02, Spring 2026  

This Registry Viewer is designed as a companion project to your **Windows Event Viewer** refactoring journey, following the same architectural discipline but applied to a different Windows subsystem.

