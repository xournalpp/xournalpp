/*
 * Xournal++
 *
 * The main application
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#include <iostream>
#include <sys/stat.h>

#include <glib.h>
#include <stdlib.h>

#include "gui/MainWindow.h"
#include "util/CrashHandler.h"
#include "control/Control.h"
#include <config.h>
#include "cfg.h"
#ifdef ENABLE_NLS
#include <libintl.h>
#endif

#include "control/ev-metadata-manager.h"

// TODO: use gettext:
// http://www.gnu.org/software/hello/manual/automake/gettext.html
// http://oriya.sarovar.org/docs/gettext_single.html

void initPath(const char * argv0) {
	gchar * path;
	gchar * searchPath;
	char buffer[512] = { 0 };

	// Create config directory if not exists
	gchar * file = g_build_filename(g_get_home_dir(), G_DIR_SEPARATOR_S, CONFIG_DIR, NULL);
	mkdir(file, 0700);
	g_free(file);

	// Add first home dir to search path, to add custom glade XMLs
	searchPath = g_build_filename(g_get_home_dir(), G_DIR_SEPARATOR_S, CONFIG_DIR, "ui", NULL);

	if (g_file_test(searchPath, G_FILE_TEST_EXISTS) && g_file_test(searchPath, G_FILE_TEST_IS_DIR)) {
		GladeGui::addSearchDirectory(searchPath);
	}
	g_free(searchPath);

	searchPath = g_build_filename(PACKAGE_DATA_DIR, PACKAGE, "ui", NULL);
	GladeGui::addSearchDirectory(searchPath);
	g_free(searchPath);

	path = g_path_get_dirname(argv0);
	searchPath = g_build_filename(path, "ui", NULL);
	GladeGui::addSearchDirectory(searchPath);
	g_free(searchPath);

	searchPath = g_build_filename(path, "..", "ui", NULL);
	GladeGui::addSearchDirectory(searchPath);
	g_free(searchPath);
	g_free(path);

	path = getcwd(buffer, sizeof(buffer));
	if (path == NULL) {
		return;
	}

	searchPath = g_build_filename(path, "ui", NULL);
	GladeGui::addSearchDirectory(searchPath);
	g_free(searchPath);

	searchPath = g_build_filename(path, "..", "ui", NULL);
	GladeGui::addSearchDirectory(searchPath);
	g_free(searchPath);
}

int main(int argc, char *argv[]) {
	installCrashHandlers();

#ifdef ENABLE_NLS
	const char * folder = bindtextdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
	printf("bindtextdomain=%s\n", folder);
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
	textdomain(GETTEXT_PACKAGE);
#endif

	gtk_init(&argc, &argv);

	g_thread_init(NULL);

	initPath(argv[0]);

	Control * control = new Control();

	MainWindow * win = new MainWindow(control);
	control->initWindow(win);

	bool opened = false;
	if (argc > 1) {
		GFile * file = g_file_new_for_commandline_arg(argv[1]);
		char * uri = g_file_get_uri(file);
		opened = control->openFile(uri);
		g_free(uri);
		g_object_unref(file);
	}

	if (!opened) {
		control->newFile();
	}

	win->show();

	gtk_main();
	ev_metadata_manager_shutdown();

	control->saveSettings();

	delete win;
	delete control;

	return 0;
}
