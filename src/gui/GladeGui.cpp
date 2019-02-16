#include "GladeGui.h"

#include "GladeSearchpath.h"

#include <config.h>
#include <i18n.h>
#include <XojMsgBox.h>

#include <stdlib.h>

GladeGui::GladeGui(GladeSearchpath* gladeSearchPath, string glade, string mainWnd)
{
	XOJ_INIT_TYPE(GladeGui);

	this->window = NULL;
	this->gladeSearchPath = gladeSearchPath;

	string filename = this->gladeSearchPath->findFile("", glade);

	GError* error = NULL;
	builder = gtk_builder_new();

	if (!gtk_builder_add_from_file(builder, filename.c_str(), &error))
	{
		string msg = FS(_F("Error loading glade file \"{1}\" (try to load \"{2}\")") % glade % filename);

		if (error != NULL)
		{
			msg += "\n";
			msg += error->message;
		}
		XojMsgBox::showErrorToUser(NULL, msg);

		g_error_free(error);

		exit(-1);
	}

	this->window = get(mainWnd);
}

GladeGui::~GladeGui()
{
	XOJ_CHECK_TYPE(GladeGui);

	g_object_unref(builder);

	XOJ_RELEASE_TYPE(GladeGui);
}

GtkWidget* GladeGui::get(string name)
{
	XOJ_CHECK_TYPE(GladeGui);

	GtkWidget* w = GTK_WIDGET(gtk_builder_get_object(builder, name.c_str()));
	if (w == NULL)
	{
		g_warning("GladeGui::get: Could not find glade Widget: \"%s\"", name.c_str());
	}
	return w;
}

GtkWidget* GladeGui::getWindow()
{
	XOJ_CHECK_TYPE(GladeGui);

	return this->window;
}

GladeSearchpath* GladeGui::getGladeSearchPath()
{
	XOJ_CHECK_TYPE(GladeGui);

	return this->gladeSearchPath;
}

GtkBuilder* GladeGui::getBuilder()
{
	XOJ_CHECK_TYPE(GladeGui);

	return this->builder;
}

GladeGui::operator GdkWindow* ()
{
	XOJ_CHECK_TYPE(GladeGui);

	return gtk_widget_get_window(GTK_WIDGET(getWindow()));
}

GladeGui::operator GtkWindow* ()
{
	XOJ_CHECK_TYPE(GladeGui);

	return GTK_WINDOW(getWindow());
}
