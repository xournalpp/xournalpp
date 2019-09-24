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

GdkRGBA Util::rgb_to_GdkRGBA(const uint32_t color)
{  // clang-format off
	return {((color >> 16U) & 0xFFU) / 255.0,
	        ((color >> 8U) & 0xFFU) / 255.0,
	        (color & 0xFFU) / 255.0,
	        1.0};
	// clang-format on
}

void Util::cairo_set_source_rgbi(cairo_t* cr, int color)
{
	auto rgba = rgb_to_GdkRGBA(color);
	cairo_set_source_rgb(cr, rgba.red, rgba.green, rgba.blue);
}

// Splits the double into a equal sized distribution between [0,256[ and rounding down
// inspired by, which isn't completely correct:
// https://stackoverflow.com/questions/1914115/converting-color-value-from-float-0-1-to-byte-0-255
constexpr double MAXCOLOR = 256.0 - std::numeric_limits<double>::epsilon() * 128;

inline uint32_t float_to_int_color(const double color)
{
	static_assert(MAXCOLOR < 256.0, "MAXCOLOR isn't smaler than 256");
	return static_cast<uint32_t>(color * MAXCOLOR);
}

uint32_t Util::gdkrgba_to_hex(const GdkRGBA& color)
{   // clang-format off
	return float_to_int_color(color.alpha) << 24U |
	       float_to_int_color(color.red)  << 16U |
	       float_to_int_color(color.green) << 8U |
	       float_to_int_color(color.blue);
	// clang-format on
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

void Util::openFileWithDefaultApplicaion(const Path& filename)
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
		XojMsgBox::showErrorToUser(nullptr, msg);
	}
}

void Util::openFileWithFilebrowser(const Path& filename)
{
#ifdef __APPLE__
	constexpr auto const OPEN_PATTERN = "open \"{1}\"";
#elif _WIN32
	constexpr auto const OPEN_PATTERN = "explorer.exe /n,/e,\"{1}\"";
#else // linux, unix, ...
	constexpr auto const OPEN_PATTERN = R"(nautilus "file://{1}" || dolphin "file://{1}" || konqueror "file://{1}" &)";
#endif
	string command = FS(FORMAT_STR(OPEN_PATTERN) % filename.getEscapedPath());
	if (system(command.c_str()) != 0)
	{
		string msg = FS(_F("File couldn't be opened. You have to do it manually:\n" "URL: {1}") % filename.str());
		XojMsgBox::showErrorToUser(nullptr, msg);
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

void Util::writeCoordinateString(OutputStream* out, double xVal, double yVal)
{
	char coordString[G_ASCII_DTOSTR_BUF_SIZE];
	g_ascii_formatd(coordString, G_ASCII_DTOSTR_BUF_SIZE, Util::PRECISION_FORMAT_STRING, xVal);
	out->write(coordString);
	out->write(" ");
	g_ascii_formatd(coordString, G_ASCII_DTOSTR_BUF_SIZE, Util::PRECISION_FORMAT_STRING, yVal);
	out->write(coordString);
}
