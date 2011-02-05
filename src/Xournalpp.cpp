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

#include "control/settings/ev-metadata-manager.h"

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

static bool optNoWarnSVN = false;
static gchar ** optFilename = NULL;

static GOptionEntry options[] = {
	{ "no-warn-svn",    'w', 0, G_OPTION_ARG_NONE,           &optNoWarnSVN, "Do not warn this is a development release", NULL },
	{ G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_FILENAME_ARRAY, &optFilename, "<input>" , NULL},
	{ NULL }
};

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

	GError *error = NULL;
	GOptionContext * context = context = g_option_context_new("[options] FILE");
	g_option_context_add_main_entries(context, options, GETTEXT_PACKAGE);
	g_option_context_add_group(context, gtk_get_option_group(true));
	if (!g_option_context_parse(context, &argc, &argv, &error)) {
		g_printerr("%s\n", error->message);
		g_error_free(error);
		gchar * help = g_option_context_get_help(context, true, NULL);
		g_print("%s", help);
		g_free(help);
		error = NULL;
	}
	g_option_context_free(context);

	if (!optNoWarnSVN) {
		GtkWidget * dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_YES_NO,
				_("You are using a development release of Xournal\n"
						"You can find the current release in CVS!\n"
						"DO NOT USE THIS RELEASE FOR PRODUCTIVE ENVIRONMENT!\n"
						"Are you sure you wish to start this release?\n\n\n"
						"If you don't want to show this warning, you can run\n"
						"\"xournal --help\"\n"
				));

		int result = gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);

		if (result == -9) {
			exit(-1);
		}
	}

	Control * control = new Control();

	MainWindow * win = new MainWindow(control);
	control->initWindow(win);

	bool opened = false;
	if (optFilename) {
		if (g_strv_length(optFilename) != 1) {
			GtkWidget * dialog = gtk_message_dialog_new((GtkWindow*) *win, GTK_DIALOG_DESTROY_WITH_PARENT,
					GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
					_("Sorry, Xournal can only open one file from the command line.\n"
							"Others are ignored."));
			gtk_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(dialog);
		}

		GFile * file = g_file_new_for_commandline_arg(optFilename[0]);
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
						if (t.size() > 1) {
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

	win->show();

	if (!opened) {
		control->newFile();
	}

	gtk_main();
	ev_metadata_manager_shutdown();

	control->saveSettings();

	delete win;
	delete control;

	return 0;
}
