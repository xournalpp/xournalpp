#include "MainWindow.h"

#include <gtk/gtk.h>

MainWindow::MainWindow(GtkApplication* app, XournalppStore store): store(std::move(store)) {
    this->window = reinterpret_cast<GtkWindow*>(gtk_application_window_new(app));
    auto scrolled = gtk_scrolled_window_new(NULL, NULL);
    auto child = gtk_image_new_from_file("/home/julius/Bilder/mr-robot-fsociety-logo-minimalism-qs-2560x1440.jpg");
    gtk_widget_set_size_request(child, 1000, 1000);
    gtk_scrolled_window_set_child(reinterpret_cast<GtkScrolledWindow*>(scrolled), child);
    gtk_scrolled_window_set_capture_button_press(reinterpret_cast<GtkScrolledWindow*>(scrolled), false);
    gtk_widget_set_hexpand(scrolled, true);
    gtk_widget_set_vexpand(scrolled, true);
    auto dragGesture = gtk_gesture_drag_new();
    g_signal_connect(dragGesture, "drag-begin", (GCallback)dragBegin, nullptr);
    g_signal_connect(dragGesture, "drag-update", (GCallback)dragUpdate, nullptr);
    g_signal_connect(dragGesture, "drag-end", (GCallback)dragEnd, nullptr);
    gtk_widget_add_controller(child, reinterpret_cast<GtkEventController*>(dragGesture));
    auto box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_box_append(GTK_BOX(box), scrolled);
    auto adjustment = gtk_adjustment_new(5, 0, 10, 0.1, 1, 1);
    auto scrollbar = gtk_scrollbar_new(GTK_ORIENTATION_HORIZONTAL, adjustment);
    gtk_box_append(GTK_BOX(box), scrollbar);
    gtk_window_set_child(this->window, box);
}

auto MainWindow::show() -> void {
    gtk_window_set_default_size(this->window, 600, 600);
    gtk_window_present(this->window);
}

auto MainWindow::dragBegin(GtkGestureDrag* gesture, double start_x, double start_y, gpointer user_data) -> void {
    g_message("Drag begin");
    auto sequence = gtk_gesture_single_get_current_sequence(reinterpret_cast<GtkGestureSingle*>(gesture));
    gtk_gesture_set_sequence_state(reinterpret_cast<GtkGesture*>(gesture), sequence, GTK_EVENT_SEQUENCE_DENIED);
}

auto MainWindow::dragEnd(GtkGestureDrag* gesture, double offset_x, double offset_y, gpointer user_data) -> void {
    g_message("Drag end");
}

auto MainWindow::dragUpdate(GtkGestureDrag* gesture, double offset_x, double offset_y, gpointer user_data) -> void {
    g_message("Drag update");
}
