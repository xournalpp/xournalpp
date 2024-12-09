

#pragma once

#include "GladeGui.h"
#include <glib-object.h>  // for GClosure
#include <glib.h>         // for gpointer, gboolean, gint
#include <gtk/gtk.h>      // for GtkWidget, GtkCheckMenu...
#include "MainWindow.h"

class Control;
class GladeSearchpath;

class HomeWindow : public GladeGui {
public:
    HomeWindow(GladeSearchpath* gladeSearchPath, Control* control, GtkApplication* app, MainWindow* win);
    ~HomeWindow() override;

public:
    void show(GtkWindow* app) override;

public:
    Control* getControl() const;

private:
    void initHomeWidget();

private:
    Control* control;
    MainWindow* win;
    GtkWidget* winHome = nullptr;
   // static void on_button_click_me_clicked(MainWindow* win); //se quitaron los parametros para prueba
    static void on_button_click_me_clicked(GtkButton* button, gpointer user_data);
};
