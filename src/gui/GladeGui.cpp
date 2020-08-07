#include "GladeGui.h"

#include <cstdlib>
#include <utility>

#include <config.h>

#include "GladeSearchpath.h"
#include "XojMsgBox.h"
#include "i18n.h"

GladeGui::GladeGui(GladeSearchpath* gladeSearchPath, const string& glade, const string& mainWnd) {
    this->gladeSearchPath = gladeSearchPath;

    auto filepath = this->gladeSearchPath->findFile("", glade);

    GError* error = nullptr;
    builder = gtk_builder_new();

    if (!gtk_builder_add_from_file(builder, filepath.u8string().c_str(), &error)) {
        string msg = FS(_F("Error loading glade file \"{1}\" (try to load \"{2}\")") % glade % filepath.string());

        if (error != nullptr) {
            msg += "\n";
            msg += error->message;
        }
        XojMsgBox::showErrorToUser(nullptr, msg);

        g_error_free(error);

        exit(-1);
    }

    this->window = get(mainWnd);
}

GladeGui::~GladeGui() { g_object_unref(builder); }

auto GladeGui::get(const string& name) -> GtkWidget* {
    GtkWidget* w = GTK_WIDGET(gtk_builder_get_object(builder, name.c_str()));
    if (w == nullptr) {
        g_warning("GladeGui::get: Could not find glade Widget: \"%s\"", name.c_str());
    }
    return w;
}

auto GladeGui::getWindow() -> GtkWidget* { return this->window; }

auto GladeGui::getGladeSearchPath() -> GladeSearchpath* { return this->gladeSearchPath; }

auto GladeGui::getBuilder() -> GtkBuilder* { return this->builder; }

GladeGui::operator GdkWindow*() { return gtk_widget_get_window(GTK_WIDGET(getWindow())); }

GladeGui::operator GtkWindow*() { return GTK_WINDOW(getWindow()); }
