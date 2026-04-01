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

    template <class GObjectType>
    GObjectType* get(const std::string& name) {
        GObjectType* res = static_cast<GObjectType*>(gtk_builder_get_object(builder.get(), name.c_str()));
        xoj_assert_message(res, "ERROR: Builder::get: Could not find UI object: " + name + "\n");
        return res;
    }

    inline GtkWidget* get(const std::string& name) { return GTK_WIDGET(get<GObject>(name)); }

private:
    /**
     * The Glade resources
     */
    xoj::util::GObjectSPtr<GtkBuilder> builder;
};
