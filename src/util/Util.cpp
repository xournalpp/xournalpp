#include <Util.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include "../cfg.h"
#include <stdlib.h>

#include <config.h>
#include <glib/gi18n-lib.h>


GdkColor Util::intToGdkColor(int c)
{
	GdkColor color = { 0, 0, 0, 0 };
	color.red = (c >> 8) & 0xff00;
	color.green = (c >> 0) & 0xff00;
	color.blue = (c << 8) & 0xff00;
	return color;
}

int Util::gdkColorToInt(const GdkColor& c)
{
	return (c.red >> 8) << 16 | (c.green >> 8) << 8 | (c.blue >> 8);
}

void Util::cairo_set_source_rgbi(cairo_t* cr, int c)
{
	double r = ((c >> 16) & 0xff) / 255.0;
	double g = ((c >> 8) & 0xff) / 255.0;
	double b = (c & 0xff) / 255.0;

	cairo_set_source_rgb(cr, r, g, b);
}

int Util::getPid()
{
	pid_t pid = getpid();
	return (int) pid;
}

String Util::getAutosaveFilename()
{
	String path = getSettingsSubfolder("autosave");

	path += getPid();
	path += ".xoj";

	return path;
}

String Util::getSettingsSubfolder(String subfolder)
{
	String path = String::format("%s%c%s%c%s%c", g_get_home_dir(), G_DIR_SEPARATOR,
	                             CONFIG_DIR, G_DIR_SEPARATOR, subfolder.c_str(), G_DIR_SEPARATOR);

	if (!g_file_test(path.c_str(), G_FILE_TEST_EXISTS))
	{
		mkdir(path.c_str(), 0700);
	}

	return path;
}

#if 0

GtkWidget* Util::newSepeartorImage()
{
	GtkWidget* separator = gtk_vseparator_new();
	GdkPixbuf* pixbuf = Util::newPixbufFromWidget(separator);
	gtk_widget_unref(separator);
	GtkWidget * w = gtk_image_new_from_pixbuf(pixbuf);
	g_object_unref(pixbuf);
	return w;
}

void Util::fakeExposeWidget(GtkWidget* widget, GdkPixmap* pixmap)
{
	GdkWindow* tmp_window;
	GdkEventExpose event;

	event.type = GDK_EXPOSE;
	event.window = pixmap;
	event.send_event = FALSE;
	gtk_widget_get_allocation(widget, &event.area);
	event.region = NULL;
	event.count = 0;

	tmp_window = gtk_widget_get_window(widget);
	gtk_widget_set_window(widget, pixmap);
	//widget->window = pixmap;
	gtk_widget_send_expose(widget, (GdkEvent*) &event);
	gtk_widget_set_window(widget, tmp_window);
}

#endif

GdkPixbuf* Util::newPixbufFromWidget(GtkWidget* widget, int iconSize)
{
	GtkAllocation allocation;

	gtk_widget_get_allocation(widget, &allocation);

	cairo_surface_t* surf = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
	                                                   allocation.width,
	                                                   allocation.height);

	cairo_t* cr = cairo_create(surf);

	//double m = MAX(allocation.height, allocation.width);

	//cairo_scale(cr, m / iconSize, m / iconSize);

	gtk_widget_draw(widget, cr);

	cairo_destroy(cr);

	GdkPixbuf* buf = gdk_pixbuf_get_from_surface(surf, 0, 0,
	                                             iconSize, iconSize);

	cairo_surface_destroy(surf);

	return buf;
}

void Util::openFileWithDefaultApplicaion(const char* filename)
{
#ifdef __APPLE__
#define OPEN_PATTERN "open \"%s\""
#elif _WIN32 // note the underscore: without it, it's not msdn official!
#define OPEN_PATTERN "start \"%s\""
#else // linux, unix, ...
#define OPEN_PATTERN "xdg-open \"%s\""
#endif

	String escaped = String(filename).replace("\\", "\\\\").replace("\"", "\\\"");

	char* command = g_strdup_printf(OPEN_PATTERN, escaped.c_str());
	printf("XPP Execute command: «%s»\n", command);
	if(system(command) != 0)
	{
		GtkWidget* dlgError = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL,
		                                             GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "%s",
		                                             _("File could not be opened. You have to open it manual\n:URL: %s"), filename,
		                                             filename);
		gtk_dialog_run(GTK_DIALOG(dlgError));
	}
	g_free(command);
}

void Util::openFileWithFilebrowser(const char* filename)
{
#undef OPEN_PATTERN

#ifdef __APPLE__
#define OPEN_PATTERN "open \"%s\""
#elif _WIN32 // note the underscore: without it, it's not msdn official!
#define OPEN_PATTERN "explorer.exe /n,/e,\"%s\""
#else // linux, unix, ...
#define OPEN_PATTERN "nautilus \"file://%s\" || konqueror \"file://%s\""
#endif

	String escaped = String(filename).replace("\\", "\\\\").replace("\"", "\\\"");

	char* command = g_strdup_printf(OPEN_PATTERN, escaped.c_str(),
	                                escaped.c_str()); // twice for linux...
	printf("XPP show file in filebrowser command: «%s»\n", command);
	if(system(command) != 0)
	{
		GtkWidget* dlgError = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL,
		                                             GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "%s",
		                                             _("File could not be opened. You have to open it manual\n:URL: %s"), filename,
		                                             filename);
		gtk_dialog_run(GTK_DIALOG(dlgError));
	}
	g_free(command);
}

