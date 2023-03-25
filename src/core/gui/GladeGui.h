/*
 * Xournal++
 *
 * Abstract GUI class, which loads the glade objects
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>  // for string

#include <gdk/gdk.h>  // for GdkWindow
#include <gtk/gtk.h>  // for GtkWidget, GtkWindow, GtkBuilder

#include "util/raii/GObjectSPtr.h"

class GladeSearchpath;

class GladeGui {
public:
    GladeGui(GladeSearchpath* gladeSearchPath, const std::string& glade, const std::string& mainWnd);
    virtual ~GladeGui();

    virtual void show(GtkWindow* parent) = 0;

    GtkWidget* get(const std::string& name);

    void setThemePath(std::string themePath);

    GtkWidget* getWindow() const;
    GladeSearchpath* getGladeSearchPath() const;

    GtkBuilder* getBuilder() const;

private:
    /**
     * The Glade resources
     */
    xoj::util::GObjectSPtr<GtkBuilder> builder;

    /**
     * Our search paths
     */
    GladeSearchpath* gladeSearchPath;

protected:
    /**
     * This window
     */
    GtkWidget* window = nullptr;
};
