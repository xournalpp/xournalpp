/*
 * Xournal++
 *
 * Xournal util functions
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "Path.h"

#include <gtk/gtk.h>

#include <string>
#include <functional>
#include <limits>
#include <cstdint>

using std::string;

namespace Util
{

void cairo_set_source_rgbi(cairo_t* cr, int color);

GdkRGBA rgb_to_GdkRGBA(uint32_t color);
uint32_t gdkrgba_to_hex(const GdkRGBA& color);

Path getAutosaveFilename();

pid_t getPid();

void openFileWithDefaultApplicaion(const Path& filename);
void openFileWithFilebrowser(const Path& filename);

Path getConfigSubfolder(const Path& subfolder = "");
Path getConfigFile(const Path& relativeFileName = "");

Path getTmpDirSubfolder(const Path& subfolder = "");

Path ensureFolderExists(const Path& p);

/**
	 * Execute the callback in the UI Thread.
	 *
	 * Make sure the container class is not deleted before the UI stuff is finished!
	 */
void execInUiThread(std::function<void()>&& callback);

gboolean paintBackgroundWhite(GtkWidget* widget, cairo_t* cr, void* unused);

/**
 * Format coordinates to use 8 digits of precision https://m.xkcd.com/2170/
 * This function works like g_ascii_formatd in that it stores the result in buff, and
 * also returns the result.
 */
extern gchar* getCoordinateString(gchar* buff, gulong buffLen, double xVal, double yVal);

constexpr const gchar* PRECISION_FORMAT_STRING = "%.8f";

constexpr const gchar* PRECISION_FORMAT_STRING_XY = "%.8f %.8f";  // note the space delimiter

constexpr const int PRECISION_FORMAT_BUFF_LEN = G_ASCII_DTOSTR_BUF_SIZE * 2 + 1;

}  // namespace Util

static const size_t npos = std::numeric_limits<size_t>::max();