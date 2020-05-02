#include "GladeGui.h"

#include <cstdlib>
#include <utility>

#include <config.h>

#include "GladeSearchpath.h"
#include "XojMsgBox.h"
#include "i18n.h"

GladeGui::GladeGui(const std::string& glade) { builder = gtk_builder_new_from_resource(glade.c_str()); }

GladeGui::~GladeGui() { g_object_unref(builder); }

auto GladeGui::get(const std::string& name) -> GtkWidget* {
    GtkWidget* w = GTK_WIDGET(gtk_builder_get_object(builder, name.c_str()));
    if (w == nullptr) {
        g_warning("GladeGui::get: Could not find glade Widget: \"%s\"", name.c_str());
    }
    return w;
}

auto GladeGui::getBuilder() -> GtkBuilder* { return this->builder; }

