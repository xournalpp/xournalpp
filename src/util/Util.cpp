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
		GtkWidget* dlgError = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "%s",
													 FC(_F("File couldn't be opened. You have to do it manually:\n"
														   "URL: {1}") % filename));
		gtk_dialog_run(GTK_DIALOG(dlgError));
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
		GtkWidget* dlgError = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "%s",
													 FC(_F("File couldn't be opened. You have to do it manually:\n"
														   "URL: {1}") % filename));
		gtk_dialog_run(GTK_DIALOG(dlgError));
	}
}
