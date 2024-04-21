/*
 * Xournal++
 *
 * gtk_builder wrapper to parse ui files
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <iostream>
#include <string>

#include <gtk/gtk.h>

#include "util/Assert.h"
#include "util/raii/GObjectSPtr.h"

class GladeSearchpath;

class Builder final {
public:
    Builder(GladeSearchpath* gladeSearchPath, const std::string& uiFile);

    inline GObject* getObject(const std::string& name) {
        GObject* res = gtk_builder_get_object(builder.get(), name.c_str());
        xoj_assert_message(res, "ERROR: Builder::get: Could not find UI object: " + name + "\n");
        return res;
    }

    inline GtkWidget* get(const std::string& name) { return GTK_WIDGET(getObject(name.c_str())); }

private:
    /**
     * The Glade resources
     */
    xoj::util::GObjectSPtr<GtkBuilder> builder;
};
