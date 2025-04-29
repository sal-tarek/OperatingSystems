#ifndef SIMULATOR_VIEW_H
#define SIMULATOR_VIEW_H

#include <gtk/gtk.h>

// Forward declaration of structs defined elsewhere
struct Queue;
struct MemoryWord;

// SimulatorView holds all GUI components
typedef struct {
    GtkWindow *window;              // Main application window
    GtkListBox *job_pool_display;   // List box to display job pool processes
    GtkListBox *memory_list;        // List box to display memory slots
    GtkWindow *dialog;              // Dialog window for creating processes
    GtkEntry *file_entry;           // Entry field for file path in dialog
    GtkEntry *arrival_entry;        // Entry field for arrival time in dialog
    GtkTextView *dialog_text_view;  // Text view for dialog output messages
} SimulatorView;

// Function declarations
SimulatorView *simulator_view_new(GtkApplication *app);
void simulator_view_connect_create_process(SimulatorView *view, GCallback callback, gpointer user_data);
void simulator_view_update_job_pool(SimulatorView *view);
void simulator_view_update_memory(SimulatorView *view);
void simulator_view_get_process_input(SimulatorView *view, char **file_path, int *arrival_time);
void simulator_view_append_dialog_text(SimulatorView *view, const char *text);
void simulator_view_free(SimulatorView *view);

#endif // SIMULATOR_VIEW_H