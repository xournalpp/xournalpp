

#pragma once

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

public:
    Control* getControl() const;
    void show(GtkWindow* app) override;
    void openFirstXoppFile();

private:
    void initHomeWidget();

private:
    Control* control;
    MainWindow* win;

    static void on_buttonNewDocument_clicked(GtkButton* button, gpointer user_data);
    static void on_buttonOpenRecentDocument_clicked(GtkButton* button, gpointer user_data);
};
