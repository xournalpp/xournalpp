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

#include <boost/filesystem.hpp>
using boost::filesystem::path;

#include <gtk/gtk.h>
#include "StringUtils.h"

class Util
{
private:
	Util();
	virtual ~Util();

public:
	static GdkColor intToGdkColor(int c);
	static int gdkColorToInt(const GdkColor& c);

	static void cairo_set_source_rgbi(cairo_t* cr, int color);

	static path getAutosaveFilename();

	static int getPid();

	static void fakeExposeWidget(GtkWidget* widget, GdkPixmap* pixmap);
	static GdkPixbuf* newPixbufFromWidget(GtkWidget* widget, int iconSize = 24);
	static GtkWidget* newSepeartorImage();

	static void openFileWithDefaultApplicaion(path filename);
	static void openFileWithFilebrowser(path filename);
	
	static path getSettingsSubfolder(path subfolder);
	static path getSettingsFile(path relativeFileName);

};
