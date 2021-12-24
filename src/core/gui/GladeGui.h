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

#include <string>
#include <vector>

#include <gtk/gtk.h>


class GladeSearchpath;

class GladeGui {
public:
    GladeGui(GladeSearchpath* gladeSearchPath, const std::string& glade, const std::string& mainWnd);
    virtual ~GladeGui();

    virtual void show(GtkWindow* parent) = 0;

    operator GtkWindow*();
    operator GdkWindow*();

    GtkWidget* get(const std::string& name);

    void setThemePath(std::string themePath);

    GtkWidget* getWindow();
    GladeSearchpath* getGladeSearchPath();

    GtkBuilder* getBuilder();

private:
    /**
     * The Glade resources
     */
    GtkBuilder* builder;

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
