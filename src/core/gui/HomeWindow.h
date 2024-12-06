

#pragma once

#include "GladeGui.h"
#include <glib-object.h>  // for GClosure
#include <glib.h>         // for gpointer, gboolean, gint
#include <gtk/gtk.h>      // for GtkWidget, GtkCheckMenu...

class Control;
class GladeSearchpath;

class HomeWindow : public GladeGui {
public:
    HomeWindow(GladeSearchpath* gladeSearchPath, Control* control, GtkApplication* app);
    ~HomeWindow() override;

public:
    void show(GtkWindow* app) override;

public:
    Control* getControl() const;

private:
    void initHomeWidget();

private:
    Control* control;
    GtkWidget* winHome = nullptr;
    static void on_button_click_me_clicked(GtkButton* button, gpointer user_data);
};
