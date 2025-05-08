#include <gtk/gtk.h>
#include "view.h"

View *view = NULL; // Global, non-static
static GtkCssProvider *css_provider = NULL;

QueueAnimation queue_animations[5];

static const char *css_data =
    ".scheduler-view window { background-color: #2E2F32; }"
    ".scheduler-view frame { background-color: #D9D9D9; border: 1px solid #bbb; color: #333; }"
    ".scheduler-view frame > label { color: white; font-weight: bold; font-size: 14px; background-color: #33A19A; padding: 5px; border-radius: 3px 3px 0 0; }"
    ".scheduler-view label { color: #333; font-size: 14px; font-family: 'Roboto', 'Segoe UI', system-ui, sans-serif; }"
    ".scheduler-view listbox { background-color: #D9D9D9; }"
    ".scheduler-view listbox row { padding: 5px; margin: 2px; }"
    ".scheduler-view listbox row:nth-child(even) { background-color: rgba(51, 161, 154, 0.1); }"
    ".scheduler-view listbox row:hover { background-color: rgba(51, 161, 154, 0.3); }"
    ".scheduler-view button { background-color: #33A19A; color: #D9D9D9; border-radius: 5px; padding: 5px; }"
    ".scheduler-view button:hover { background-color: #278f89; }"
    ".scheduler-view entry { background-color: white; color: #333; border: 1px solid #bbb; border-radius: 5px; padding: 5px; }"
    ".scheduler-view drop-down { background-color: white; color: #333; border: 1px solid #bbb; border-radius: 5px; padding: 5px; }"
    ".scheduler-view textview { background-color: white; color: #000; border: 1px solid #bbb; padding: 8px; font-family: 'Roboto', 'Segoe UI', system-ui, sans-serif; font-size: 13px; }"
    ".scheduler-view .memory-tag { background-color: #33A19A; color: white; border-radius: 3px 0 0 3px; padding: 5px; font-weight: bold; margin-left: -10px; box-shadow: 1px 1px 3px rgba(0,0,0,0.3); }"
    ".scheduler-view .memory-content { color: #333; padding: 5px; }"
    ".scheduler-view .memory-content:hover { color: #196761; }"
    ".scheduler-view .memory-empty { color: #777; font-style: italic; padding: 5px; }"
    ".scheduler-view .memory-pcb { background-color: #f5f5f5; border-radius: 0 0 5px 5px; padding: 0; border: 1px solid #33A19A; }"
    ".scheduler-view .memory-pcb-row { padding: 5px 10px; border-bottom: 1px solid rgba(51, 161, 154, 0.2); }"
    ".scheduler-view .memory-pcb-row:last-child { border-bottom: none; }"
    ".scheduler-view .memory-slot { border-bottom: 1px solid #ccc; background-color: #f5f5f5; }"
    ".scheduler-view .memory-slot:hover .memory-content { color: #196761; }"
    ".scheduler-view .frame-title { background-color: #33A19A; color: white; padding: 5px; border-radius: 3px 3px 0 0; }"
    ".scheduler-view .pcb-tab { background-color: #33A19A; color: white; padding: 6px 12px; border-radius: 5px 5px 0 0; font-weight: bold; margin-bottom: 0; }"
    ".scheduler-view .process-title { background-color: #196761; color: white; padding: 4px 10px; border-radius: 5px 5px 0 0; font-weight: bold; margin-bottom: 0; font-size: 13px; }"
    ".scheduler-view .queue-area { background-color: #D9D9D9; border: 1px solid #bbb; border-radius: 5px; }"
    ".scheduler-view .running-process { background-color: #196761; color: white; font-size: 14px; font-family: 'Roboto', 'Segoe UI', system-ui, sans-serif; padding: 8px; border-radius: 5px; margin-top: 10px; box-shadow: 0 2px 4px rgba(0,0,0,0.2); }"
    ".scheduler-view .blocked-queue-header { background-color: #FF0000; color: white; padding: 5px; border-radius: 3px 3px 0 0; }"
    ".scheduler-view .blocked-process { background-color: #FF0000; color: white; border-radius: 5px; }";

