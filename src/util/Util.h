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

using std::string;

namespace Util
{

void cairo_set_source_rgbi(cairo_t* cr, int color);

void apply_rgb_togdkrgba(GdkRGBA& col, int color);
int gdkrgba_to_hex(GdkRGBA& color);

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

}  // namespace Util

static const size_t npos = std::numeric_limits<size_t>::max();
