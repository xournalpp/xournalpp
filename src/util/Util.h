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

#include "StringUtils.h"

#include <boost/filesystem/path.hpp>
using boost::filesystem::path;

#include <gtk/gtk.h>

class Util
{
private:
	Util();
	virtual ~Util();

public:
	static GdkColor intToGdkColor(int c);
	static int gdkColorToInt(const GdkColor& c);

	static void cairo_set_source_rgbi(cairo_t* cr, int color);

	static void apply_rgb_togdkrgba(GdkRGBA& col, int color);
	static int gdkrgba_to_hex(GdkRGBA& color);

	static path getAutosaveFilename();

	static int getPid();

	static void openFileWithDefaultApplicaion(path filename);
	static void openFileWithFilebrowser(path filename);
	
	static path getConfigSubfolder(path subfolder = "");
	static path getConfigFile(path relativeFileName = "");

};

static const size_t size_t_npos = static_cast<size_t>(-1);
// for 64b systems it's 18446744073709551615 and for 32b – 4294967295