static void draw_queue_callback(GtkDrawingArea *area, cairo_t *cr, int width, int height, gpointer data)
{
    int queue_index = GPOINTER_TO_INT(data);
    GList *processes = view->queue_processes[queue_index];
    GList *anim_iter = queue_animations[queue_index].animations;

    cairo_set_source_rgba(cr, 0, 0, 0, 0);
    cairo_paint(cr);

    if (queue_index == 4)
    {
        cairo_set_source_rgb(cr, 1.0, 0.0, 0.0);
    }
    else
    {
        cairo_set_source_rgb(cr, 0.2, 0.631, 0.604);
    }
    cairo_rectangle(cr, 0, 0, width, 25);
    cairo_fill(cr);

    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_select_font_face(cr, "Roboto", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 14);

    if (queue_index == 4)
    {
        cairo_move_to(cr, 10, 18);
        cairo_show_text(cr, "Blocked Queue");
    }
    else
    {
        cairo_move_to(cr, 10, 18);
        char label[20];
        snprintf(label, sizeof(label), "Ready Queue %d", queue_index);
        cairo_show_text(cr, label);
    }

    GList *proc_iter;
    int process_count = 0;

    for (proc_iter = processes; proc_iter != NULL; proc_iter = proc_iter->next)
    {
        int pid = GPOINTER_TO_INT(proc_iter->data);
        ProcessAnimation *anim = NULL;

        for (GList *a = anim_iter; a != NULL; a = a->next)
        {
            ProcessAnimation *pa = (ProcessAnimation *)a->data;
            if (pa->pid == pid)
            {
                anim = pa;
                break;
            }
        }

        float x, y;
        float alpha = 1.0;

        if (queue_index == 4)
        {
            x = width / 2;
            y = 70 + process_count * 60;
        }
        else
        {
            x = 120 + process_count * 80;
            y = height / 2 + 5;
        }

        if (anim && anim->animating)
        {
            float t = (float)anim->steps / anim->total_steps;
            x = anim->start_x + t * (anim->end_x - anim->start_x);
            y = anim->start_y + t * (anim->end_y - anim->start_y);
            alpha = anim->alpha;
        }

        cairo_pattern_t *box_gradient = cairo_pattern_create_linear(x - 25, y - 20, x + 25, y + 20);

        if (pid == view->running_pid)
        {
            cairo_pattern_add_color_stop_rgba(box_gradient, 0, 0.098, 0.529, 0.380, alpha);
            cairo_pattern_add_color_stop_rgba(box_gradient, 1, 0.059, 0.388, 0.278, alpha);
        }
        else if (queue_index == 4)
        {
            cairo_pattern_add_color_stop_rgba(box_gradient, 0, 1.0, 0.0, 0.0, alpha);
            cairo_pattern_add_color_stop_rgba(box_gradient, 1, 0.8, 0.0, 0.0, alpha);
        }
        else
        {
            cairo_pattern_add_color_stop_rgba(box_gradient, 0, 0.6, 0.3, 0.7, alpha);
            cairo_pattern_add_color_stop_rgba(box_gradient, 1, 0.4, 0.1, 0.5, alpha);
        }

        cairo_set_source(cr, box_gradient);

        double box_radius = 8.0;
        double degrees = M_PI / 180.0;
        cairo_new_sub_path(cr);
        cairo_arc(cr, x + 20 - box_radius, y - 15 + box_radius, box_radius, -90 * degrees, 0 * degrees);
        cairo_arc(cr, x + 20 - box_radius, y + 15 - box_radius, box_radius, 0 * degrees, 90 * degrees);
        cairo_arc(cr, x - 20 + box_radius, y + 15 - box_radius, box_radius, 90 * degrees, 180 * degrees);
        cairo_arc(cr, x - 20 + box_radius, y - 15 + box_radius, box_radius, 180 * degrees, 270 * degrees);
        cairo_close_path(cr);

        cairo_fill(cr);

        cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.2);
        cairo_set_line_width(cr, 1);
        cairo_new_sub_path(cr);
        cairo_arc(cr, x + 20 - box_radius, y - 15 + box_radius, box_radius, -90 * degrees, 0 * degrees);
        cairo_arc(cr, x + 20 - box_radius, y + 15 - box_radius, box_radius, 0 * degrees, 90 * degrees);
        cairo_arc(cr, x - 20 + box_radius, y + 15 - box_radius, box_radius, 90 * degrees, 180 * degrees);
        cairo_arc(cr, x - 20 + box_radius, y - 15 + box_radius, box_radius, 180 * degrees, 270 * degrees);
        cairo_close_path(cr);
        cairo_stroke(cr);

        cairo_pattern_destroy(box_gradient);

        char pid_str[16];
        snprintf(pid_str, sizeof(pid_str), "P%d", pid);
        cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, alpha);
        cairo_select_font_face(cr, "Roboto", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
        cairo_set_font_size(cr, 14);

        cairo_text_extents_t extents;
        cairo_text_extents(cr, pid_str, &extents);
        cairo_move_to(cr, x - extents.width / 2, y + extents.height / 2);
        cairo_show_text(cr, pid_str);

        if (proc_iter->next != NULL)
        {
            if (queue_index == 4)
            {
                float next_y = 50 + (process_count + 1) * 50;
                cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
                cairo_move_to(cr, x, y + 15);
                cairo_line_to(cr, x, y + 30);
                cairo_stroke(cr);
                cairo_move_to(cr, x, y + 30);
                cairo_line_to(cr, x - 5, y + 25);
                cairo_move_to(cr, x, y + 30);
                cairo_line_to(cr, x + 5, y + 25);
                cairo_stroke(cr);
            }
            else
            {
                x += 70;
                cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
                cairo_move_to(cr, x - 40, y);
                cairo_line_to(cr, x - 20, y);
                cairo_stroke(cr);
                cairo_move_to(cr, x - 20, y);
                cairo_line_to(cr, x - 25, y - 5);
                cairo_move_to(cr, x - 20, y);
                cairo_line_to(cr, x - 25, y + 5);
                cairo_stroke(cr);
            }
        }

        process_count++;
    }
}

