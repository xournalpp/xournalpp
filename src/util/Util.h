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

#include <cstdint>
#include <functional>
#include <limits>
#include <string>

#include <gtk/gtk.h>

#include "OutputStream.h"
#include "filesystem.h"

using std::string;

namespace Util {

void cairo_set_source_rgbi(cairo_t* cr, int color);

GdkRGBA rgb_to_GdkRGBA(uint32_t color);
GdkRGBA argb_to_GdkRGBA(uint32_t color);
uint32_t gdkrgba_to_hex(const GdkRGBA& color);

fs::path getAutosaveFilename();

pid_t getPid();

void openFileWithDefaultApplicaion(const fs::path& filename);
void openFileWithFilebrowser(const fs::path& filename);

/**
 * Return the configuration folder path (may not be guaranteed to exist).
 */
fs::path getConfigFolder();
fs::path getConfigSubfolder(const fs::path& subfolder = "");
fs::path getCacheSubfolder(const fs::path& subfolder = "");
fs::path getDataSubfolder(const fs::path& subfolder = "");

fs::path getConfigFile(const fs::path& relativeFileName = "");
fs::path getCacheFile(const fs::path& relativeFileName = "");

fs::path getTmpDirSubfolder(const fs::path& subfolder = "");

fs::path ensureFolderExists(const fs::path& p);

/**
 * Wrap the system call to redirect errors to a dialog
 */
void systemWithMessage(const char* command);

/**
 * Execute the callback in the UI Thread.
 *
 * Make sure the container class is not deleted before the UI stuff is finished!
 */
void execInUiThread(std::function<void()>&& callback);

gboolean paintBackgroundWhite(GtkWidget* widget, cairo_t* cr, void* unused);

/**
 * Format coordinates to use 8 digits of precision https://m.xkcd.com/2170/
 * This function directy writes to the given OutputStream.
 */
extern void writeCoordinateString(OutputStream* out, double xVal, double yVal);

constexpr const gchar* PRECISION_FORMAT_STRING = "%.8f";

constexpr const auto DPI_NORMALIZATION_FACTOR = 72.0;

}  // namespace Util

constexpr auto npos = std::numeric_limits<size_t>::max();
