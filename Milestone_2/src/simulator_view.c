#include "simulator_view.h"
#include "memory.h"
#include "PCB.h"
#include "Queue.h"
#include "memory_manager.h"
#include "mutex.h"
#include <stdio.h>
#include <string.h>
#include <pango/pango.h>

#define MEMORY_SIZE 60

extern struct Queue *job_pool;
extern MemoryWord *memory;
extern int ranges_count;

typedef struct
{
    GCallback callback;
    gpointer user_data;
} DialogCallbackData;

static DialogCallbackData dialog_callback_data = {NULL, NULL};

typedef void (*ButtonSignalHandler)(GtkButton *, gpointer);

// Helper function to set label text color to white directly using markup
static void set_label_white_text(GtkWidget *label)
{
    // Get the current text content
    const char *text = gtk_label_get_text(GTK_LABEL(label));

    // Create a markup string with white color and bold text
    char *markup = g_markup_printf_escaped("<span foreground='white' weight='bold'>%s</span>", text);

    // Set markup to force white color regardless of theme
    gtk_label_set_markup(GTK_LABEL(label), markup);

    // Free the markup string
    g_free(markup);

    // Also add a custom CSS class to make extra sure
    gtk_widget_add_css_class(label, "white-text-label");
}

static void show_process_dialog(GtkButton *button, gpointer user_data)
{
    SimulatorView *view = (SimulatorView *)user_data;
    if (!view || !view->main_container || !view->main_window)
    {
        fprintf(stderr, "Error: Cannot show process dialog - view, main_container, or main_window is NULL\n");
        return;
    }

    GtkWidget *dialog = gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(dialog), "Create Process");
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
    gtk_window_set_transient_for(GTK_WINDOW(dialog), view->main_window);
    gtk_window_set_default_size(GTK_WINDOW(dialog), 400, 500);
    view->dialog = GTK_WINDOW(dialog);

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_start(box, 10);
    gtk_widget_set_margin_end(box, 10);
    gtk_widget_set_margin_top(box, 10);
    gtk_widget_set_margin_bottom(box, 10);
    gtk_window_set_child(GTK_WINDOW(dialog), box);

    GtkWidget *file_label = gtk_label_new("Enter program file path:");
    gtk_box_append(GTK_BOX(box), file_label);
    GtkWidget *file_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(file_entry), "../programs/Program_1.txt");
    gtk_box_append(GTK_BOX(box), file_entry);
    view->file_entry = GTK_ENTRY(file_entry);

    GtkWidget *arrival_label = gtk_label_new("Enter program arrival time:");
    gtk_box_append(GTK_BOX(box), arrival_label);
    GtkWidget *arrival_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(arrival_entry), "ex: 0");
    gtk_box_append(GTK_BOX(box), arrival_entry);
    view->arrival_entry = GTK_ENTRY(arrival_entry);

    GtkWidget *done_button = gtk_button_new_with_label("Done");
    gtk_box_append(GTK_BOX(box), done_button);

    if (dialog_callback_data.callback && dialog_callback_data.user_data)
    {
        fprintf(stderr, "Connecting Done button to callback\n");
        g_signal_connect(done_button, "clicked", dialog_callback_data.callback, dialog_callback_data.user_data);
    }
    else
    {
        fprintf(stderr, "Warning: dialog_callback_data.callback or user_data is NULL\n");
    }

    GtkWidget *text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_WORD);
    GtkWidget *scrolled_window = gtk_scrolled_window_new();
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled_window), text_view);
    gtk_widget_set_vexpand(scrolled_window, TRUE);
    gtk_box_append(GTK_BOX(box), scrolled_window);
    view->dialog_text_view = GTK_TEXT_VIEW(text_view);

    g_signal_connect_swapped(dialog, "map", G_CALLBACK(simulator_view_update_memory), view);
    g_signal_connect(dialog, "destroy", G_CALLBACK(gtk_window_destroy), NULL);

    gtk_widget_set_visible(dialog, TRUE);
}

// Helper function to create a status label
static GtkWidget *create_status_label(const char *text)
{
    GtkWidget *label = gtk_label_new(text);
    gtk_widget_add_css_class(label, "simulator-label");
    gtk_label_set_xalign(GTK_LABEL(label), 0);
    gtk_label_set_ellipsize(GTK_LABEL(label), PANGO_ELLIPSIZE_END);
    return label;
}