static void draw_legend_callback(GtkDrawingArea *area, cairo_t *cr, int width, int height, gpointer data)
{
    int x = 30;
    int y = height / 2;
    double circle_radius = 12;
    double degrees = M_PI / 180.0;

    cairo_set_source_rgba(cr, 0, 0, 0, 0);
    cairo_paint(cr);

    cairo_pattern_t *ready_gradient = cairo_pattern_create_radial(x, y, 0, x, y, circle_radius);
    cairo_pattern_add_color_stop_rgba(ready_gradient, 0, 0.6, 0.3, 0.7, 1.0);
    cairo_pattern_add_color_stop_rgba(ready_gradient, 1, 0.4, 0.1, 0.5, 1.0);
    cairo_set_source(cr, ready_gradient);
    cairo_arc(cr, x, y, circle_radius, 0, 2 * M_PI);
    cairo_fill(cr);
    cairo_pattern_destroy(ready_gradient);

    cairo_set_source_rgb(cr, 0.2, 0.2, 0.2);
    cairo_select_font_face(cr, "Roboto", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 14);
    cairo_move_to(cr, x + 20, y + 5);
    cairo_show_text(cr, "Ready");

    x += 100;

    cairo_pattern_t *running_gradient = cairo_pattern_create_radial(x, y, 0, x, y, circle_radius);
    cairo_pattern_add_color_stop_rgba(running_gradient, 0, 0.098, 0.529, 0.380, 1.0);
    cairo_pattern_add_color_stop_rgba(running_gradient, 1, 0.059, 0.388, 0.278, 1.0);
    cairo_set_source(cr, running_gradient);
    cairo_arc(cr, x, y, circle_radius, 0, 2 * M_PI);
    cairo_fill(cr);
    cairo_pattern_destroy(running_gradient);

    cairo_set_source_rgb(cr, 0.2, 0.2, 0.2);
    cairo_move_to(cr, x + 20, y + 5);
    cairo_show_text(cr, "Running");

    x += 100;

    cairo_pattern_t *blocked_gradient = cairo_pattern_create_radial(x, y, 0, x, y, circle_radius);
    cairo_pattern_add_color_stop_rgba(blocked_gradient, 0, 1.0, 0.0, 0.0, 1.0);
    cairo_pattern_add_color_stop_rgba(blocked_gradient, 1, 0.8, 0.0, 0.0, 1.0);
    cairo_set_source(cr, blocked_gradient);
    cairo_arc(cr, x, y, circle_radius, 0, 2 * M_PI);
    cairo_fill(cr);
    cairo_pattern_destroy(blocked_gradient);

    cairo_set_source_rgb(cr, 0.2, 0.2, 0.2);
    cairo_move_to(cr, x + 20, y + 5);
    cairo_show_text(cr, "Blocked");
}

