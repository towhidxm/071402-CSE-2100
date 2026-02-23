/**
 * @file registry_view.h
 * @brief View layer – GTK widget construction and status helpers.
 *
 * Builds all widgets and wires them into the layout.  No Win32 or
 * model logic lives here.
 *
 * Course:     <Course Name>
 * University: <University Name>
 * Author:     <Your Name>
 * Date:       2026-02-17
 */

#ifndef REGISTRY_VIEW_H
#define REGISTRY_VIEW_H

#include "registry_types.h"

/**
 * Build and show the main application window.
 * Stores widget pointers back into @p state.
 *
 * @param app    The GtkApplication instance.
 * @param state  Application state to populate with widget refs.
 */
void view_build_main_window (GtkApplication *app, AppState *state);

/**
 * Push a new message onto the status bar, replacing the last one.
 *
 * @param state    Application state.
 * @param message  UTF-8 message string.
 */
void view_set_status (AppState *state, const gchar *message);

/**
 * Display the read-only safety notice dialog (modal).
 *
 * @param parent  Parent window for the dialog.
 */
void view_show_safety_notice (GtkWindow *parent);

#endif /* REGISTRY_VIEW_H */
