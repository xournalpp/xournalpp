#include "Util.h"
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include "../cfg.h"

GdkColor Util::intToGdkColor(int c) {
	GdkColor color = { 0, 0, 0, 0 };
	color.red = (c >> 8) & 0xff00;
	color.green = (c >> 0) & 0xff00;
	color.blue = (c << 8) & 0xff00;
	return color;
}

int Util::gdkColorToInt(const GdkColor & c) {
	return (c.red >> 8) << 16 | (c.green >> 8) << 8 | (c.blue >> 8);
}

void Util::cairo_set_source_rgbi(cairo_t *cr, int c) {
	double r = ((c >> 16) & 0xff) / 255.0;
	double g = ((c >> 8) & 0xff) / 255.0;
	double b = (c & 0xff) / 255.0;

	cairo_set_source_rgb(cr, r, g, b);
}

String Util::getAutosaveFilename() {
	String path = getSettingsSubfolder("autosave");

	pid_t pid = getpid();
	path += (int) pid;
	path += ".xoj";

	return path;
}

String Util::getSettingsSubfolder(String subfolder) {
	gchar * folder = g_strconcat(g_get_home_dir(), G_DIR_SEPARATOR_S, CONFIG_DIR, G_DIR_SEPARATOR_S, subfolder.c_str(),
			G_DIR_SEPARATOR_S, NULL);
	String path(folder, true);

	if (!g_file_test(path.c_str(), G_FILE_TEST_EXISTS)) {
		mkdir(path.c_str(), 0700);
	}

	return path;
}

MemoryCheckObject::MemoryCheckObject() {
	this->d1 = 0xffff0000;
	this->d2 = 465456;
	this->d3 = 89535395;
}

bool MemoryCheckObject::isMemoryCorrupted() {
	return (this->d1 != 0xffff0000 || this->d2 != 465456 || this->d3 != 89535395);
}

