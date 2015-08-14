#include "XournalMain.h"

#include "Control.h"

#include "gui/GladeSearchpath.h"
#include "gui/MainWindow.h"
#include "gui/toolbarMenubar/model/ToolbarColorNames.h"
#include "gui/XournalView.h"
#include "pdf/popplerdirect/PdfExport.h"
#include "xojfile/LoadHandler.h"

#include <config.h>
#include <config-dev.h>
#include <config-paths.h>
#include <i18n.h>

#include <gtk/gtk.h>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem.hpp>
#include <boost/locale.hpp>
namespace bf = boost::filesystem;

#include <string>
using std::string;
#include <iostream>
using std::cout;
using std::cerr;
using std::endl;
#include <vector>
using std::vector;

XournalMain::XournalMain()
{
	XOJ_INIT_TYPE(XournalMain);
}

XournalMain::~XournalMain()
{
	XOJ_RELEASE_TYPE(XournalMain);
}

//it HAS to be done – otherwise such things like boost::algorithm::to_lower wont work, throwing casting exceptions
void XournalMain::initLocalisation()
{
	XOJ_CHECK_TYPE(XournalMain);

	//locale generator
	boost::locale::generator gen;
#ifdef ENABLE_NLS
	gen.add_messages_path(PACKAGE_LOCALE_DIR);
	gen.add_messages_domain(GETTEXT_PACKAGE);
	
	bindtextdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
	textdomain(GETTEXT_PACKAGE);
#endif //ENABLE_NLS

	std::locale::global(gen("")); //"" - system default locale
	std::cout.imbue(std::locale());
}

void XournalMain::checkForErrorlog()
{
	XOJ_CHECK_TYPE(XournalMain);

	bf::path filename = Util::getConfigFile("errorlog.log");
	if (bf::exists(filename))
	{
		string msg = _("There is an errorlogfile from Xournal++. Please send a Bugreport, "
					   "so the bug may been fixed.\n");
#if defined(GIT_BRANCH) && defined(GIT_REPO_OWNER)
		msg += (bl::format("You're using {1}/{2} branch. Send Bugreport will direct you to this repo's issue tracker.\n")
						% GIT_REPO_OWNER % GIT_BRANCH).str();
#endif
		msg += _("Logfile: %s");
		
		GtkWidget* dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL,
			GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE, msg.c_str(),
			filename.c_str());
		//I know it's formatting/i18n hell, but for now it have to wait some time
		
		gtk_dialog_add_button(GTK_DIALOG(dialog), _C("Send Bugreport"), 1);
		gtk_dialog_add_button(GTK_DIALOG(dialog), _C("Open Logfile"), 2);
		gtk_dialog_add_button(GTK_DIALOG(dialog), _C("Delete Logfile"), 3);
		gtk_dialog_add_button(GTK_DIALOG(dialog), _C("Cancel"), 4);

		int res = gtk_dialog_run(GTK_DIALOG(dialog));

		if (res == 1) // Send Bugreport
		{
			Util::openFileWithDefaultApplicaion(PROJECT_BUGREPORT);
			Util::openFileWithFilebrowser(filename);
		}
		else if (res == 2) // Open Logfile
		{
			Util::openFileWithFilebrowser(filename);
		}
		else if (res == 3) // Delete Logfile
		{
			if (bf::remove(filename) != 1)
			{
				GtkWidget* dlgError = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL,
					GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "%s",
					_C("Errorlog could not be deleted. You have to delete it manually.\nLogfile: %s"),
					filename.c_str());
				gtk_dialog_run(GTK_DIALOG(dlgError));
			}
		}
		else if (res == 4) // Cancel
		{
			// Nothing to do
		}

		gtk_widget_destroy(dialog);
	}
}

