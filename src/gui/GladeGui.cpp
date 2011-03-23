#include "GladeGui.h"
#include <config.h>
#include <stdlib.h>
// TODO: AA: type check

#include <config.h>
#include <glib/gi18n-lib.h>

#include "GladeSearchpath.h"

GladeGui::GladeGui(GladeSearchpath * gladeSearchPath, const char * glade, const char * mainWnd) {
	this->window = NULL;
	this->gladeSearchPath = gladeSearchPath;

	char * filename = this->gladeSearchPath->findFile(NULL, glade);
	this->xml = glade_xml_new(filename, NULL, NULL);
	if (!this->xml) {
		GtkWidget * dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
				_("Error loading glade file '%s' (try to load '%s')"), glade, filename);
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(GTK_WIDGET(dialog));
		exit(-1);
	}

	this->window = get(mainWnd);

	g_free(filename);
}

GladeGui::~GladeGui() {
	gtk_widget_destroy(this->window);
	g_object_unref(this->xml);
}

GtkWidget * GladeGui::get(const char * name) {
	GtkWidget * w = glade_xml_get_widget(xml, name);
	if (w == NULL) {
		g_warning("GladeGui::get: Could not find glade Widget: \"%s\"", name);
	}
	return w;
}

GtkWidget * GladeGui::loadIcon(const char * filename) {
	GdkPixbuf * icon = loadIconPixbuf(filename);
	if (icon == NULL) {
		return gtk_image_new();
	}

	return gtk_image_new_from_pixbuf(icon);
}

GdkPixbuf * GladeGui::loadIconPixbuf(const char * filename) {
	if (!filename || !filename[0]) {
		return NULL;
	}

	char * pathname = this->gladeSearchPath->findFile("pixmaps", filename);

	if (!pathname) {
		g_warning("GladeGui::get: Couldn't find pixmap file: %s", filename);
		return NULL;
	}

	GError * error = NULL;
	GdkPixbuf * pixbuf = gdk_pixbuf_new_from_file(pathname, &error);
	if (!pixbuf) {
		g_error("Failed to load pixbuf file: %s: %s\n", pathname, error->message);
		g_error_free(error);
	}
	g_free(pathname);
	return pixbuf;
}

GtkWidget * GladeGui::getWindow() {
	return this->window;
}

GladeGui::operator GdkWindow *() {
	return GTK_WIDGET(getWindow())->window;
}

GladeGui::operator GtkWindow *() {
	return GTK_WINDOW(getWindow());
}

