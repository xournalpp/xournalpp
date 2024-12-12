

#pragma once

#include "GladeGui.h"
#include <glib-object.h>  // for GClosure
#include <glib.h>         // for gpointer, gboolean, gint
#include <gtk/gtk.h>      // for GtkWidget, GtkCheckMenu...
#include "MainWindow.h"
#include "control/Control.h"

class Control;
class GladeSearchpath;

class HomeWindow : public GladeGui {
public:
    HomeWindow(GladeSearchpath* gladeSearchPath, Control* control, GtkApplication* app, MainWindow* win);
    ~HomeWindow() override;

public:
    void show(GtkWindow* app) override;

private:
    void initHomeWidget();

private:
    Control* control;
    MainWindow* win;
    
    static void on_buttonNewDocument_clicked(GtkButton* button, gpointer user_data);
	static void on_buttonOpenRecentDocument_clicked(GtkButton* button, gpointer user_data);
};