void XournalMain::checkForEmergencySave() {
	// TODO Check for emergency save document!
	//	gchar * filename = g_strconcat(g_get_home_dir(), G_DIR_SEPARATOR_S, CONFIG_DIR, G_DIR_SEPARATOR_S, "errorlog.log", NULL);
	//	if (g_file_test(filename, G_FILE_TEST_EXISTS)) {
	//		GtkWidget * dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE, _C(
	//				"There is an errorlogfile from Xournal++. Please send a Bugreport, so the bug may been fixed.\nLogfile: %s"), filename);
	//		gtk_dialog_add_button(GTK_DIALOG(dialog), _C("Send Bugreport"), 1);
	//		gtk_dialog_add_button(GTK_DIALOG(dialog), _C("Open Logfile"), 2);
	//		gtk_dialog_add_button(GTK_DIALOG(dialog), _C("Delete Logfile"), 3);
	//		gtk_dialog_add_button(GTK_DIALOG(dialog), _C("Cancel"), 4);
	//
	//		int res = gtk_dialog_run(GTK_DIALOG(dialog));
	//
	//		if (res == 1) { // Send Bugreport
	//			Util::openFileWithDefaultApplicaion("http://sourceforge.net/tracker/?group_id=163434&atid=827733");
	//		} else if (res == 2) { // Open Logfile
	//			Util::openFileWithFilebrowser(filename);
	//		} else if (res == 3) { // Delete Logfile
	//			if (g_unlink(filename) != 0) {
	//				GtkWidget * dlgError = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "%s", _C(
	//						"Errorlog could not be deleted. You have to delete it manually.\nLogfile: %s"), filename);
	//				gtk_dialog_run(GTK_DIALOG(dlgError));
	//			}
	//		} else if (res == 4) { // Cancel
	//			// Nothing to do
	//		}
	//
	//		gtk_widget_destroy(dialog);
	//	}
}

int XournalMain::exportPdf(const char* input, const char* output)
{
	XOJ_CHECK_TYPE(XournalMain);

	LoadHandler loader;

	Document* doc = loader.loadDocument(input);
	if (doc == NULL)
	{
		cerr << loader.getLastError() << endl;
		return -2;
	}

	GFile* file = g_file_new_for_commandline_arg(output);

	PdfExport pdf(doc, NULL);
	if (!pdf.createPdf(g_file_get_uri(file)))
	{
		cerr << pdf.getLastError() << endl;

		g_object_unref(file);
		return -3;
	}

	g_object_unref(file);

	cout << "PDF File successfully created" << endl;

	return 0; // no error
}

