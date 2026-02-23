/**
 * @file registry_controller.h
 * @brief Controller layer – GTK signal handlers and timer management.
 *
 * Connects user actions (selection, expansion, button clicks) to
 * Model updates.  References AppState for all shared data.
 *
 * Course:     <Course Name>
 * University: <University Name>
 * Author:     <Your Name>
 * Date:       2026-02-17
 */

#ifndef REGISTRY_CONTROLLER_H
#define REGISTRY_CONTROLLER_H

#include "registry_types.h"

/**
 * Connect all signal handlers to the widgets stored in @p state.
 * Must be called after view_build_main_window().
 *
 * @param state  Application state with fully initialised widget refs.
 */
void controller_connect_signals (AppState *state);

/**
 * Start the auto-refresh timer (no-op if already running).
 *
 * @param state  Application state.
 */
void controller_start_auto_refresh (AppState *state);

/**
 * Stop the auto-refresh timer (no-op if not running).
 *
 * @param state  Application state.
 */
void controller_stop_auto_refresh (AppState *state);

/**
 * Release all resources owned by the controller / application state.
 * Safe to call more than once.
 *
 * @param state  Application state.
 */
void controller_cleanup (AppState *state);

#endif /* REGISTRY_CONTROLLER_H */
