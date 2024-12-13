/*
 * HomeWindow.h
 *
 * The new start home page
 * 
 *  This file defines the HomeWindow class, which is responsible for managing the new home window of the application.
 * The HomeWindow class provides functionality to initialize and display the home window, handle user interactions,
 * and manage recent documents.
 */

#pragma once

#include <string>
#include <vector>

#include <glib-object.h>  // for GClosure
#include <glib.h>         // for gpointer, gboolean, gint
#include <gtk/gtk.h>      // for GtkWidget, GtkCheckMenu...

#include "control/Control.h"

#include "GladeGui.h"
#include "MainWindow.h"

class Control;
class GladeSearchpath;

class HomeWindow: public GladeGui {
public:
    HomeWindow(GladeSearchpath* gladeSearchPath, Control* control, GtkApplication* app, MainWindow* win);
    ~HomeWindow() override;
    void initHomeWidget();
    void show(GtkWindow* app) override;
    void openFirstXoppFile();
    Control* getControl() const;

private:
    Control* control;
    MainWindow* win;
    GtkWidget* recentDocumentsGrid;
    GtkAllocation allocation;

    std::vector<std::string> getRecentDocuments();
    void createRecentDocumentButtons(int button_width, int button_height, const gchar* search_text = "");

    static void on_searchDocumentEntry_activate(GtkEntry* entry, gpointer user_data); // Search for a recent document
    static void on_recentDocument_button_clicked(GtkButton* button, gpointer user_data); // Open a recent document
    static void on_buttonNewDocument_clicked(GtkButton* button, gpointer user_data); // Create a new document
};