SimulatorView *simulator_view_new(GtkApplication *app, GtkWidget *parent_container, GtkWindow *main_window)
{
    SimulatorView *view = g_new(SimulatorView, 1);
    view->window = NULL;
    view->job_pool_display = NULL;
    view->memory_list = NULL;
    view->dialog = NULL;
    view->file_entry = NULL;
    view->arrival_entry = NULL;
    view->dialog_text_view = NULL;
    view->main_window = main_window;

    // Create main horizontal box to contain all simulator sections
    GtkWidget *horizontal_container = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_margin_start(horizontal_container, 10);
    gtk_widget_set_margin_end(horizontal_container, 10);
    gtk_widget_set_margin_top(horizontal_container, 10);
    gtk_widget_set_margin_bottom(horizontal_container, 10);
    gtk_box_append(GTK_BOX(parent_container), horizontal_container);

    // Left side container for memory and job pool - reduce width
    view->main_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_size_request(view->main_container, 160, -1);
    gtk_widget_set_hexpand(view->main_container, FALSE);
    gtk_box_append(GTK_BOX(horizontal_container), view->main_container);

    // Middle container for ready queues
    GtkWidget *middle_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_hexpand(middle_container, TRUE);
    gtk_box_append(GTK_BOX(horizontal_container), middle_container);

    // Right side container for resource panel
    GtkWidget *right_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_hexpand(right_container, FALSE);
    gtk_widget_set_size_request(right_container, 200, -1);
    gtk_box_append(GTK_BOX(horizontal_container), right_container);

    // Create the resource panel - moved to after the ready queue setup
    // This will be added later to allow proper alignment

    // Job pool setup
    GtkWidget *job_pool_frame = gtk_frame_new(NULL);
    gtk_box_append(GTK_BOX(view->main_container), job_pool_frame);
    GtkWidget *job_pool_title = gtk_label_new("Job Pool");
    gtk_widget_add_css_class(job_pool_title, "simulator-frame-title");
    gtk_widget_set_name(job_pool_title, "white_text_label");
    set_label_white_text(job_pool_title);

    GtkWidget *job_pool_content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    GtkWidget *job_pool_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_append(GTK_BOX(job_pool_box), job_pool_title);
    gtk_box_append(GTK_BOX(job_pool_box), job_pool_content);
    gtk_frame_set_child(GTK_FRAME(job_pool_frame), job_pool_box);

    GtkWidget *job_pool_display = gtk_list_box_new();
    gtk_list_box_set_selection_mode(GTK_LIST_BOX(job_pool_display), GTK_SELECTION_NONE);
    gtk_box_append(GTK_BOX(job_pool_content), job_pool_display);
    view->job_pool_display = GTK_LIST_BOX(job_pool_display);

    GtkWidget *create_button = gtk_button_new_with_label("+");
    gtk_box_append(GTK_BOX(job_pool_content), create_button);
    g_signal_connect(create_button, "clicked", G_CALLBACK((ButtonSignalHandler)show_process_dialog), view);

    // Memory section setup
    GtkWidget *memory_frame = gtk_frame_new(NULL);
    gtk_widget_set_vexpand(memory_frame, TRUE);
    gtk_widget_set_hexpand(memory_frame, FALSE);
    gtk_widget_set_size_request(memory_frame, 250, -1);
    gtk_box_append(GTK_BOX(view->main_container), memory_frame);
    GtkWidget *memory_title = gtk_label_new("Memory");
    gtk_widget_add_css_class(memory_title, "simulator-frame-title");
    gtk_widget_set_name(memory_title, "white_text_label");
    set_label_white_text(memory_title);

    GtkWidget *memory_scrolled = gtk_scrolled_window_new();
    gtk_widget_set_vexpand(memory_scrolled, TRUE);
    gtk_widget_set_hexpand(memory_scrolled, FALSE);
    gtk_widget_set_size_request(memory_scrolled, 250, -1);

    GtkWidget *memory_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_append(GTK_BOX(memory_box), memory_title);
    gtk_box_append(GTK_BOX(memory_box), memory_scrolled);
    gtk_frame_set_child(GTK_FRAME(memory_frame), memory_box);

    GtkWidget *memory_list = gtk_list_box_new();
    gtk_list_box_set_selection_mode(GTK_LIST_BOX(memory_list), GTK_SELECTION_NONE);
    gtk_widget_set_size_request(memory_list, 250, -1);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(memory_scrolled), memory_list);
    view->memory_list = GTK_LIST_BOX(memory_list);

    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_string(provider, ".simulator-frame { background-color: rgb(236, 236, 234); border: 1px solid #bbb; }"
                                                ".simulator-frame > label { color: white !important; -gtk-text-color: white; font-weight: bold; font-size: 14px; background-color: #33A19A; padding: 5px; border-radius: 3px 3px 0 0; }"
                                                ".simulator-label { color: black; font-size: 14px; font-family: 'Roboto', 'Segoe UI', system-ui, sans-serif; }"
                                                ".simulator-listbox { background-color: #D9D9D9; }"
                                                ".simulator-listbox row { padding: 5px; margin: 2px; }"
                                                ".simulator-listbox row:nth-child(even) { background-color: rgba(51, 161, 154, 0.1); }"
                                                ".simulator-listbox row:hover { background-color: rgba(51, 161, 154, 0.3); }"
                                                ".simulator-button { background-color: #33A19A; color: white; border-radius: 5px; padding: 5px; }"
                                                ".simulator-button:hover { background-color: #278f89; }"
                                                ".simulator-entry { background-color: white; color: white; border: 1px solid #bbb; border-radius: 5px; padding: 5px; }"
                                                ".simulator-textview { background-color: white; color: white; border: 1px solid #bbb; padding: 8px; font-family: 'Roboto', 'Segoe UI', system-ui, sans-serif; font-size: 13px; }"
                                                ".simulator-memory-tag { background-color: #33A19A; color: white !important; border-radius: 3px 0 0 3px; padding: 5px; font-weight: bold; margin-left: -10px; box-shadow: 1px 1px 3px rgba(0,0,0,0.3); width: 30px; text-align: center; }"
                                                ".simulator-memory-content { color: white; padding: 2px; }"
                                                ".simulator-memory-content:hover { color: white; }"
                                                ".simulator-memory-empty { color: white; font-style: italic; padding: 2px; }"
                                                ".simulator-memory-pcb { background-color: #f5f5f5; border-radius: 0 0 5px 5px; padding: 0; border: 1px solid #33A19A; margin-bottom: 7px; margin-right:2px;}"
                                                ".simulator-memory-pcb-row { padding: 2px 5px; border-bottom: 1px solid rgba(51, 161, 154, 0.2); font-size: 12px; color: white; }"
                                                ".simulator-memory-pcb-row:last-child { border-bottom: none; }"
                                                ".simulator-memory-slot { border-bottom: 1px solid #ccc; background-color: #f5f5f5; padding: 2px; max-width: 250px; }"
                                                ".simulator-memory-slot:hover .simulator-memory-content { color: white; }"
                                                ".simulator-frame-title { background-color: #33A19A; color: white !important; -gtk-text-color: white !important; padding: 5px; border-radius: 3px 3px 0 0; }"
                                                ".simulator-pcb-tab { background-color: #33A19A; color: white !important; -gtk-text-color: white !important; padding: 6px 12px; border-radius: 5px 5px 0 0; font-weight: bold; margin-bottom: 0; margin-top:5px; margin-right:2px;}"
                                                ".simulator-process-title { background-color: #196761; color: white !important; -gtk-text-color: white !important; padding: 4px 10px; border-radius: 5px 5px 0 0; font-weight: bold; margin-bottom: 0; font-size: 13px; max-width:180px; }"
                                                ".resource-label { margin: 2px 0; padding: 3px 5px; border-radius: 3px; color: black; }"
                                                ".resource-held { background-color: #ffcccc; border-left: 3px solid #ff3333; color: white; }"
                                                ".resource-available { background-color: #ccffcc; border-left: 3px solid #33cc33; color: white; }"
                                                ".blocked-process { color: white; font-weight: bold; }"
                                                /* Enhanced CSS styles for resource panel */ ".resource-frame-title { background-color: #33A19A; color: white !important; -gtk-text-color: white !important; padding: 5px; border-radius: 3px 3px 0 0; font-weight: bold; font-size: 14px; }"
                                                ".resource-separator { margin: 8px 0; background-color:  #33A19A;; }"
                                                ".resource-queue-container { margin-top: 5px; border-radius: 4px; padding: 2px; background-color: rgba(138, 43, 226, 0.05); }"
                                                ".resource-queue-header { color: white !important; -gtk-text-color: white !important; background-color: #33A19A;; padding: 3px 6px; border-radius: 3px 3px 0 0; font-size: 13px; font-weight: bold; width: 100%; }"
                                                ".resource-queue-content { padding: 5px; background-color: #f9f6fc; border: 1px solid #e6d5f5; border-radius: 0 0 3px 3px; font-size: 18px; height: 100%; color: white; }");

    gtk_style_context_add_provider_for_display(
        gdk_display_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);

    return view;
}

