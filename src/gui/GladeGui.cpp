#include "GladeGui.h"

#include "GladeSearchpath.h"

#include <config.h>
#include <i18n.h>
#include <XojMsgBox.h>

#include <stdlib.h>

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

GtkWidget* GladeGui::get(string name)
{
	GtkWidget* w = GTK_WIDGET(gtk_builder_get_object(builder, name.c_str()));
	if (w == nullptr)
	{
		g_warning("GladeGui::get: Could not find glade Widget: \"%s\"", name.c_str());
	}
	return w;
}

GtkWidget* GladeGui::getWindow()
{
	return this->window;
}

GladeSearchpath* GladeGui::getGladeSearchPath()
{
	return this->gladeSearchPath;
}

GtkBuilder* GladeGui::getBuilder()
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
