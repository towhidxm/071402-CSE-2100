/**
 * @file main.c
 * @brief Application entry point.
 *
 * Wires Model, View, and Controller together and hands control to
 * the GtkApplication event loop.
 *
 * Course:     <Course Name>
 * University: <University Name>
 * Author:     <Your Name>
 * Date:       2026-02-17
 */

#include "registry_types.h"
#include "registry_view.h"
#include "registry_model.h"
#include "registry_controller.h"

/* ─────────────────────────────────────────────────────────────────
 * GtkApplication "activate" callback
 * ───────────────────────────────────────────────────────────────── */

static void on_activate (GtkApplication *app, gpointer user_data)
{
    AppState *state = (AppState *)user_data;

    /* 1. Build all widgets and store references in state. */
    view_build_main_window (app, state);

    /* 2. Seed the tree with the five root HKEY rows. */
    model_populate_root_keys (state);

    /* 3. Connect GTK signals to controller handlers. */
    controller_connect_signals (state);

    /* 4. Set initial status. */
    view_set_status (state, "Ready  –  Select a key to begin.");

    /* Optional: show safety dialog on first launch.
     * Uncomment the line below to re-enable it.
     *   view_show_safety_notice(GTK_WINDOW(state->main_window));
     */
}

/* ─────────────────────────────────────────────────────────────────
 * main()
 * ───────────────────────────────────────────────────────────────── */

int main (int argc, char **argv)
{
    /* Zero-initialise application state. */
    AppState state = { 0 };

    GtkApplication *app = gtk_application_new (APP_ID,
                                               G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect (app, "activate", G_CALLBACK (on_activate), &state);

    int exit_code = g_application_run (G_APPLICATION (app), argc, argv);

    /* Cleanup before exit. */
    controller_cleanup (&state);
    g_object_unref (app);

    return exit_code;
}