void simulator_view_show(SimulatorView *view)
{
    if (!view || !view->main_container)
    {
        fprintf(stderr, "Error: Cannot show simulator view - view or main_container is NULL\n");
        return;
    }
    gtk_widget_set_visible(view->main_container, TRUE);
}

void simulator_view_connect_create_process(SimulatorView *view, GCallback callback, gpointer user_data)
{
    if (!view || !callback || !user_data)
    {
        fprintf(stderr, "Error: Cannot connect create process - view, callback, or user_data is NULL\n");
        return;
    }
    dialog_callback_data.callback = callback;
    dialog_callback_data.user_data = user_data;
}

void simulator_view_update_job_pool(SimulatorView *view)
{
    if (!view || !view->job_pool_display || !job_pool)
    {
        fprintf(stderr, "Error: Cannot update job pool - view, job_pool_display, or job_pool is NULL\n");
        return;
    }

    while (gtk_list_box_get_row_at_index(view->job_pool_display, 0) != NULL)
    {
        gtk_list_box_remove(view->job_pool_display, GTK_WIDGET(gtk_list_box_get_row_at_index(view->job_pool_display, 0)));
    }

    int size = getQueueSize(job_pool);
    for (int i = 0; i < size; i++)
    {
        char pid_str[16];
        Process *process = dequeue(job_pool);
        if (!process)
        {
            fprintf(stderr, "Error: Failed to dequeue process from job_pool\n");
            continue;
        }
        snprintf(pid_str, sizeof(pid_str), "p%d", process->pid);
        GtkWidget *label = gtk_label_new(pid_str);
        gtk_widget_add_css_class(label, "job-pool-item");
        enqueue(job_pool, process);
        gtk_list_box_append(view->job_pool_display, label);
    }
}