static gboolean animate_process(gpointer user_data)
{
    int queue_index = GPOINTER_TO_INT(user_data);
    GList *anim_iter = queue_animations[queue_index].animations;
    gboolean any_animating = FALSE;

    for (GList *a = anim_iter; a != NULL; a = a->next)
    {
        ProcessAnimation *anim = (ProcessAnimation *)a->data;
        if (anim->animating)
        {
            anim->steps++;
            if (anim->steps >= anim->total_steps)
            {
                anim->animating = 0;
                anim->alpha = 1.0;
            }
            else
            {
                float progress = (float)anim->steps / anim->total_steps;
                anim->alpha = progress < 0.5 ? 4 * progress * progress * progress : 1 - pow(-2 * progress + 2, 3) / 2;
                any_animating = TRUE;
            }
        }
    }

    gtk_widget_queue_draw(view->drawing_areas[queue_index]);
    return any_animating ? G_SOURCE_CONTINUE : G_SOURCE_REMOVE;
}

void view_init(GtkWidget *window, GtkWidget *main_box)
{
    view = g_new0(View, 1);
    css_provider = gtk_css_provider_new();
    gtk_css_provider_load_from_string(css_provider, css_data);
    gtk_style_context_add_provider_for_display(
        gdk_display_get_default(),
        GTK_STYLE_PROVIDER(css_provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    view->window = window;
    gtk_widget_add_css_class(window, "scheduler-view");

    // Use provided main_box instead of creating a new one
    g_print("view_init: using main_box: %p\n", main_box);

    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 15);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 20);
    gtk_box_append(GTK_BOX(main_box), grid);

    view->drawing_areas[4] = gtk_drawing_area_new();
    gtk_widget_set_size_request(view->drawing_areas[4], 130, 320);
    gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(view->drawing_areas[4]),
                                   draw_queue_callback,
                                   GINT_TO_POINTER(4),
                                   NULL);
    gtk_widget_add_css_class(view->drawing_areas[4], "queue-area");
    gtk_grid_attach(GTK_GRID(grid), view->drawing_areas[4], 0, 0, 1, 4);

    for (int i = 0; i < 4; i++)
    {
        view->drawing_areas[i] = gtk_drawing_area_new();
        gtk_widget_set_size_request(view->drawing_areas[i], 720, 90);
        gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(view->drawing_areas[i]),
                                       draw_queue_callback,
                                       GINT_TO_POINTER(i),
                                       NULL);
        gtk_widget_add_css_class(view->drawing_areas[i], "queue-area");
        gtk_grid_attach(GTK_GRID(grid), view->drawing_areas[i], 1, i, 1, 1);
    }

    GtkWidget *legend_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(legend_area, 720, 40);
    gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(legend_area),
                                   draw_legend_callback,
                                   NULL,
                                   NULL);
    gtk_widget_add_css_class(legend_area, "queue-area");
    gtk_grid_attach(GTK_GRID(grid), legend_area, 0, 4, 2, 1);

    view->running_process_label = gtk_label_new("Process Info: None");
    gtk_widget_add_css_class(view->running_process_label, "running-process");
    gtk_label_set_xalign(GTK_LABEL(view->running_process_label), 0.0);
    gtk_widget_set_margin_top(view->running_process_label, 10);
    gtk_box_append(GTK_BOX(main_box), view->running_process_label);

    GtkWidget *button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 15);
    gtk_widget_set_margin_top(button_box, 15);
    gtk_widget_set_halign(button_box, GTK_ALIGN_CENTER);
    
    GtkWidget *scheduler_label = gtk_label_new("Scheduler:");
    gtk_widget_add_css_class(scheduler_label, "label");
    gtk_box_append(GTK_BOX(button_box), scheduler_label);

    GtkStringList *scheduler_list = gtk_string_list_new(NULL);
    gtk_string_list_append(scheduler_list, "MLFQ");
    gtk_string_list_append(scheduler_list, "FCFS");
    gtk_string_list_append(scheduler_list, "Round Robin");
    view->scheduler_combo = gtk_drop_down_new(G_LIST_MODEL(scheduler_list), NULL);
    gtk_drop_down_set_selected(GTK_DROP_DOWN(view->scheduler_combo), 0);
    gtk_widget_add_css_class(view->scheduler_combo, "drop-down");
    gtk_box_append(GTK_BOX(button_box), view->scheduler_combo);

    view->quantum_label = gtk_label_new("Quantum:");
    gtk_widget_add_css_class(view->quantum_label, "label");
    view->quantum_entry = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(view->quantum_entry), "2");
    gtk_entry_set_max_length(GTK_ENTRY(view->quantum_entry), 3);
    gtk_widget_set_size_request(view->quantum_entry, 50, -1);
    gtk_widget_add_css_class(view->quantum_entry, "entry");
    gtk_widget_set_visible(view->quantum_label, FALSE);
    gtk_widget_set_visible(view->quantum_entry, FALSE);
    gtk_box_append(GTK_BOX(button_box), view->quantum_label);
    gtk_box_append(GTK_BOX(button_box), view->quantum_entry);

    view->step_button = gtk_button_new_with_label("Step");
    gtk_widget_add_css_class(view->step_button, "button");
    view->automatic_button = gtk_button_new_with_label("Automatic");
    gtk_widget_add_css_class(view->automatic_button, "button");
    view->pause_button = gtk_button_new_with_label("Pause");
    gtk_widget_add_css_class(view->pause_button, "button");
    view->reset_button = gtk_button_new_with_label("Reset");
    gtk_widget_add_css_class(view->reset_button, "button");
    gtk_widget_set_sensitive(view->pause_button, FALSE);

    gtk_box_append(GTK_BOX(button_box), view->step_button);
    gtk_box_append(GTK_BOX(button_box), view->automatic_button);
    gtk_box_append(GTK_BOX(button_box), view->pause_button);
    gtk_box_append(GTK_BOX(button_box), view->reset_button);

    gtk_box_append(GTK_BOX(main_box), button_box);

    for (int i = 0; i < 5; i++)
    {
        view->queue_processes[i] = NULL;
        queue_animations[i].animations = NULL;
    }

    view->running_pid = -1;
}

