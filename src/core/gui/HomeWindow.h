

#pragma once

#include "GladeGui.h"
#include <glib-object.h>  // for GClosure
#include <glib.h>         // for gpointer, gboolean, gint
#include <gtk/gtk.h>      // for GtkWidget, GtkCheckMenu...
#include "MainWindow.h"
#include "control/Control.h"
#include <vector>
#include <string>

class Control;
class GladeSearchpath;

class HomeWindow : public GladeGui {
public:
    HomeWindow(GladeSearchpath* gladeSearchPath, Control* control, GtkApplication* app, MainWindow* win);
    ~HomeWindow() override;
    void initHomeWidget();
    void show(GtkWindow* app) override;
    
private:
    Control* control;
    MainWindow* win;
    GtkWidget* recentDocumentsGrid;
    GtkAllocation allocation;

    std::vector<std::string> getRecentDocuments();
    void createRecentDocumentButtons(int button_width, int button_height);
    static void on_recent_document_button_clicked(GtkButton* button, gpointer user_data);
    
    static void on_buttonNewDocument_clicked(GtkButton* button, gpointer user_data);

};
