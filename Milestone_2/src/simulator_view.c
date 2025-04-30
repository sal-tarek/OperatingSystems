#include "simulator_view.h"
#include "memory.h"
#include "PCB.h"
#include "Queue.h"
#include "memory_manager.h"
#include <stdio.h>
#include <string.h>

// Define memory size (number of slots to display)
#define MEMORY_SIZE 60

// External global variables (defined in simulator.c)
extern struct Queue *job_pool;
extern MemoryWord *memory;
extern int ranges_count; // Number of processes with allocated memory ranges

// Structure to hold callback data for the dialog's "Done" button
typedef struct {
    GCallback callback;
    gpointer user_data;
} DialogCallbackData;

// Forward declaration of the function to show the process creation dialog
static void show_process_dialog(GtkButton *button, gpointer user_data);

// Static variables to store the callback and user_data for the "Done" button
static DialogCallbackData dialog_callback_data = {NULL, NULL};

// Create a new SimulatorView (sets up the main GUI)
SimulatorView *simulator_view_new(GtkApplication *app) {
    // Allocate memory for the SimulatorView struct
    SimulatorView *view = g_new(SimulatorView, 1);
    if (!view) {
        fprintf(stderr, "Error: Failed to allocate SimulatorView\n");
        return NULL;
    }

    // Initialize all fields to NULL to avoid dangling pointers
    view->window = NULL;
    view->job_pool_display = NULL;
    view->memory_list = NULL;
    view->dialog = NULL;
    view->file_entry = NULL;
    view->arrival_entry = NULL;
    view->dialog_text_view = NULL;

    // Create the main window
    GtkWidget *window = gtk_application_window_new(app);
    if (!window) {
        fprintf(stderr, "Error: Failed to create main window\n");
        g_free(view);
        return NULL;
    }
    gtk_window_set_title(GTK_WINDOW(window), "Operating Systems Simulator");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);
    view->window = GTK_WINDOW(window);

    // Create a horizontal box to split the window into left and right sections
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_margin_start(main_box, 10);
    gtk_widget_set_margin_end(main_box, 10);
    gtk_widget_set_margin_top(main_box, 10);
    gtk_widget_set_margin_bottom(main_box, 10);
    gtk_window_set_child(GTK_WINDOW(window), main_box);

    // Left section: Job Pool and Memory displays
    GtkWidget *left_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_size_request(left_box, 200, -1); // Fixed width for left section
    gtk_box_append(GTK_BOX(main_box), left_box);

    // Job Pool section with custom title
    GtkWidget *job_pool_frame = gtk_frame_new(NULL); // No title, we'll add a custom one
    gtk_box_append(GTK_BOX(left_box), job_pool_frame);

    // Custom title for job pool frame
    GtkWidget *job_pool_title = gtk_label_new("Job Pool");
    gtk_widget_add_css_class(job_pool_title, "frame-title");
    
    // Container for the job pool content
    GtkWidget *job_pool_content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    
    // Box to hold everything
    GtkWidget *job_pool_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_append(GTK_BOX(job_pool_box), job_pool_title);
    gtk_box_append(GTK_BOX(job_pool_box), job_pool_content);
    
    gtk_frame_set_child(GTK_FRAME(job_pool_frame), job_pool_box);

    GtkWidget *job_pool_display = gtk_list_box_new();
    gtk_list_box_set_selection_mode(GTK_LIST_BOX(job_pool_display), GTK_SELECTION_NONE);
    gtk_box_append(GTK_BOX(job_pool_content), job_pool_display);
    view->job_pool_display = GTK_LIST_BOX(job_pool_display);

    // "+" button to create a new process
    GtkWidget *create_button = gtk_button_new_with_label("+");
    gtk_box_append(GTK_BOX(job_pool_content), create_button);
    g_signal_connect(create_button, "clicked", G_CALLBACK(show_process_dialog), view);

    // Memory section with custom title
    GtkWidget *memory_frame = gtk_frame_new(NULL); // No title, we'll add a custom one
    gtk_widget_set_vexpand(memory_frame, TRUE); // Allow memory section to expand vertically
    gtk_box_append(GTK_BOX(left_box), memory_frame);

    // Custom title for memory frame
    GtkWidget *memory_title = gtk_label_new("Memory");
    gtk_widget_add_css_class(memory_title, "frame-title");
    
    // Container for the memory content
    GtkWidget *memory_scrolled = gtk_scrolled_window_new();
    
    // Box to hold everything
    GtkWidget *memory_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_append(GTK_BOX(memory_box), memory_title);
    gtk_box_append(GTK_BOX(memory_box), memory_scrolled);
    gtk_widget_set_vexpand(memory_scrolled, TRUE);
    
    gtk_frame_set_child(GTK_FRAME(memory_frame), memory_box);

    GtkWidget *memory_list = gtk_list_box_new();
    gtk_list_box_set_selection_mode(GTK_LIST_BOX(memory_list), GTK_SELECTION_NONE);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(memory_scrolled), memory_list);
    view->memory_list = GTK_LIST_BOX(memory_list);

    // Right section: Placeholder (no buttons since Update Memory is handled by Controller)
    GtkWidget *right_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_hexpand(right_box, TRUE); // Allow right section to expand horizontally
    gtk_box_append(GTK_BOX(main_box), right_box);

    // Apply CSS for styling
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_string(provider,
        "window { background-color: #2E2F32; }"
        "frame { background-color: #D9D9D9; border: 1px solid #bbb; color: #333; }"
        "frame > label { color: white; font-weight: bold; font-size: 14px; background-color: #33A19A; padding: 5px; border-radius: 3px 3px 0 0; }"
        "label { color: #333; font-size: 14px; }"
        "listbox { background-color: #D9D9D9; }"
        "listbox row { padding: 5px; margin: 2px; }"
        "listbox row:nth-child(even) { background-color: rgba(51, 161, 154, 0.1); }"
        "listbox row:hover { background-color: rgba(51, 161, 154, 0.2); }"
        "button { background-color: #33A19A; color: #D9D9D9; border-radius: 5px; padding: 5px; }"
        "button:hover { background-color: #278f89; }"
        "entry { background-color: white; color: #333; border: 1px solid #bbb; border-radius: 5px; padding: 5px; }"
        "textview { background-color: #e5f4f3; color: #333; border: 1px solid #bbb; padding: 5px; font-size: 12px; }"
        ".memory-tag { background-color: #33A19A; color: white; border-radius: 3px 0 0 3px; padding: 5px; font-weight: bold; margin-left: -10px; box-shadow: 1px 1px 3px rgba(0,0,0,0.3); }"
        ".memory-content { color: #333; padding: 5px; }"
        ".memory-empty { color: #777; font-style: italic; padding: 5px; }"
        ".memory-pcb { background-color: rgba(51, 161, 154, 0.15); border-radius: 5px; padding: 5px; border: 1px solid rgba(51, 161, 154, 0.3); }"
        ".memory-slot { border-bottom: 1px solid #ccc; background-color: #f5f5f5; }"
        ".frame-title { background-color: #33A19A; color: white; padding: 5px; border-radius: 3px 3px 0 0; }"
    );
    gtk_style_context_add_provider_for_display(
        gdk_display_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);

    // Show the window
    gtk_widget_set_visible(window, TRUE);

    return view;
}