static int get_position_in_queue(GList *queue, int pid)
{
    int pos = 0;
    for (GList *p = queue; p != NULL; p = p->next)
    {
        if (GPOINTER_TO_INT(p->data) == pid)
        {
            return pos;
        }
        pos++;
    }
    return -1;
}

static void get_process_coords(int queue_index, int position, float *x, float *y)
{
    if (queue_index == 4)
    {
        *x = 65;
        *y = 70 + position * 60;
    }
    else
    {
        *x = 120 + position * 80;
        *y = 40 + queue_index * 75;
    }
}

void view_update_queue(int queue_index, GList *processes, int running_pid)
{
    if (queue_index < 0 || queue_index >= 5)
        return;

    g_list_free_full(queue_animations[queue_index].animations, g_free);
    queue_animations[queue_index].animations = NULL;

    GList *old_processes = view->queue_processes[queue_index];
    GList *new_processes = processes;

    int new_position = 0;
    for (GList *proc_iter = processes; proc_iter != NULL; proc_iter = proc_iter->next)
    {
        int pid = GPOINTER_TO_INT(proc_iter->data);
        ProcessAnimation *anim = g_new0(ProcessAnimation, 1);

        anim->pid = pid;
        anim->animating = 1;
        anim->steps = 0;
        anim->total_steps = 20;

        int old_queue = -1;
        int old_position = -1;

        for (int q = 0; q < 5; q++)
        {
            int pos = get_position_in_queue(view->queue_processes[q], pid);
            if (pos != -1)
            {
                old_queue = q;
                old_position = pos;
                break;
            }
        }

        get_process_coords(queue_index, new_position, &anim->end_x, &anim->end_y);

        if (old_queue == -1)
        {
            anim->start_x = anim->end_x;
            anim->start_y = anim->end_y;
            anim->alpha = 0.0;
        }
        else
        {
            get_process_coords(old_queue, old_position, &anim->start_x, &anim->start_y);
            anim->alpha = 1.0;
        }

        queue_animations[queue_index].animations = g_list_append(queue_animations[queue_index].animations, anim);
        new_position++;
    }

    g_list_free(view->queue_processes[queue_index]);
    view->queue_processes[queue_index] = g_list_copy(processes);

    view->running_pid = running_pid;

    g_timeout_add(16, animate_process, GINT_TO_POINTER(queue_index));
}

GtkWidget *view_get_running_process_label()
{
    return view->running_process_label;
}

GtkWidget *view_get_step_button()
{
    return view->step_button;
}

GtkWidget *view_get_automatic_button()
{
    return view->automatic_button;
}

GtkWidget *view_get_pause_button()
{
    return view->pause_button;
}

GtkWidget *view_get_reset_button()
{
    return view->reset_button;
}

GtkWidget *view_get_scheduler_combo()
{
    return view->scheduler_combo;
}

GtkWidget *view_get_quantum_entry()
{
    return view->quantum_entry;
}

GtkWidget *view_get_quantum_label()
{
    return view->quantum_label;
}