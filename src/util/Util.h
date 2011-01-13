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

#include <gtk/gtk.h>
#include <assert.h>
#include "../model/String.h"

class Util {
public:
	static GdkColor intToGdkColor(int c);
	static int gdkColorToInt(const GdkColor & c);

	static void cairo_set_source_rgbi(cairo_t *cr, int color);

	static String getAutosaveFilename();

private:
	static String getSettingsSubfolder(String subfolder);
};

#define CHECK_MEMORY(obj) {	bool corrupted = obj->isMemoryCorrupted(); \
	if(corrupted) { \
		fprintf(stderr, "%s:%i\tMemory corrupted!\n", __FILE__, __LINE__); \
	} \
	assert(!corrupted); }


/**
 * Used for testing memory violations
 */

class MemoryCheckObject {
public:
	MemoryCheckObject();

	bool isMemoryCorrupted();
private:
	int d1;
	int d2;
	int d3;
};

#endif /* __UTIL_H__ */