// Connect the "Done" button in the process dialog to a callback
void simulator_view_connect_create_process(SimulatorView *view, GCallback callback, gpointer user_data) {
    if (!view) {
        fprintf(stderr, "Error: Cannot connect create process - view is NULL\n");
        return;
    }
    if (!callback || !user_data) {
        fprintf(stderr, "Error: Cannot connect create process - callback or user_data is NULL\n");
        return;
    }

    // Store the callback and user_data to be used in show_process_dialog
    dialog_callback_data.callback = callback;
    dialog_callback_data.user_data = user_data;
}

// Update the job pool display based on the job_pool queue
void simulator_view_update_job_pool(SimulatorView *view) {
    if (!view || !view->job_pool_display) {
        fprintf(stderr, "Error: Cannot update job pool - view or job_pool_display is NULL\n");
        return;
    }

    // Clear the current display
    while (gtk_list_box_get_row_at_index(view->job_pool_display, 0) != NULL) {
        gtk_list_box_remove(view->job_pool_display, GTK_WIDGET(gtk_list_box_get_row_at_index(view->job_pool_display, 0)));
    }

    // Add each process from job_pool to the display
    int size = getQueueSize(job_pool);
    for (int i = 0; i < size; i++) {
        char pid_str[16];
        Process *process = dequeue(job_pool);
        if (!process) {
            fprintf(stderr, "Error: Failed to dequeue process from job_pool\n");
            continue;
        }
        snprintf(pid_str, sizeof(pid_str), "p%d", process->pid); // Simple PID display (p1, p2, etc.)
        GtkWidget *label = gtk_label_new(pid_str);
        gtk_widget_add_css_class(label, "job-pool-item");
        enqueue(job_pool, process); // Re-enqueue the process to maintain job pool state
        gtk_list_box_append(view->job_pool_display, label);
    }
}

