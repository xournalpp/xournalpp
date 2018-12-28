#include "XournalMain.h"

#include "Control.h"

#include "gui/GladeSearchpath.h"
#include "gui/MainWindow.h"
#include "gui/toolbarMenubar/model/ToolbarColorNames.h"
#include "gui/XournalView.h"
#include "pdf/base/XojPdfExport.h"
#include "pdf/base/XojPdfExportFactory.h"
#include "xojfile/LoadHandler.h"

#include <config.h>
#include <config-dev.h>
#include <config-paths.h>
#include <i18n.h>
#include <Stacktrace.h>
#include <StringUtils.h>
#include <XojMsgBox.h>

#include <libintl.h>
#include <gtk/gtk.h>

#if __linux__
#include <libgen.h>
#endif

#include <algorithm> // std::sort
#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

XournalMain::XournalMain()
{
	XOJ_INIT_TYPE(XournalMain);
}

XournalMain::~XournalMain()
{
	XOJ_RELEASE_TYPE(XournalMain);
}

void XournalMain::initLocalisation()
{
	XOJ_CHECK_TYPE(XournalMain);

#ifdef ENABLE_NLS

#ifdef WIN32
#undef PACKAGE_LOCALE_DIR
#define PACKAGE_LOCALE_DIR "../share/po/"
#endif

	bindtextdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
	textdomain(GETTEXT_PACKAGE);
	
#ifdef WIN32
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
#endif
	
#endif //ENABLE_NLS

	// Not working on Windows! Working on Linux, but not sure if it's needed
#ifndef WIN32
	std::locale::global(std::locale("")); // "" - system default locale
#endif
	std::cout.imbue(std::locale());
}

void XournalMain::checkForErrorlog()
{
	XOJ_CHECK_TYPE(XournalMain);

	Path errorDir = Util::getConfigSubfolder(ERRORLOG_DIR);
	GDir* home = g_dir_open(errorDir.c_str(), 0, NULL);

	if (home == NULL)
	{
		return;
	}

	vector<string> errorList;

	const gchar* file;
	while ((file = g_dir_read_name(home)) != NULL)
	{
		if (g_file_test(file, G_FILE_TEST_IS_REGULAR))
		{
			if (StringUtils::startsWith(file, "errorlog."))
			{
				errorList.push_back(file);
			}
		}
	}
	g_dir_close(home);

	
	if (errorList.empty())
	{
		return;
	}

	std::sort(errorList.begin(), errorList.end());
	string msg = errorList.size() == 1
			? _("There is an errorlogfile from Xournal++. Please send a Bugreport, so the bug may be fixed.")
			: _("There are errorlogfiles from Xournal++. Please send a Bugreport, so the bug may be fixed.");
	msg += "\n";
#if defined(GIT_BRANCH) && defined(GIT_REPO_OWNER)
	msg += FS(_F("You're using {1}/{2} branch. Send Bugreport will direct you to this repo's issue tracker.")
					% GIT_REPO_OWNER % GIT_BRANCH);
	msg += "\n";
#endif
	msg += FS(_F("The most recent log file name: {1}") % errorList[0]);

	GtkWidget* dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL,
		GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE, "%s", msg.c_str());

	gtk_dialog_add_button(GTK_DIALOG(dialog), _("Send Bugreport"), 1);
	gtk_dialog_add_button(GTK_DIALOG(dialog), _("Open Logfile"), 2);
	gtk_dialog_add_button(GTK_DIALOG(dialog), _("Open Logfile directory"), 3);
	gtk_dialog_add_button(GTK_DIALOG(dialog), _("Delete Logfile"), 4);
	gtk_dialog_add_button(GTK_DIALOG(dialog), _("Cancel"), 5);

	int res = gtk_dialog_run(GTK_DIALOG(dialog));

	Path errorlogPath = Util::getConfigSubfolder(ERRORLOG_DIR);
	errorlogPath /= errorList[0];
	if (res == 1) // Send Bugreport
	{
		Util::openFileWithDefaultApplicaion(PROJECT_BUGREPORT);
		Util::openFileWithDefaultApplicaion(errorlogPath);
	}
	else if (res == 2) // Open Logfile
	{
		Util::openFileWithDefaultApplicaion(errorlogPath);
	}
	else if (res == 3) // Open Logfile directory
	{
		Util::openFileWithFilebrowser(errorlogPath.getParentPath());
	}
	else if (res == 4) // Delete Logfile
	{
		if (!errorlogPath.exists())
		{
			string msg = FS(_F("Errorlog cannot be deleted. You have to do it manually.\nLogfile: {1}")
					% errorlogPath.str());
			XojMsgBox::showErrorToUser(NULL, msg);
		}
	}
	else if (res == 5) // Cancel
	{
		// Nothing to do
	}

	gtk_widget_destroy(dialog);
}

void XournalMain::checkForEmergencySave() {
	// Not implemented. See Ticket #627
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

	XojPdfExport* pdfe = XojPdfExportFactory::createExport(doc, NULL);
	if (!pdfe->createPdf(g_file_get_path(file)))
	{
		cerr << pdfe->getLastError() << endl;

		g_object_unref(file);
		delete pdfe;
		return -3;
	}
	delete pdfe;

	g_object_unref(file);

	cout << _("PDF file successfully created") << endl;

	return 0; // no error
}

