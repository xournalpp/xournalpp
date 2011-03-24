#include "XournalMain.h"
#include <glib/gi18n-lib.h>
#include <gtk/gtk.h>
#include <iostream>

#include "gui/MainWindow.h"
#include "Control.h"
#include "LoadHandler.h"
#include "../gui/GladeSearchpath.h"
#include "../gui/XournalView.h"
#include "../pdf/PdfExport.h"
#include "cfg.h"

XournalMain::XournalMain() {
	XOJ_INIT_TYPE(XournalMain);
}

XournalMain::~XournalMain() {
	XOJ_RELEASE_TYPE(XournalMain);
}

#ifdef ENABLE_NLS
void XournalMain::initLocalisation() {
	XOJ_CHECK_TYPE(XournalMain);

	setlocale(LC_ALL, "");
	bindtextdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
	textdomain(GETTEXT_PACKAGE);
}
#endif

int XournalMain::exportPdf(const char * input, const char * output) {
	XOJ_CHECK_TYPE(XournalMain);

	LoadHandler loader;

	Document * doc = loader.loadDocument(input);
	if (doc == NULL) {
		String err = loader.getLastError();
		printf("%s\n", err.c_str());
		return -2;
	}

	GFile * file = g_file_new_for_commandline_arg(output);

	PdfExport pdf(doc, NULL);
	if (!pdf.createPdf(g_file_get_uri(file))) {
		String err = pdf.getLastError();
		printf("%s\n", err.c_str());

		g_object_unref(file);
		return -3;
	}

	g_object_unref(file);

	printf("%s\n", _("PDF File successfully created"));

	return 0; // no error
}

int XournalMain::run(int argc, char * argv[]) {
	XOJ_CHECK_TYPE(XournalMain);

#ifdef ENABLE_NLS
	this->initLocalisation();
#endif

	GError * error = NULL;
	GOptionContext * context = context = g_option_context_new("FILE");

	bool optNoWarnSVN = false;
	gchar ** optFilename = NULL;
	gchar * pdfFilename = NULL;
	int openAtPageNumber = -1;

	GOptionEntry options[] = { { "no-warn-svn", 'w', 0, G_OPTION_ARG_NONE, &optNoWarnSVN, "Do not warn this is a development release", NULL }, { "create-pdf",
			'p', 0, G_OPTION_ARG_FILENAME, &pdfFilename, "PDF output filename", NULL }, { "page", 'n', 0, G_OPTION_ARG_INT, &openAtPageNumber,
			"Jump to Page (first Page: 1)", "N" }, { G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_FILENAME_ARRAY, &optFilename, "<input>", NULL }, { NULL } };

	g_option_context_add_main_entries(context, options, GETTEXT_PACKAGE);
	// parse options, so we don't need gtk_init, but don't init display (so we have a commandline mode)
	g_option_context_add_group(context, gtk_get_option_group(false));
	if (!g_option_context_parse(context, &argc, &argv, &error)) {
		g_printerr("%s\n", error->message);
		g_error_free(error);
		gchar * help = g_option_context_get_help(context, true, NULL);
		g_print("%s", help);
		g_free(help);
		error = NULL;
	}
	g_option_context_free(context);

	// Init threads (used for our Sheduler, Jobs)
	g_thread_init(NULL);

	if (pdfFilename && optFilename && *optFilename) {
		return exportPdf(*optFilename, pdfFilename);
	}

	// Init GTK Display
	gdk_display_open_default_libgtk_only();

	GladeSearchpath * gladePath = initPath(argv[0]);

	if (!optNoWarnSVN) {
		GtkWidget * dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_YES_NO,
				_("You are using a development release of Xournal\n"
						"You can find the current release in CVS!\n"
						"DO NOT USE THIS RELEASE FOR PRODUCTIVE ENVIRONMENT!\n"
						"Are you sure you wish to start this release?\n\n\n"
						"If you don't want to show this warning, you can run\n"
						"\"xournalpp --help\"\n"
				));

		int result = gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);

		if (result == -9) {
			exit(-1);
		}
	}

	Control * control = new Control(gladePath);

	MainWindow * win = new MainWindow(gladePath, control);
	control->initWindow(win);

	bool opened = false;
	if (optFilename) {
		if (g_strv_length(optFilename) != 1) {
			GtkWidget * dialog = gtk_message_dialog_new((GtkWindow*) *win, GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
					_("Sorry, Xournal can only open one file from the command line.\n"
							"Others are ignored."));
			gtk_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(dialog);
		}

		GFile * file = g_file_new_for_commandline_arg(optFilename[0]);
		char * filename = g_file_get_path(file);
		char * uri = g_file_get_uri(file);
		String sUri = uri;
		g_free(uri);
		g_object_unref(file);

		if (sUri.startsWith("file://")) {
			opened = control->openFile(filename, openAtPageNumber);
			g_free(filename);
		} else {
			GtkWidget * dialog = gtk_message_dialog_new((GtkWindow*) *win, GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
					_("Sorry, Xournal cannot open remote files at the moment.\n"
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

	control->saveSettings();

	win->getXournal()->clearSelection();

	delete win;
	delete control;
	delete gladePath;

#ifdef XOJ_MEMORY_LEAK_CHECK_ENABLED
	xoj_momoryleak_printRemainingObjects();
#endif

	return 0;
}

/**
 * Path for glade files and Pixmaps, first searches in the home folder, so you can customize glade files
 */
GladeSearchpath * XournalMain::initPath(const char * argv0) {
	XOJ_CHECK_TYPE(XournalMain);

	GladeSearchpath * gladePath = new GladeSearchpath();

	// Create config directory if not exists
	gchar * file = g_build_filename(g_get_home_dir(), G_DIR_SEPARATOR_S, CONFIG_DIR, NULL);
	mkdir(file, 0700);
	g_free(file);

	// Add first home dir to search path, to add custom glade XMLs
	gchar * searchPath = g_build_filename(g_get_home_dir(), G_DIR_SEPARATOR_S, CONFIG_DIR, "ui", NULL);

	if (g_file_test(searchPath, G_FILE_TEST_EXISTS) && g_file_test(searchPath, G_FILE_TEST_IS_DIR)) {
		gladePath->addSearchDirectory(searchPath);
	}
	g_free(searchPath);

	searchPath = g_build_filename(PACKAGE_DATA_DIR, PACKAGE, "ui", NULL);
	gladePath->addSearchDirectory(searchPath);
	g_free(searchPath);

	gchar * path;
	path = g_path_get_dirname(argv0);
	searchPath = g_build_filename(path, "ui", NULL);
	gladePath->addSearchDirectory(searchPath);
	g_free(searchPath);

	searchPath = g_build_filename(path, "..", "ui", NULL);
	gladePath->addSearchDirectory(searchPath);
	g_free(searchPath);
	g_free(path);

	char buffer[512] = { 0 };
	path = getcwd(buffer, sizeof(buffer));
	if (path == NULL) {
		return gladePath;
	}

	searchPath = g_build_filename(path, "ui", NULL);
	gladePath->addSearchDirectory(searchPath);
	g_free(searchPath);

	searchPath = g_build_filename(path, "..", "ui", NULL);
	gladePath->addSearchDirectory(searchPath);
	g_free(searchPath);

	return gladePath;
}