// Update the memory display based on the memory hashmap
void simulator_view_update_memory(SimulatorView *view) {
    if (!view || !view->memory_list) {
        fprintf(stderr, "Error: Cannot update memory - view or memory_list is NULL\n");
        return;
    }

    // Clear the current display
    while (gtk_list_box_get_row_at_index(view->memory_list, 0) != NULL) {
        gtk_list_box_remove(view->memory_list, GTK_WIDGET(gtk_list_box_get_row_at_index(view->memory_list, 0)));
    }

    // Iterate over memory slots (0 to MEMORY_SIZE-1)
    for (int i = 0; i < MEMORY_SIZE; i++) {
        MemoryWord *slot;
        HASH_FIND_INT(memory, &i, slot); // Look up slot i in memory hashmap

        char content_text[256] = {0}; // Buffer to hold the display text
        char *content = NULL;
        int pid = -1;
        MemoryRange range = {0, 0, 0, 0, 0, 0}; // Initialize to invalid range
        gboolean slot_mapped = FALSE;

        // First, check if the slot contains a PCB
        if (slot && slot->type == TYPE_PCB) {
            PCB *pcb = (PCB*)slot->data;
            pid = getPCBId(pcb);
            const char *state_str;
            switch (getPCBState(pcb)) {
                case NEW: state_str = "NEW"; break;
                case READY: state_str = "READY"; break;
                case RUNNING: state_str = "RUNNING"; break;
                case BLOCKED: state_str = "WAITING"; break;
                case TERMINATED: state_str = "TERMINATED"; break;
                default: state_str = "UNKNOWN";
            }

            // Format PCB details with each element on a new line
            snprintf(content_text, sizeof(content_text),
                     "Process %d\n"
                     "P%d PCB\n"
                     "State: %s\n"
                     "Priority: %d\n"
                     "PC: %d\n"
                     "Mem Bounds: %d-%d",
                     pid,
                     pid,
                     state_str,
                     getPCBPriority(pcb),
                     getPCBProgramCounter(pcb),
                     getPCBMemLowerBound(pcb),
                     getPCBMemUpperBound(pcb));
            slot_mapped = TRUE;
        }
        // If the slot isn't a PCB, check if it falls within a process's memory range
        else {
            // Iterate over all possible PIDs (1 to ranges_count)
            for (int p = 1; p <= ranges_count; p++) {
                range = getProcessMemoryRange(p);
                // Check if the range is valid (non-zero counts indicate a valid range)
                if (range.inst_count == 0 && range.var_count == 0 && range.pcb_count == 0) {
                    continue; // Skip invalid ranges
                }

                // Check if the current slot is within the instructions range
                if (i >= range.inst_start && i < range.inst_start + range.inst_count) {
                    pid = p;
                    if (slot && slot->type == TYPE_STRING) {
                        content = (char*)slot->data;
                        snprintf(content_text, sizeof(content_text), "%s", content ? content : "Empty");
                    } else {
                        snprintf(content_text, sizeof(content_text), "Empty");
                    }
                    slot_mapped = TRUE;
                    break;
                }
                // Check if the current slot is within the variables range
                else if (i >= range.var_start && i < range.var_start + range.var_count) {
                    pid = p;
                    if (slot && slot->type == TYPE_STRING) {
                        content = (char*)slot->data;
                        snprintf(content_text, sizeof(content_text), "%s", content ? content : "Empty");
                    } else {
                        snprintf(content_text, sizeof(content_text), "Empty");
                    }
                    slot_mapped = TRUE;
                    break;
                }
                // Check if the current slot is the PCB (already handled above, but for completeness)
                else if (i == range.pcb_start && range.pcb_count > 0) {
                    // This should already be handled by the TYPE_PCB case above
                    continue;
                }
            }
        }

        // If the slot wasn't mapped to any process, mark it as Empty
        if (!slot_mapped) {
            snprintf(content_text, sizeof(content_text), "Empty");
        }

        // Create a box to display the slot
        GtkWidget *slot_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
        gtk_widget_add_css_class(slot_box, "memory-slot");
        gtk_widget_set_margin_start(slot_box, 15); // Increased to allow tag to extend to the left
        gtk_widget_set_margin_end(slot_box, 5);
        gtk_widget_set_margin_top(slot_box, 2);
        gtk_widget_set_margin_bottom(slot_box, 2);

        // Create the numbered tag with teal background
        char tag_text[20];
        snprintf(tag_text, sizeof(tag_text), "%d", i);
        GtkWidget *tag_label = gtk_label_new(tag_text);
        gtk_widget_add_css_class(tag_label, "memory-tag");
        gtk_widget_set_margin_end(tag_label, 5);
        gtk_widget_set_size_request(tag_label, 30, -1); // Fixed width for number tags
        gtk_box_append(GTK_BOX(slot_box), tag_label);

        // Content area
        if (slot && slot->type == TYPE_PCB) {
            // Create a vertical box for PCB elements
            GtkWidget *pcb_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
            gtk_widget_add_css_class(pcb_box, "memory-pcb");
            gtk_widget_set_margin_start(pcb_box, 5);
            gtk_widget_set_margin_end(pcb_box, 5);
            gtk_widget_set_margin_top(pcb_box, 5);
            gtk_widget_set_margin_bottom(pcb_box, 5);
            gtk_widget_set_hexpand(pcb_box, TRUE);

            // Split the content into lines and add each as a separate label
            char *line = strtok(content_text, "\n");
            while (line != NULL) {
                GtkWidget *line_label = gtk_label_new(line);
                gtk_label_set_xalign(GTK_LABEL(line_label), 0);
                gtk_widget_add_css_class(line_label, "memory-content");
                gtk_box_append(GTK_BOX(pcb_box), line_label);
                line = strtok(NULL, "\n");
            }

            gtk_box_append(GTK_BOX(slot_box), pcb_box);
        } else {
            GtkWidget *content_label = gtk_label_new(content_text);
            gtk_label_set_xalign(GTK_LABEL(content_label), 0);
            gtk_label_set_wrap(GTK_LABEL(content_label), TRUE);
            
            // Apply "memory-empty" class for empty slots, otherwise use "memory-content"
            if (strcmp(content_text, "Empty") == 0) {
                gtk_widget_add_css_class(content_label, "memory-empty");
            } else {
                gtk_widget_add_css_class(content_label, "memory-content");
            }
            
            gtk_widget_set_hexpand(content_label, TRUE);
            
            // If the content is too long, use a tooltip to show the full content
            if (content && strlen(content) > 50) {
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
        }

        // Add to memory list
        gtk_list_box_append(view->memory_list, slot_box);
    }
    
    // Also update job pool display whenever memory is updated
    simulator_view_update_job_pool(view);
}

// Show the process creation dialog (called when "+" button is clicked)
static void show_process_dialog(GtkButton *button, gpointer user_data) {
    SimulatorView *view = (SimulatorView *)user_data;

    // Debugging
    printf("Showing process dialog: view=%p, view->window=%p\n", (void*)view, view ? (void*)view->window : NULL);

    if (!view || !view->window) {
        fprintf(stderr, "Error: Cannot show process dialog - view or view->window is NULL\n");
        return;
    }

    // Create the dialog
    GtkWidget *dialog = gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(dialog), "Create Process");
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
    gtk_window_set_transient_for(GTK_WINDOW(dialog), view->window);
    gtk_window_set_default_size(GTK_WINDOW(dialog), 400, 500);
    view->dialog = GTK_WINDOW(dialog);

    // Create dialog content (vertical box)
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_start(box, 10);
    gtk_widget_set_margin_end(box, 10);
    gtk_widget_set_margin_top(box, 10);
    gtk_widget_set_margin_bottom(box, 10);
    gtk_window_set_child(GTK_WINDOW(dialog), box);

    // File path input
    GtkWidget *file_label = gtk_label_new("Enter program file path:");
    gtk_box_append(GTK_BOX(box), file_label);
    GtkWidget *file_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(file_entry), "../programs/Program_1.txt");
    gtk_box_append(GTK_BOX(box), file_entry);
    view->file_entry = GTK_ENTRY(file_entry);

    // Arrival time input
    GtkWidget *arrival_label = gtk_label_new("Enter program arrival time:");
    gtk_box_append(GTK_BOX(box), arrival_label);
    GtkWidget *arrival_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(arrival_entry), "ex: 0");
    gtk_box_append(GTK_BOX(box), arrival_entry);
    view->arrival_entry = GTK_ENTRY(arrival_entry);

    // Done button
    GtkWidget *done_button = gtk_button_new_with_label("Done");
    gtk_box_append(GTK_BOX(box), done_button);

    // Connect the "Done" button to the stored callback
    if (dialog_callback_data.callback && dialog_callback_data.user_data) {
        g_signal_connect(done_button, "clicked", dialog_callback_data.callback, dialog_callback_data.user_data);
    } else {
        fprintf(stderr, "Warning: No callback set for Done button\n");
    }

    // Text view for output messages
    GtkWidget *text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_WORD);
    GtkWidget *scrolled_window = gtk_scrolled_window_new();
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled_window), text_view);
    gtk_widget_set_vexpand(scrolled_window, TRUE);
    gtk_box_append(GTK_BOX(box), scrolled_window);
    view->dialog_text_view = GTK_TEXT_VIEW(text_view);

    // Connect the "destroy" signal to clean up
    g_signal_connect(dialog, "destroy", G_CALLBACK(gtk_window_destroy), NULL);

    // Show the dialog
    gtk_widget_set_visible(dialog, TRUE);
}

