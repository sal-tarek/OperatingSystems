#include "console_controller.h"
#include "console_view.h"
#include "console_model.h"

static GAsyncQueue *input_queue = NULL;
static char *pending_input = NULL;
static gboolean input_request_active = FALSE;

void console_controller_init(void) {
    if (!input_queue) {
        input_queue = g_async_queue_new_full(g_free);
    }
    input_request_active = FALSE;
    pending_input = NULL;
}

void console_controller_cleanup(void) {
    if (input_queue) {
        g_async_queue_unref(input_queue);
        input_queue = NULL;
    }
    if (pending_input) {
        g_free(pending_input);
        pending_input = NULL;
    }
    input_request_active = FALSE;
}

// Callback to check for input periodically
static gboolean check_input_queue(gpointer user_data) {
    char *input = (char*)g_async_queue_try_pop(input_queue);
    if (input) {
        pending_input = input;
        console_controller_process_input(input);
        console_clear_input(); // Clear and disable entry
        input_request_active = FALSE;
        return G_SOURCE_REMOVE; // Stop the timeout
    }
    return G_SOURCE_CONTINUE; // Keep checking
}

void console_controller_on_entry_activate(GtkWidget *widget, gpointer user_data) {
    if (!input_request_active) {
        console_clear_input(); // Clear any unprompted input
        return; // Ignore unprompted input
    }

    char *text = console_get_input_text(); // View (disables entry)
    if (text && strlen(text) > 0) {
        g_async_queue_push(input_queue, text);
    } else {
        g_free(text);
    }
}

char* console_controller_request_input(const char *prompt) {
    if (input_request_active) {
        return g_strdup(""); // Avoid multiple simultaneous requests
    }

    input_request_active = TRUE;
    if (prompt) {
        console_update_program_output(prompt); // View
    }
    console_set_input_focus(); // View (enables entry)

    // Check queue immediately
    char *input = (char*)g_async_queue_try_pop(input_queue);
    if (input) {
        pending_input = input;
        console_controller_process_input(input);
        console_clear_input(); // Clear and disable entry
        input_request_active = FALSE;
        return input;
    }

    // Schedule periodic checks
    g_timeout_add(100, check_input_queue, NULL); // Check every 100ms
    return NULL; // Input will be processed asynchronously
}

void console_controller_process_input(const char *input) {
    console_model_process_input(input); // Model
}