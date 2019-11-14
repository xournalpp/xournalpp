#include "GladeGui.h"

#include "GladeSearchpath.h"

#include <config.h>
#include <i18n.h>
#include <XojMsgBox.h>

#include <cstdlib>

GladeGui::GladeGui(GladeSearchpath* gladeSearchPath, string glade, string mainWnd)
{
	this->gladeSearchPath = gladeSearchPath;

	string filename = this->gladeSearchPath->findFile("", glade);

	GError* error = nullptr;
	builder = gtk_builder_new();

	if (!gtk_builder_add_from_file(builder, filename.c_str(), &error))
	{
		string msg = FS(_F("Error loading glade file \"{1}\" (try to load \"{2}\")") % glade % filename);

		if (error != nullptr)
		{
			msg += "\n";
			msg += error->message;
		}
		XojMsgBox::showErrorToUser(nullptr, msg);

		g_error_free(error);

		exit(-1);
	}

	this->window = get(mainWnd);
}

GladeGui::~GladeGui()
{
	g_object_unref(builder);
}

auto GladeGui::get(string name) -> GtkWidget*
{
	GtkWidget* w = GTK_WIDGET(gtk_builder_get_object(builder, name.c_str()));
	if (w == nullptr)
	{
		g_warning("GladeGui::get: Could not find glade Widget: \"%s\"", name.c_str());
	}
	return w;
}

auto GladeGui::getWindow() -> GtkWidget*
{
	return this->window;
}

auto GladeGui::getGladeSearchPath() -> GladeSearchpath*
{
	return this->gladeSearchPath;
}

auto GladeGui::getBuilder() -> GtkBuilder*
{
	return this->builder;
}

GladeGui::operator GdkWindow* ()
{
	return gtk_widget_get_window(GTK_WIDGET(getWindow()));
}

GladeGui::operator GtkWindow* ()
{
	return GTK_WINDOW(getWindow());
}