// Get the user input from the process dialog
void simulator_view_get_process_input(SimulatorView *view, char **file_path, int *arrival_time) {
    if (!view || !view->file_entry || !view->arrival_entry) {
        fprintf(stderr, "Error: Cannot get process input - view or entries are NULL\n");
        *file_path = NULL;
        *arrival_time = 0;
        return;
    }

    *file_path = g_strdup(gtk_editable_get_text(GTK_EDITABLE(view->file_entry)));
    const char *arrival_time_str = gtk_editable_get_text(GTK_EDITABLE(view->arrival_entry));
    *arrival_time = (arrival_time_str && strlen(arrival_time_str) > 0) ? atoi(arrival_time_str) : 0;
}

// Append text to the dialog's text view (e.g., for error messages)
void simulator_view_append_dialog_text(SimulatorView *view, const char *text) {
    if (!view || !view->dialog_text_view) {
        fprintf(stderr, "Error: Cannot append dialog text - view or dialog_text_view is NULL\n");
        return;
    }

    GtkTextBuffer *buffer = gtk_text_view_get_buffer(view->dialog_text_view);
    gtk_text_buffer_set_text(buffer, "", -1); // Clear previous text
    gtk_text_buffer_insert_at_cursor(buffer, text, -1);
}

// Free the SimulatorView struct
void simulator_view_free(SimulatorView *view) {
    if (view) {
        g_free(view);
    }
}