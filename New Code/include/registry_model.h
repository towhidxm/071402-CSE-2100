/**
 * @file registry_model.h
 * @brief Model layer – GTK store population from registry data.
 *
 * Bridges the Facade (raw Win32 data) and the View (GTK widgets).
 * All functions are pure data manipulations; no UI side-effects.
 *
 * Course:     <Course Name>
 * University: <University Name>
 * Author:     <Your Name>
 * Date:       2026-02-17
 */

#ifndef REGISTRY_MODEL_H
#define REGISTRY_MODEL_H

#include "registry_types.h"

/**
 * Populate the GTK TreeStore with the five standard root keys.
 * Each root gets a "Loading…" placeholder child so the expander
 * arrow is visible before the node is first expanded.
 *
 * @param state  Application state carrying the tree store.
 */
void model_populate_root_keys (AppState *state);

/**
 * Expand a tree node: remove its "Loading…" placeholder and
 * insert real subkey rows (each with their own placeholder).
 *
 * @param state        Application state.
 * @param parent_iter  Iterator of the node being expanded.
 */
void model_expand_node (AppState *state, GtkTreeIter *parent_iter);

/**
 * Populate the ListStore with all values for @p hkey.
 * Clears any previously displayed values first.
 *
 * @param state  Application state carrying the list store.
 * @param hkey   Open, readable registry key.
 */
void model_load_values (AppState *state, HKEY hkey);

#endif /* REGISTRY_MODEL_H */
