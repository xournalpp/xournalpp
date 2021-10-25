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
    operator GdkSurface*();

    template <typename Func>
    auto get(std::string const& name, Func&& func) -> decltype(std::declval<Func>()(std::declval<GObject*>())) {
        auto* o = func(gtk_builder_get_object(builder, name.c_str()));
        if (o == nullptr) {
            g_warning("GladeGui::get: Could not find glade Object: \"%s\"", name.c_str());
        }
        return o;
    }

    auto get(std::string const& name) -> GtkWidget*;

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