void simulator_view_update_memory(SimulatorView *view)
{
    if (!view || !view->memory_list)
    {
        fprintf(stderr, "Error: Cannot update memory - view or memory_list is NULL\n");
        return;
    }

    while (gtk_list_box_get_row_at_index(view->memory_list, 0) != NULL)
    {
        gtk_list_box_remove(view->memory_list, GTK_WIDGET(gtk_list_box_get_row_at_index(view->memory_list, 0)));
    }

    for (int i = 0; i < MEMORY_SIZE; i++)
    {
        MemoryWord *slot = NULL;
        HASH_FIND_INT(memory, &i, slot);

        char content_text[256] = {0};
        char *content = NULL;
        int pid = -1;
        MemoryRange range = {0, 0, 0, 0, 0, 0};
        gboolean slot_mapped = FALSE;

        if (slot && slot->type == TYPE_PCB && slot->data)
        {
            PCB *pcb = (PCB *)slot->data;
            int pcb_id = getPCBId(pcb);
            pid = pcb_id;
            const char *state_str;
            switch (getPCBState(pcb))
            {
            case NEW:
                state_str = "NEW";
                break;
            case READY:
                state_str = "READY";
                break;
            case RUNNING:
                state_str = "RUNNING";
                break;
            case BLOCKED:
                state_str = "WAITING";
                break;
            case TERMINATED:
                state_str = "TERMINATED";
                break;
            default:
                state_str = "UNKNOWN";
            }

            char process_title[32];
            snprintf(process_title, sizeof(process_title), "Process %d", pid);
            char pcb_label[32];
            snprintf(pcb_label, sizeof(pcb_label), "PCB");
            char state_info[32];
            snprintf(state_info, sizeof(state_info), "State: %s", state_str);
            char priority_info[32];
            snprintf(priority_info, sizeof(priority_info), "Priority: %d", getPCBPriority(pcb));
            char pc_info[32];
            snprintf(pc_info, sizeof(pc_info), "PC: %d", getPCBProgramCounter(pcb));
            char mem_bounds[32];
            snprintf(mem_bounds, sizeof(mem_bounds), "Mem Bounds: %d-%d", getPCBMemLowerBound(pcb), getPCBMemUpperBound(pcb));
            slot_mapped = TRUE;

            GtkWidget *full_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
            gtk_widget_set_margin_start(full_container, 5);
            gtk_widget_set_margin_end(full_container, 5);
            gtk_widget_set_margin_top(full_container, 2);
            gtk_widget_set_margin_bottom(full_container, 2);
            GtkWidget *process_title_label = gtk_label_new(process_title);
            gtk_widget_add_css_class(process_title_label, "simulator-process-title");
            set_label_white_text(process_title_label);
            gtk_label_set_ellipsize(GTK_LABEL(process_title_label), PANGO_ELLIPSIZE_END);
            gtk_box_append(GTK_BOX(full_container), process_title_label);
            GtkWidget *slot_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
            gtk_widget_add_css_class(slot_box, "simulator-memory-slot");
            gtk_box_append(GTK_BOX(full_container), slot_box);

            char tag_text[20];
            snprintf(tag_text, sizeof(tag_text), "%d", i);
            GtkWidget *tag_label = gtk_label_new(tag_text);
            gtk_widget_add_css_class(tag_label, "simulator-memory-tag");
            set_label_white_text(tag_label);
            gtk_widget_set_margin_end(tag_label, 5);
            gtk_box_append(GTK_BOX(slot_box), tag_label);

            GtkWidget *pcb_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
            gtk_widget_set_hexpand(pcb_container, FALSE);

            GtkWidget *pcb_tab = gtk_label_new(pcb_label);
            gtk_widget_add_css_class(pcb_tab, "simulator-pcb-tab");
            gtk_box_append(GTK_BOX(pcb_container), pcb_tab);

            GtkWidget *pcb_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
            gtk_widget_add_css_class(pcb_box, "simulator-memory-pcb");
            gtk_widget_set_hexpand(pcb_box, FALSE);

            GtkWidget *state_row = gtk_label_new(state_info);
            gtk_label_set_xalign(GTK_LABEL(state_row), 0);
            gtk_label_set_ellipsize(GTK_LABEL(state_row), PANGO_ELLIPSIZE_END);
            gtk_widget_add_css_class(state_row, "simulator-memory-pcb-row");
            gtk_box_append(GTK_BOX(pcb_box), state_row);

            GtkWidget *priority_row = gtk_label_new(priority_info);
            gtk_label_set_xalign(GTK_LABEL(priority_row), 0);
            gtk_label_set_ellipsize(GTK_LABEL(priority_row), PANGO_ELLIPSIZE_END);
            gtk_widget_add_css_class(priority_row, "simulator-memory-pcb-row");
            gtk_box_append(GTK_BOX(pcb_box), priority_row);

            GtkWidget *pc_row = gtk_label_new(pc_info);
            gtk_label_set_xalign(GTK_LABEL(pc_row), 0);
            gtk_label_set_ellipsize(GTK_LABEL(pc_row), PANGO_ELLIPSIZE_END);
            gtk_widget_add_css_class(pc_row, "simulator-memory-pcb-row");
            gtk_box_append(GTK_BOX(pcb_box), pc_row);

            GtkWidget *mem_row = gtk_label_new(mem_bounds);
            gtk_label_set_xalign(GTK_LABEL(mem_row), 0);
            gtk_label_set_ellipsize(GTK_LABEL(mem_row), PANGO_ELLIPSIZE_END);
            gtk_widget_add_css_class(mem_row, "simulator-memory-pcb-row");
            gtk_box_append(GTK_BOX(pcb_box), mem_row);

            gtk_box_append(GTK_BOX(pcb_container), pcb_box);
            gtk_box_append(GTK_BOX(slot_box), pcb_container);
            gtk_list_box_append(view->memory_list, full_container);
            gtk_widget_set_visible(full_container, TRUE);
            continue;
        }
        else
        {
            for (int p = 1; p <= ranges_count; p++)
            {
                range = getProcessMemoryRange(p);
                if (range.inst_count == 0 && range.var_count == 0 && range.pcb_count == 0)
                {
                    continue;
                }
                if (range.inst_start < 0 || range.var_start < 0 || range.pcb_start < 0)
                {
                    fprintf(stderr, "Warning: Invalid memory range for PID %d\n", p);
                    continue;
                }
                if (i >= range.inst_start && i < range.inst_start + range.inst_count)
                {
                    pid = p;
                    if (slot && slot->type == TYPE_STRING && slot->data)
                    {
                        content = (char *)slot->data;
                        snprintf(content_text, sizeof(content_text), "%s", content ? content : "Empty");
                    }
                    else
                    {
                        snprintf(content_text, sizeof(content_text), "Empty");
                    }
                    slot_mapped = TRUE;
                    break;
                }
                else if (i >= range.var_start && i < range.var_start + range.var_count)
                {
                    pid = p;
                    if (slot && slot->type == TYPE_STRING && slot->data)
                    {
                        content = (char *)slot->data;
                        snprintf(content_text, sizeof(content_text), "%s", content ? content : "Empty");
                    }
                    else
                    {
                        snprintf(content_text, sizeof(content_text), "Empty");
                    }
                    slot_mapped = TRUE;
                    break;
                }
            }
        }

        if (!slot_mapped)
        {
            snprintf(content_text, sizeof(content_text), "Empty");
        }

        GtkWidget *slot_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
        gtk_widget_add_css_class(slot_box, "simulator-memory-slot");
        gtk_widget_set_margin_start(slot_box, 5);
        gtk_widget_set_margin_end(slot_box, 5);
        gtk_widget_set_margin_top(slot_box, 2);
        gtk_widget_set_margin_bottom(slot_box, 2);
        char tag_text[20];
        snprintf(tag_text, sizeof(tag_text), "%d", i);
        GtkWidget *tag_label = gtk_label_new(tag_text);
        gtk_widget_add_css_class(tag_label, "simulator-memory-tag");
        set_label_white_text(tag_label);
        gtk_widget_set_margin_end(tag_label, 5);
        gtk_box_append(GTK_BOX(slot_box), tag_label);

        GtkWidget *content_label = gtk_label_new(content_text);
        gtk_label_set_xalign(GTK_LABEL(content_label), 0);
        gtk_label_set_ellipsize(GTK_LABEL(content_label), PANGO_ELLIPSIZE_END);

        if (strcmp(content_text, "Empty") == 0)
        {
            gtk_widget_add_css_class(content_label, "simulator-memory-empty");
        }
        else
        {
            gtk_widget_add_css_class(content_label, "simulator-memory-content");
        }

        gtk_widget_set_hexpand(content_label, FALSE);
        if (content && strlen(content) > 50)
        {
            gtk_widget_set_tooltip_text(content_label, content);
            char truncated[51];
            strncpy(truncated, content, 47);
            truncated[47] = '.';
            truncated[48] = '.';
            truncated[49] = '.';
            truncated[50] = '\0';
            gtk_label_set_text(GTK_LABEL(content_label), truncated);
        }

        gtk_box_append(GTK_BOX(slot_box), content_label);
        gtk_list_box_append(view->memory_list, slot_box);
        gtk_widget_set_visible(slot_box, TRUE);
    }
    
    simulator_view_update_job_pool(view);
}

void simulator_view_get_process_input(SimulatorView *view, char **file_path, int *arrival_time)
{
    if (!view || !view->file_entry || !view->arrival_entry)
    {
        fprintf(stderr, "Error: Cannot get process input - view or entries are NULL\n");
        *file_path = NULL;
        *arrival_time = 0;
        return;
    }

    *file_path = g_strdup(gtk_editable_get_text(GTK_EDITABLE(view->file_entry)));
    const char *arrival_time_str = gtk_editable_get_text(GTK_EDITABLE(view->arrival_entry));
    *arrival_time = (arrival_time_str && strlen(arrival_time_str) > 0) ? atoi(arrival_time_str) : 0;
}

void simulator_view_append_dialog_text(SimulatorView *view, const char *text)
{
    if (!view || !view->dialog_text_view)
    {
        fprintf(stderr, "Error: Cannot append dialog text - view or dialog_text_view is NULL\n");
        return;
    }

    GtkTextBuffer *buffer = gtk_text_view_get_buffer(view->dialog_text_view);
    gtk_text_buffer_set_text(buffer, "", -1);
    gtk_text_buffer_insert_at_cursor(buffer, text, -1);
}

void simulator_view_free(SimulatorView *view) {
    if (view) {
        g_free(view);
    }
}