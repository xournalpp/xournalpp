#include "Util.h"

#include "config.h"
#include "config-dev.h"
#include "i18n.h"
#include "StringUtils.h"
#include "XojMsgBox.h"

#include <unistd.h>
#include <utility>

struct CallbackUiData
{
	explicit CallbackUiData(std::function<void()> callback)
	 : callback(std::move(callback))
	{
	}

	std::function<void()> callback;  //NOLINT
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
void Util::execInUiThread(std::function<void()>&& callback)
{
	gdk_threads_add_idle((GSourceFunc) execInUiThreadCallback, new CallbackUiData(std::move(callback)));
}

void Util::cairo_set_source_rgbi(cairo_t* cr, int color)
{
	auto tmp = static_cast<unsigned>(color);

	double r = ((tmp >> 16u) & 0xffu) / 255.0;
	double g = ((tmp >> 8u) & 0xffu) / 255.0;
	double b = (tmp & 0xffu) / 255.0;

	cairo_set_source_rgb(cr, r, g, b);
}


void Util::apply_rgb_togdkrgba(GdkRGBA& col, int color)
{
	auto tmp = static_cast<unsigned>(color);
	col.red = ((tmp >> 16u) & 0xFFu) / 255.0;
	col.green = ((tmp >> 8u) & 0xFFu) / 255.0;
	col.blue = (tmp & 0xFFu) / 255.0;
	col.alpha = 1.0;
}

int Util::gdkrgba_to_hex(GdkRGBA& color)
{
	return ((static_cast<unsigned>(color.red * 255)) & 0xffu) << 16u |
	       ((static_cast<unsigned>(color.green * 255)) & 0xffu) << 8u |
	       ((static_cast<unsigned>(color.blue * 255)) & 0xffu);
}

pid_t Util::getPid()
{
	return ::getpid();
}

Path Util::getAutosaveFilename()
{
	Path p(getConfigSubfolder("autosave"));
	p /= std::to_string(getPid()) + ".xopp";
	return p;
}

Path Util::getConfigSubfolder(const Path& subfolder)
{
	Path p(g_get_home_dir());
	p /= CONFIG_DIR;
	p /= subfolder;

	return Util::ensureFolderExists(p);
}

Path Util::getConfigFile(const Path& relativeFileName)
{
	Path p = getConfigSubfolder(relativeFileName.getParentPath());
	p /= relativeFileName.getFilename();
	return p;
}

Path Util::getTmpDirSubfolder(const Path& subfolder)
{
	Path p(g_get_tmp_dir());
	p /= FS(_F("xournalpp-{1}") % Util::getPid());
	p /= subfolder;
	return Util::ensureFolderExists(p);
}

Path Util::ensureFolderExists(const Path& p)
{
	if (g_mkdir_with_parents(p.c_str(), 0700) == -1)
	{
		Util::execInUiThread([=]() {
			string msg = FS(_F("Could not create folder: {1}") % p.str());
			g_warning("%s", msg.c_str());
			XojMsgBox::showErrorToUser(nullptr, msg);
		});
	}
	return p;
}

void Util::openFileWithDefaultApplicaion(Path filename)
{
#ifdef __APPLE__
	constexpr auto const OPEN_PATTERN = "open \"{1}\"";
#elif _WIN32 // note the underscore: without it, it's not msdn official!
	constexpr auto const OPEN_PATTERN = "start \"{1}\"";
#else // linux, unix, ...
	constexpr auto const OPEN_PATTERN = "xdg-open \"{1}\"";
#endif

	string command = FS(FORMAT_STR(OPEN_PATTERN) % filename.getEscapedPath());
	if (system(command.c_str()) != 0)
	{
		string msg = FS(_F("File couldn't be opened. You have to do it manually:\n" "URL: {1}") % filename.str());
		XojMsgBox::showErrorToUser(NULL, msg);
	}
}

void Util::openFileWithFilebrowser(Path filename)
{
#ifdef __APPLE__
	constexpr auto const OPEN_PATTERN = "open \"{1}\"";
#elif WIN32
	constexpr auto const OPEN_PATTERN = "explorer.exe /n,/e,\"{1}\"";
#else // linux, unix, ...
	constexpr auto const OPEN_PATTERN = R"(nautilus "file://{1}" || dolphin "file://{1}" || konqueror "file://{1}" &)";
#endif
	string command = FS(FORMAT_STR(OPEN_PATTERN) % filename.getEscapedPath());
	if (system(command.c_str()) != 0)
	{
		string msg = FS(_F("File couldn't be opened. You have to do it manually:\n" "URL: {1}") % filename.str());
		XojMsgBox::showErrorToUser(NULL, msg);
	}
}

gboolean Util::paintBackgroundWhite(GtkWidget* widget, cairo_t* cr, void*)
{
	GtkAllocation alloc;
	gtk_widget_get_allocation(widget, &alloc);
	cairo_set_source_rgb(cr, 1, 1, 1);
	cairo_rectangle(cr, 0, 0, alloc.width, alloc.height);
	cairo_fill(cr);
	return false;
}

