#include "console_controller.h"
#include "console_view.h"
#include "console_model.h"

static GAsyncQueue *input_queue = NULL;
static char *pending_input = NULL;
static gboolean input_request_active = FALSE;
static InputCallback current_callback = NULL;
static void *current_user_data = NULL;
static gboolean waiting_for_input = FALSE;

// Forward declarations for callback functions
static gboolean process_input_on_main_thread(gpointer data);
static gboolean clear_input_on_main_thread(gpointer data);
static gboolean show_prompt_on_main_thread(gpointer data);

typedef struct
{
    InputCallback callback;
    void *user_data;
    char *input;
} MainThreadCallbackData;

void console_controller_init(void)
{
    if (!input_queue)
    {
        input_queue = g_async_queue_new();
    }
    waiting_for_input = FALSE;
}

void console_controller_cleanup(void)
{
    if (input_queue)
    {
        char *input;
        while ((input = g_async_queue_try_pop(input_queue)) != NULL)
        {
            g_free(input);
        }
        g_async_queue_unref(input_queue);
        input_queue = NULL;
    }
    if (pending_input)
    {
        g_free(pending_input);
        pending_input = NULL;
    }
    input_request_active = FALSE;
    waiting_for_input = FALSE;
    current_callback = NULL;
    current_user_data = NULL;
}

void console_controller_reset_view(void)
{
    console_view_reset();
}

static gboolean process_input_on_main_thread(gpointer data)
{
    MainThreadCallbackData *cb_data = (MainThreadCallbackData *)data;
    if (cb_data)
    {
        if (cb_data->callback)
        {
            cb_data->callback(cb_data->user_data, cb_data->input);
        }
        g_free(cb_data);
    }
    return G_SOURCE_REMOVE;
}

static gboolean clear_input_on_main_thread(gpointer data)
{
    console_clear_input();
    return G_SOURCE_REMOVE;
}

static gboolean show_prompt_on_main_thread(gpointer data)
{
    const char *prompt = (const char *)data;
    if (prompt)
    {
        console_update_program_output(prompt);
        g_free((char *)prompt);
    }
    return G_SOURCE_REMOVE;
}

void console_controller_on_entry_activate(GtkWidget *widget, gpointer user_data)
{
    if (!input_request_active || !input_queue)
    {
        return;
    }

    char *text = console_get_input_text();
    if (!text || strlen(text) == 0)
    {
        g_free(text);
        // Keep input field enabled and focused if input is empty
        console_set_input_focus();
        return;
    }

    // Push the input to the queue
    g_async_queue_push(input_queue, text);
    
    // Process the input immediately if we have a callback
    if (current_callback)
    {
        MainThreadCallbackData *main_data = g_new0(MainThreadCallbackData, 1);
        main_data->callback = current_callback;
        main_data->user_data = current_user_data;
        main_data->input = text;
        g_idle_add(process_input_on_main_thread, main_data);
    }
    else
    {
        pending_input = text;
        console_controller_process_input(text);
    }

    // Reset state
    input_request_active = FALSE;
    waiting_for_input = FALSE;
    current_callback = NULL;
    current_user_data = NULL;
    
    // Clear input field
    g_idle_add(clear_input_on_main_thread, NULL);
}

void console_controller_request_input_with_callback(const char *prompt, void (*callback)(void *, char *), void *user_data)
{
    if (!input_queue)
    {
        g_warning("Cannot request input - input queue is not initialized");
        if (callback)
        {
            callback(user_data, g_strdup(""));
        }
        return;
    }

    // If input is already being requested, don't start a new request
    if (input_request_active)
    {
        g_warning("Input request already in progress - ignoring new request");
        return;
    }

    // Show prompt in program output using idle to ensure thread safety
    g_idle_add(show_prompt_on_main_thread, g_strdup(prompt));

    // Set up the input request
    input_request_active = TRUE;
    waiting_for_input = TRUE;
    current_callback = callback;
    current_user_data = user_data;

    // Set focus on the entry widget
    console_set_input_focus();
}

void console_controller_request_input(const char *prompt)
{
    console_controller_request_input_with_callback(prompt, NULL, NULL);
}

void console_controller_process_input(const char *input)
{
    if (!waiting_for_input)
    {
        return;
    }

    if (input && strlen(input) > 0)
    {
        console_model_process_input(input);
        waiting_for_input = FALSE;
    }
}

gboolean console_controller_is_waiting_for_input(void)
{
    return waiting_for_input;
}