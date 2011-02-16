#include "GladeGui.h"
#include <config.h>
#include <stdlib.h>

#include <config.h>
#include <glib/gi18n-lib.h>

GList *GladeGui::directories = NULL;

GladeGui::GladeGui(const char * glade, const char * mainWnd) {
	this->window = NULL;
	gchar * filename = findFile(NULL, glade);
	this->xml = glade_xml_new(filename, NULL, NULL);
	if (!this->xml) {
		GtkWidget * dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR,
				GTK_BUTTONS_CLOSE, "Error loading glade file '%s' (try to load '%s')", glade, filename);
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
	if(w == NULL) {
		g_warning("Could not find glade Widget: \"%s\"", name);
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
	gchar *pathname = NULL;
	GdkPixbuf *pixbuf;
	GError *error = NULL;

	if (!filename || !filename[0])
		return NULL;

	pathname = findFile("pixmaps", filename);

	if (!pathname) {
		g_warning(_("Couldn't find pixmap file: %s"), filename);
		return NULL;
	}

	pixbuf = gdk_pixbuf_new_from_file(pathname, &error);
	if (!pixbuf) {
		fprintf(stderr, "Failed to load pixbuf file: %s: %s\n", pathname, error->message);
		g_error_free(error);
	}
	g_free(pathname);
	return pixbuf;
}

char * GladeGui::findFile(const char * subdir, const char * file) {
	GList *elem;

	char * filename = NULL;
	if (subdir == NULL) {
		filename = g_strdup(file);
	} else {
		filename = g_strdup_printf("%s%s%s", subdir, G_DIR_SEPARATOR_S, file);
	}

	// We step through each directory to find it.
	elem = directories;
	while (elem) {
		gchar *pathname = g_strdup_printf("%s%s%s", (gchar*) elem->data, G_DIR_SEPARATOR_S, filename);
		if (g_file_test(pathname, G_FILE_TEST_EXISTS)) {
			g_free(filename);
			return pathname;
		}
		g_free(pathname);
		elem = elem->next;
	}

	g_free(filename);
	return NULL;

}

/*
 * Use this function to set the directory containing installed pixmaps and Glade XML files.
 */
void GladeGui::addSearchDirectory(const char *directory) {
	GladeGui::directories = g_list_prepend(GladeGui::directories, g_strdup(directory));
}

GtkWidget * GladeGui::getWindow() {
	return this->window;
}
