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
#include "gettext.h"
#include "cfg.h"

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

char hexValue(char c) {
	if (c <= '9') {
		return c - '0';
	}
	if (c <= 'f') {
		return c - 'a';
	}
	if (c <= 'F') {
		return c - 'A';
	}
	return 0;
}

int main(int argc, char *argv[]) {
	installCrashHandlers();

//#ifdef ENABLE_NLS
//	const char * folder = bindtextdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
//	printf("bindtextdomain=%s\n", folder);
//	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
//	textdomain(GETTEXT_PACKAGE);
//#endif

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
		String sUri = uri;
		g_free(uri);
		g_object_unref(file);

		if (sUri.startsWith("file://")) {
			String path = "";
			StringTokenizer token(sUri.substring(7), '%', true);

			const char * tmp = token.next();
			bool special = false;
			while (tmp) {
				if (!strcmp("%", tmp)) {
					special = true;
				} else {
					if (special) {
						special = false;
						String t = tmp;
						if (t.length() > 1) {
							char x[2] = { 0 };
							x[0] = hexValue(tmp[0]) << 4 + hexValue(tmp[1]);
							path += x;
							path += t.substring(2);
						}
					} else {
						path += tmp;
					}
				}

				tmp = token.next();
			}
			opened = control->openFile(path);
		} else {
			GtkWidget * dialog = gtk_message_dialog_new((GtkWindow*) *win, GTK_DIALOG_DESTROY_WITH_PARENT,
					GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, _("Sorry, Xournal cannot open remote files at the moment.\n"
						"You have to copy the file to a local directory."));
			gtk_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(dialog);
		}
	}

	if (!opened) {
		control->newFile();
	}

	win->show();

	gtk_main();
	ev_metadata_manager_shutdown();

	control->saveSettings();

	delete win;
//	delete control;

	return 0;
}
