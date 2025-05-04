#include "console_controller.h"
#include "console_view.h"
#include "console_model.h"

static GAsyncQueue *input_queue = NULL;
static char *pending_input = NULL;
static gboolean input_request_active = FALSE;

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

// Callback to check for input periodically
static gboolean check_input_queue(gpointer user_data)
{
    InputCallbackData *cb_data = (InputCallbackData *)user_data;

    // Guard against null input_queue
    if (!input_queue)
    {
        g_warning("Input queue is unavailable");
        if (cb_data && cb_data->callback)
        {
            cb_data->callback(cb_data->user_data, g_strdup(""));
        }
        g_free(cb_data);
        return G_SOURCE_REMOVE;
    }

    char *input = (char *)g_async_queue_try_pop(input_queue);
    if (input)
    {
        if (cb_data && cb_data->callback)
        {
            // Use g_idle_add to ensure callback runs on the main thread
            MainThreadCallbackData *main_data = g_new0(MainThreadCallbackData, 1);

            main_data->callback = cb_data->callback;
            main_data->user_data = cb_data->user_data;
            main_data->input = input;

            g_idle_add(process_input_on_main_thread, main_data);
        }
        else
        {
            pending_input = input;
            console_controller_process_input(input);
            g_free(input);
        }

        // Use idle to safely clear input on main thread
        g_idle_add(clear_input_on_main_thread, NULL);

        input_request_active = FALSE;
        g_free(cb_data);
        return G_SOURCE_REMOVE;
    }

    return G_SOURCE_CONTINUE;
}

void console_controller_on_entry_activate(GtkWidget *widget, gpointer user_data)
{
    if (!input_request_active || !input_queue)
    {
        console_clear_input();
        return;
    }

    char *text = console_get_input_text();

    if (text && strlen(text) > 0 && input_queue)
    {
        g_async_queue_push(input_queue, text);
    }
    else
    {
        g_free(text);
    }
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

    // Show prompt in program output using idle to ensure thread safety
    g_idle_add(show_prompt_on_main_thread, g_strdup(prompt));

    input_request_active = TRUE;

    // Set focus on the entry widget
    console_set_input_focus();

    char *input = (char *)g_async_queue_try_pop(input_queue);
    if (input)
    {
        if (callback)
        {
            callback(user_data, input);
        }
        else
        {
            pending_input = input;
            console_controller_process_input(input);
            g_free(input);
        }
        console_clear_input();
        input_request_active = FALSE;
    }
    else
    {
        InputCallbackData *cb_data = g_new0(InputCallbackData, 1);
        cb_data->callback = callback;
        cb_data->user_data = user_data;
        g_timeout_add(100, check_input_queue, cb_data);
    }
}

void console_controller_request_input(const char *prompt)
{
    console_controller_request_input_with_callback(prompt, NULL, NULL);
}

void console_controller_process_input(const char *input)
{
    console_model_process_input(input); // Model
}