int XournalMain::run(int argc, char* argv[])
{
	XOJ_CHECK_TYPE(XournalMain);

	this->initLocalisation();

	GError* error = NULL;
	GOptionContext* context = context = g_option_context_new("FILE");

	bool optNoPdfCompress = false;
	gchar** optFilename = NULL;
	gchar* pdfFilename = NULL;
	int openAtPageNumber = -1;

	GOptionEntry options[] = {
		{ "pdf-no-compress",   0, 0, G_OPTION_ARG_NONE,           &optNoPdfCompress, "Don't compress PDF files (for debugging)", NULL },
		{ "create-pdf",      'p', 0, G_OPTION_ARG_FILENAME,       &pdfFilename,      "PDF output filename", NULL },
		{ "page",            'n', 0, G_OPTION_ARG_INT,            &openAtPageNumber, "Jump to Page (first Page: 1)", "N" },
		{G_OPTION_REMAINING,   0, 0, G_OPTION_ARG_FILENAME_ARRAY, &optFilename,      "<input>", NULL},
		{NULL}
	};

	g_option_context_add_main_entries(context, options, GETTEXT_PACKAGE);
	// parse options, so we don't need gtk_init, but don't init display (so we have a commandline mode)
	g_option_context_add_group(context, gtk_get_option_group(false));
	if (!g_option_context_parse(context, &argc, &argv, &error))
	{
		cerr << error->message << endl;
		g_error_free(error);
		gchar* help = g_option_context_get_help(context, true, NULL);
		cout << help;
		g_free(help);
		error = NULL;
	}
	g_option_context_free(context);

#ifdef DEV_MEMORY_LEAK_CHECKING
	xoj_type_initMutex();
#endif

	if (optNoPdfCompress)
	{
		PdfWriter::setCompressPdfOutput(false);
	}

	if (pdfFilename && optFilename && *optFilename)
	{
		return exportPdf(*optFilename, pdfFilename);
	}

	// Init GTK Display
	gdk_display_open_default_libgtk_only();

	GladeSearchpath* gladePath = initPath(argv[0]);

	// init singleton
	string colorNameFile = Util::getConfigFile("colornames.ini").string();
	ToolbarColorNames::getInstance().loadFile(colorNameFile);

	Control* control = new Control(gladePath);

	MainWindow* win = new MainWindow(gladePath, control);
	control->initWindow(win);

	win->show(NULL);

	bool opened = false;
	if (optFilename)
	{
		if (g_strv_length(optFilename) != 1)
		{
			GtkWidget* dialog = gtk_message_dialog_new((GtkWindow*) *win,
													GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
													"Sorry, Xournal can only open one file from the command line.\n"
													"Others are ignored.");
			gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(win->getWindow()));
			gtk_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(dialog);
		}

		GFile* file = g_file_new_for_commandline_arg(optFilename[0]);
		char* filename = g_file_get_path(file);
		char* uri = g_file_get_uri(file);
		string sUri = uri;
		g_free(uri);
		g_object_unref(file);

		if (ba::starts_with(sUri, "file://"))
		{
			opened = control->openFile(filename, openAtPageNumber);
			g_free(filename);
		}
		else
		{
			GtkWidget* dialog = gtk_message_dialog_new((GtkWindow*) *win,
													   GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
													   _C("Sorry, Xournal cannot open remote files at the moment.\n"
													   "You have to copy the file to a local directory."));
			gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(win->getWindow()));
			gtk_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(dialog);
		}
	}

	control->getScheduler()->start();

	if (!opened)
	{
		control->newFile();
	}

	checkForErrorlog();
	checkForEmergencySave();

	gdk_threads_enter();
	gtk_main();
	gdk_threads_leave();

	control->saveSettings();

	win->getXournal()->clearSelection();

	control->getScheduler()->stop();

	delete win;
	delete control;
	delete gladePath;

	ToolbarColorNames::getInstance().saveFile(colorNameFile);
	ToolbarColorNames::freeInstance();

	return 0;
}

/**
 * Path for glade files and Pixmaps, first searches in the home folder, so you can customize glade files
 */
GladeSearchpath* XournalMain::initPath(const char* argv0)
{
	XOJ_CHECK_TYPE(XournalMain);

	GladeSearchpath* gladePath = new GladeSearchpath();

	// Create config directory if not exists
	path file = Util::getConfigSubfolder("");
	bf::create_directories(file);
	bf::permissions(file, bf::perms::owner_all);

	// Add first home dir to search path, to add custom glade XMLs
	{
		path searchPath = Util::getConfigSubfolder("ui");
		if (bf::exists(searchPath) && bf::is_directory(searchPath))
		{
			gladePath->addSearchDirectory(searchPath.c_str());
		}
	}

	gchar* path = g_path_get_dirname(argv0);
	gchar* searchPath = g_build_filename(path, "ui", NULL);
	gladePath->addSearchDirectory(searchPath);
	g_free(searchPath);

	searchPath = g_build_filename(path, "..", "ui", NULL);
	gladePath->addSearchDirectory(searchPath);
	g_free(searchPath);
	g_free(path);

	char buffer[512] = { 0 };
	path = getcwd(buffer, sizeof(buffer));
	if (path == NULL)
	{
		return gladePath;
	}

	searchPath = g_build_filename(path, "ui", NULL);
	gladePath->addSearchDirectory(searchPath);
	g_free(searchPath);

	searchPath = g_build_filename(path, "..", "ui", NULL);
	gladePath->addSearchDirectory(searchPath);
	g_free(searchPath);
	
	searchPath = g_build_filename(PROJECT_SOURCE_DIR, "ui", NULL);
	gladePath->addSearchDirectory(searchPath);
	g_free(searchPath);

	searchPath = g_build_filename(PACKAGE_DATA_DIR, PROJECT_PACKAGE, "ui", NULL);
	gladePath->addSearchDirectory(searchPath);
	g_free(searchPath);

	return gladePath;
}