int XournalMain::run(int argc, char* argv[])
{
	XOJ_CHECK_TYPE(XournalMain);

	this->initLocalisation();

	GError* error = NULL;
	GOptionContext* context = g_option_context_new("FILE");

	bool optNoPdfCompress = false;
	gchar** optFilename = NULL;
	gchar* pdfFilename = NULL;
	int openAtPageNumber = -1;

	string pdf_no_compress = _("Don't compress PDF files (for debugging)");
	string create_pdf = _("PDF output filename");
	string page_jump = _("Jump to Page (first Page: 1)");
	string audio_folder = _("Absolute path for the audio files playback");
	GOptionEntry options[] = {
		{ "pdf-no-compress",   0, 0, G_OPTION_ARG_NONE,           &optNoPdfCompress, pdf_no_compress.c_str(), NULL },
		{ "create-pdf",      'p', 0, G_OPTION_ARG_FILENAME,       &pdfFilename,      create_pdf.c_str(), NULL },
		{ "page",            'n', 0, G_OPTION_ARG_INT,            &openAtPageNumber, page_jump.c_str(), "N" },
		{G_OPTION_REMAINING,   0, 0, G_OPTION_ARG_FILENAME_ARRAY, &optFilename,      "<input>", NULL },
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

	if (optNoPdfCompress)
	{
		XojPdfExportFactory::setCompressPdfOutput(false);
	}

	if (pdfFilename && optFilename && *optFilename)
	{
		return exportPdf(*optFilename, pdfFilename);
	}

	// Checks for input method compatibility
	
	const char* imModule = g_getenv("GTK_IM_MODULE");
	if (imModule != NULL && strcmp(imModule, "xim") == 0)
	{
		g_setenv("GTK_IM_MODULE", "ibus", true);
		g_warning("Unsupported input method: xim, changed to: ibus");
	}

	// Init GTK Display
	gtk_init(&argc, &argv);

	GladeSearchpath* gladePath = new GladeSearchpath();
	initResourcePath(gladePath);

	// init singleton
	string colorNameFile = Util::getConfigFile("colornames.ini").str();
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
			string msg = _("Sorry, Xournal++ can only open one file at once.\n"
						   "Others are ignored.");
			XojMsgBox::showErrorToUser((GtkWindow*) *win, msg);
		}

		GFile* file = g_file_new_for_commandline_arg(optFilename[0]);
		char* filename = g_file_get_path(file);
		char* uri = g_file_get_uri(file);
		string sUri = uri;
		g_free(uri);
		g_object_unref(file);

		if (StringUtils::startsWith(sUri, "file://"))
		{
			opened = control->openFile(filename, openAtPageNumber);
			g_free(filename);
		}
		else
		{
			string msg = _("Sorry, Xournal++ cannot open remote files at the moment.\n"
						   "You have to copy the file to a local directory.");
			XojMsgBox::showErrorToUser((GtkWindow*) *win, msg);
		}
	}

	control->getScheduler()->start();

	if (!opened)
	{
		control->newFile();
	}

	checkForErrorlog();
	checkForEmergencySave();

	// There is a timing issue with the layout
	// This fixes it, see #405
	Util::execInUiThread([=]() {
		control->getWindow()->getXournal()->layoutPages();
	});

	gtk_main();

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
 * Find a file in a resource folder, and return the resource folder path
 * Return an empty string, if the folder was not found
 */
string XournalMain::findResourcePath(string searchFile)
{
	XOJ_CHECK_TYPE(XournalMain);

	// First check if the files are available relative to the path
	// So a "portable" installation will be possible
	Path relative1 = searchFile;

	if (relative1.exists())
	{
		return relative1.getParentPath().str();
	}

	// -----------------------------------------------------------------------

	// Check if we are in the "build" directory, and therefore the resources
	// are installed two folders back
	Path relative2 = "../..";
	relative2 /= searchFile;

	if (relative2.exists())
	{
		return relative2.getParentPath().str();
	}

	// -----------------------------------------------------------------------

	Path executableDir = Stacktrace::getExePath();
	executableDir = executableDir.getParentPath();

	// First check if the files are available relative to the executable
	// So a "portable" installation will be possible
	Path relative3 = executableDir;
	relative3 /= searchFile;

	if (relative3.exists())
	{
		return relative3.getParentPath().str();
	}

	// -----------------------------------------------------------------------

	// Check one folder back, for windows portable
	Path relative4 = executableDir;
	relative4 /= "..";
	relative4 /= searchFile;

	if (relative4.exists())
	{
		return relative4.getParentPath().str();
	}

	// -----------------------------------------------------------------------

	// Check if we are in the "build" directory, and therefore the resources
	// are installed two folders back
	Path relative5 = executableDir;
	relative5 /= "../..";
	relative5 /= searchFile;

	if (relative5.exists())
	{
		return relative5.getParentPath().str();
	}

	// Not found
	return "";
}

void XournalMain::initResourcePath(GladeSearchpath* gladePath)
{
	XOJ_CHECK_TYPE(XournalMain);

	string uiPath = findResourcePath("ui/about.glade");

	if (uiPath != "")
	{
		gladePath->addSearchDirectory(uiPath);
		return;
	}

	// -----------------------------------------------------------------------

	// Check at the target installation directory
	Path absolute = PACKAGE_DATA_DIR;
	absolute /= PROJECT_PACKAGE;
	absolute /= "ui/about.glade";

	if (absolute.exists())
	{
		gladePath->addSearchDirectory(absolute.getParentPath().str());
		return;
	}

	string msg = FS(_F("Missing the needed UI file, could not find them at any location.\nNot relative\nNot in the Working Path\nNot in {1}") % PACKAGE_DATA_DIR);
	XojMsgBox::showErrorToUser(NULL, msg);

	exit(12);
}
