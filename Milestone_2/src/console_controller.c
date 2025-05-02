#include "console_controller.h"
#include "console_view.h"
#include "console_model.h"

static GAsyncQueue *input_queue = NULL;
static char *pending_input = NULL;
static gboolean input_request_active = FALSE;

void console_controller_init(void)
{
    if (!input_queue)
    {
        input_queue = g_async_queue_new();
        g_print("Initialized input_queue: %p\n", input_queue);
    }
}

void console_controller_cleanup(void)
{
    g_print("Cleaning up controller, entry_widget: %p, input_queue: %p\n", entry_widget, input_queue);
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

// Callback to check for input periodically
static gboolean check_input_queue(gpointer user_data)
{
    InputCallbackData *cb_data = (InputCallbackData *)user_data;
    g_print("Checking input queue: %p\n", input_queue);
    if (!input_queue)
    {
        g_printerr("Error: input_queue is NULL\n");
        if (cb_data->callback)
        {
            cb_data->callback(cb_data->user_data, g_strdup(""));
        }
        g_free(cb_data);
        return G_SOURCE_REMOVE;
    }
    char *input = (char *)g_async_queue_try_pop(input_queue);
    if (input)
    {
        g_print("Popped input from queue: %s\n", input);
        if (cb_data->callback)
        {
            cb_data->callback(cb_data->user_data, input); // Pass input to callback
        }
        else
        {
            pending_input = input;
            console_controller_process_input(input);
            g_free(input);
        }
        console_clear_input();
        input_request_active = FALSE;
        g_free(cb_data);
        return G_SOURCE_REMOVE;
    }
    return G_SOURCE_CONTINUE;
}

void console_controller_on_entry_activate(GtkWidget *widget, gpointer user_data)
{
    g_print("Entry activated, input_request_active: %d, input_queue: %p\n", input_request_active, input_queue);
    if (!input_request_active)
    {
        console_clear_input();
        return;
    }
    char *text = console_get_input_text();
    g_print("Got input text: %s\n", text ? text : "(null)");
    if (text && strlen(text) > 0 && input_queue)
    {
        g_print("Pushing input to queue: %s\n", text);
        g_async_queue_push(input_queue, text);
    }
    else
    {
        g_print("Freeing empty or null input\n");
        g_free(text);
    }
}

void console_controller_request_input_with_callback(const char *prompt, void (*callback)(void *, char *), void *user_data)
{
    g_print("Requesting input with prompt: %s, input_queue: %p\n", prompt, input_queue);
    if (!input_queue)
    {
        g_printerr("Error: input_queue is NULL\n");
        if (callback)
        {
            callback(user_data, g_strdup(""));
        }
        return;
    }
    console_update_program_output(prompt);
    input_request_active = TRUE;
    console_set_input_focus();
    char *input = (char *)g_async_queue_try_pop(input_queue);
    if (input)
    {
        g_print("Immediate input found: %s\n", input);
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
        g_print("No immediate input, starting queue polling\n");
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