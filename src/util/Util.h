/*
 * Xournal++
 *
 * Xournal util functions
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __UTIL_H__
#define __UTIL_H__

#include <glib.h>
#include "../util/String.h"

class Util {
public:
	static GdkColor intToGdkColor(int c);
	static int gdkColorToInt(const GdkColor & c);

	static void cairo_set_source_rgbi(cairo_t *cr, int color);

	static String getAutosaveFilename();

	static int getPid();

private:
	static String getSettingsSubfolder(String subfolder);
};

#endif /* __UTIL_H__ */
