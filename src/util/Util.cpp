#include "Util.h"

#include <config.h>
#include <config-dev.h>
#include <i18n.h>

#include <boost/filesystem.hpp>

#include <sys/types.h>
#include <unistd.h>

#include <iostream>
using std::cout;
using std::endl;

class CallbackUiData {
public:
	CallbackUiData(std::function<void()> callback)
	 : callback(callback)
	{
	}
	std::function<void()> callback;
};

/**
 * This method is called in the GTK UI Thread
 */
static bool execInUiThreadCallback(CallbackUiData* cb)
{
	cb->callback();

	delete cb;

	// Do not call again
	return false;
}

/**
 * Execute the callback in the UI Thread.
 *
 * Make sure the container class is not deleted before the UI stuff is finished!
 */
void Util::execInUiThread(std::function<void()> callback)
{
	gdk_threads_add_idle((GSourceFunc) execInUiThreadCallback, new CallbackUiData(callback));
}

void Util::showErrorToUser(GtkWindow* win, string msg)
{
	GtkWidget* dialog = gtk_message_dialog_new(win, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
											   "%s", msg.c_str());
	if (win != NULL)
	{
		gtk_window_set_transient_for(GTK_WINDOW(dialog), win);
	}
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
}

void Util::cairo_set_source_rgbi(cairo_t* cr, int c)
{
	double r = ((c >> 16) & 0xff) / 255.0;
	double g = ((c >> 8) & 0xff) / 255.0;
	double b = (c & 0xff) / 255.0;

	cairo_set_source_rgb(cr, r, g, b);
}


void Util::apply_rgb_togdkrgba(GdkRGBA& col, int color)
{
	col.red = ((color >> 16) & 0xFF) / 255.0;
	col.green = ((color >> 8) & 0xFF) / 255.0;
	col.blue = (color & 0xFF) / 255.0;
	col.alpha = 1.0;
}

int Util::gdkrgba_to_hex(GdkRGBA& color)
{
	return (((int)(color.red * 255)) & 0xff) << 16 |
			(((int)(color.green * 255)) & 0xff) << 8 |
			(((int)(color.blue * 255)) & 0xff);
}

int Util::getPid()
{
	pid_t pid = ::getpid();
	return (int) pid;
}

path Util::getAutosaveFilename()
{
	path p(getConfigSubfolder("autosave"));
	p /= std::to_string(getPid()) + ".xoj";
	return p;
}

path Util::getConfigSubfolder(path subfolder)
{
	using namespace boost::filesystem;
	path p(g_get_home_dir());
	p /= CONFIG_DIR;
	p /= subfolder;

	if (!exists(p))
	{
		create_directory(p);
		permissions(p, owner_all);
	}

	return p;
}

path Util::getConfigFile(path relativeFileName)
{
	path p = getConfigSubfolder(relativeFileName.parent_path());
	p /= relativeFileName.filename();
	return p;
}

void Util::openFileWithDefaultApplicaion(path filename)
{
#ifdef __APPLE__
#define OPEN_PATTERN "open \"{1}\""
#elif _WIN32 // note the underscore: without it, it's not msdn official!
#define OPEN_PATTERN "start \"{1}\""
#else // linux, unix, ...
#define OPEN_PATTERN "xdg-open \"{1}\""
#endif

	string escaped = filename.string();
	StringUtils::replace_all_chars(escaped, {
		replace_pair('\\', "\\\\"),
		replace_pair('\"', "\\\"")
	});

	string command = FS(bl::format(OPEN_PATTERN) % escaped);
	cout << bl::format("XPP Execute command: «{1}»") % command << endl;
	if (system(command.c_str()) != 0)
	{
		string msg = FS(_F("File couldn't be opened. You have to do it manually:\n" "URL: {1}") % filename);
		showErrorToUser(NULL, msg);
	}
}

void Util::openFileWithFilebrowser(path filename)
{
#undef OPEN_PATTERN

#ifdef __APPLE__
#define OPEN_PATTERN "open \"{1}\""
#elif _WIN32 // note the underscore: without it, it's not msdn official!
#define OPEN_PATTERN "explorer.exe /n,/e,\"{1}\""
#else // linux, unix, ...
#define OPEN_PATTERN "nautilus \"file://{1}\" || dolphin \"file://{1}\" || konqueror \"file://{1}\" &"
#endif

	string escaped = filename.string();
	StringUtils::replace_all_chars(escaped, {
		replace_pair('\\', "\\\\"),
		replace_pair('\"', "\\\"")
	});

	string command = FS(bl::format(OPEN_PATTERN) % escaped);
	cout << bl::format("XPP show file in filebrowser command: «{1}»") % command << endl;
	if (system(command.c_str()) != 0)
	{
		string msg = FS(_F("File couldn't be opened. You have to do it manually:\n" "URL: {1}") % filename);
		showErrorToUser(NULL, msg);
	}
}

gboolean Util::paintBackgroundWhite(GtkWidget* widget, cairo_t* cr, void* unused)
{
	GtkAllocation alloc;
	gtk_widget_get_allocation(widget, &alloc);
	cairo_set_source_rgb(cr, 1, 1, 1);
	cairo_rectangle(cr, 0, 0, alloc.width, alloc.height);
	cairo_fill(cr);
	return false;
}

