#include "XournalMain.h"

#include "Control.h"

#include "control/jobs/ImageExport.h"
#include "control/jobs/ProgressListener.h"
#include "gui/GladeSearchpath.h"
#include "gui/MainWindow.h"
#include "gui/toolbarMenubar/model/ToolbarColorNames.h"
#include "gui/XournalView.h"
#include "pdf/base/XojPdfExport.h"
#include "pdf/base/XojPdfExportFactory.h"
#include "undo/EmergencySaveRestore.h"
#include "xojfile/LoadHandler.h"


#include "config.h"
#include "config-dev.h"
#include "config-paths.h"
#include "i18n.h"
#include "Stacktrace.h"
#include "StringUtils.h"
#include "XojMsgBox.h"
#include "util/cpp14memory.h"

#include <libintl.h>
#include <gtk/gtk.h>
#include <glib/gstdio.h>

#if __linux__
#include <libgen.h>
#endif

#include <algorithm>  // std::sort


XournalMain::XournalMain()
{
}

XournalMain::~XournalMain()
{
}

void XournalMain::initLocalisation()
{
#ifdef ENABLE_NLS

#ifdef _WIN32
#undef PACKAGE_LOCALE_DIR
#define PACKAGE_LOCALE_DIR "../share/locale/"
#endif

#ifdef __APPLE__
#undef PACKAGE_LOCALE_DIR
	Path p = Stacktrace::getExePath();
	p /= "../Resources/share/locale/";
	const char* PACKAGE_LOCALE_DIR = p.c_str();
#endif
	bindtextdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
	textdomain(GETTEXT_PACKAGE);

#ifdef _WIN32
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
#endif

#endif //ENABLE_NLS

	// Not working on Windows! Working on Linux, but not sure if it's needed
#ifndef _WIN32
try
{
	std::locale::global(std::locale("")); // "" - system default locale
}
catch (std::runtime_error &e)
{
	g_warning("XournalMain: System default locale could not be set.\nCaused by: %s", e.what());
}
#endif
	std::cout.imbue(std::locale());
}

void XournalMain::checkForErrorlog()
{
	Path errorDir = Util::getConfigSubfolder(ERRORLOG_DIR);
	GDir* home = g_dir_open(errorDir.c_str(), 0, nullptr);

	if (home == nullptr)
	{
		return;
	}

	vector<string> errorList;

	const gchar* file;
	while ((file = g_dir_read_name(home)) != nullptr)
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

	GtkWidget* dialog = gtk_message_dialog_new(nullptr, GTK_DIALOG_MODAL,
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
			XojMsgBox::showErrorToUser(nullptr, msg);
		}
	}
	else if (res == 5) // Cancel
	{
		// Nothing to do
	}

	gtk_widget_destroy(dialog);
}

void XournalMain::checkForEmergencySave(Control* control) {
	Path filename = Util::getConfigFile("emergencysave.xopp");

	if (!filename.exists())
	{
		return;
	}

	string msg = _("Xournal++ crashed last time. Would you like to restore the last edited file?");

	GtkWidget* dialog = gtk_message_dialog_new(nullptr, GTK_DIALOG_MODAL,
		GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE, "%s", msg.c_str());

	gtk_dialog_add_button(GTK_DIALOG(dialog), _("Delete file"), 1);
	gtk_dialog_add_button(GTK_DIALOG(dialog), _("Restore file"), 2);

	int res = gtk_dialog_run(GTK_DIALOG(dialog));

	if (res == 1) // Delete file
	{
		g_unlink(filename.c_str());
	}
	else if (res == 2) // Open File
	{
		if (control->openFile(filename, -1, true))
		{
			control->getDocument()->setFilename("");

			// Make sure the document is changed, there is a question to ask for save
			control->getUndoRedoHandler()->addUndoAction(mem::make_unique<EmergencySaveRestore>());
			control->updateWindowTitle();
			g_unlink(filename.c_str());
		}
	}

	gtk_widget_destroy(dialog);
}

int XournalMain::exportImg(const char* input, const char* output)
{
	LoadHandler loader;

	Document* doc = loader.loadDocument(input);
	if (doc == nullptr)
	{
		g_error("%s", loader.getLastError().c_str());
		return -2;
	}

	GFile* file = g_file_new_for_commandline_arg(output);

	char* cpath = g_file_get_path(file);
	string path = cpath;
	g_free(cpath);
	g_object_unref(file);

	ExportGraphicsFormat format = EXPORT_GRAPHICS_PNG;

	if (StringUtils::endsWith(path, ".svg"))
	{
		format = EXPORT_GRAPHICS_SVG;
	}

	PageRangeVector exportRange;
	exportRange.push_back(new PageRangeEntry(0, doc->getPageCount() - 1));
	DummyProgressListener progress;

	ImageExport imgExport(doc, path, format, false, exportRange);
	imgExport.exportGraphics(&progress);

	for (PageRangeEntry* e : exportRange)
	{
		delete e;
	}
	exportRange.clear();

	string errorMsg = imgExport.getLastErrorMsg();
	if (errorMsg != "")
	{
		g_message("Error exporting image: %s\n", errorMsg.c_str());
		return -3;
	}

	g_message("%s", _("Image file successfully created"));

	return 0; // no error
}

int XournalMain::exportPdf(const char* input, const char* output)
{
	LoadHandler loader;

	Document* doc = loader.loadDocument(input);
	if (doc == nullptr)
	{
		g_error("%s", loader.getLastError().c_str());
		return -2;
	}

	GFile* file = g_file_new_for_commandline_arg(output);

	XojPdfExport* pdfe = XojPdfExportFactory::createExport(doc, nullptr);
	char* cpath = g_file_get_path(file);
	string path = cpath;
	g_free(cpath);
	g_object_unref(file);

	if (!pdfe->createPdf(path))
	{
		g_error("%s", pdfe->getLastError().c_str());

		delete pdfe;
		return -3;
	}
	delete pdfe;

	g_message("%s", _("PDF file successfully created"));

	return 0; // no error
}

int XournalMain::run(int argc, char* argv[])
{
	this->initLocalisation();

	GError* error = nullptr;
	GOptionContext* context = g_option_context_new("FILE");

	gchar** optFilename = nullptr;
	gchar* pdfFilename = nullptr;
	gchar* imgFilename = nullptr;
	int openAtPageNumber = -1;

	string create_pdf = _("PDF output filename");
	string create_img = _("Image output filename (.png / .svg)");
	string page_jump = _("Jump to Page (first Page: 1)");
	string audio_folder = _("Absolute path for the audio files playback");
	GOptionEntry options[] = {
		{ "create-pdf",      'p', 0, G_OPTION_ARG_FILENAME,       &pdfFilename,      create_pdf.c_str(), nullptr },
		{ "create-img",      'i', 0, G_OPTION_ARG_FILENAME,       &imgFilename,      create_img.c_str(), nullptr },
		{ "page",            'n', 0, G_OPTION_ARG_INT,            &openAtPageNumber, page_jump.c_str(), "N" },
		{G_OPTION_REMAINING,   0, 0, G_OPTION_ARG_FILENAME_ARRAY, &optFilename,      "<input>", nullptr },
		{nullptr}
	};

	g_option_context_add_main_entries(context, options, GETTEXT_PACKAGE);
	// parse options, so we don't need gtk_init, but don't init display (so we have a commandline mode)
	g_option_context_add_group(context, gtk_get_option_group(false));
	if (!g_option_context_parse(context, &argc, &argv, &error))
	{
		g_error("%s", error->message);
		g_error_free(error);
		gchar* help = g_option_context_get_help(context, true, nullptr);
		g_message("%s", help);
		g_free(help);
		error = nullptr;
	}
	g_option_context_free(context);

	if (pdfFilename && optFilename && *optFilename)
	{
		return exportPdf(*optFilename, pdfFilename);
	}
	if (imgFilename && optFilename && *optFilename)
	{
		return exportImg(*optFilename, imgFilename);
	}

	// Checks for input method compatibility

	const char* imModule = g_getenv("GTK_IM_MODULE");
	if (imModule != nullptr && strcmp(imModule, "xim") == 0)
	{
		g_setenv("GTK_IM_MODULE", "ibus", true);
		g_warning("Unsupported input method: xim, changed to: ibus");
	}

	// Init GTK Display
	gtk_init(&argc, &argv);

	GladeSearchpath* gladePath = new GladeSearchpath();
	initResourcePath(gladePath, "ui/about.glade");
	initResourcePath(gladePath, "ui/xournalpp.css",  false); 	//will notify user if file not present. Path ui/ already added above.

	// init singleton
	string colorNameFile = Util::getConfigFile("colornames.ini").str();
	ToolbarColorNames::getInstance().loadFile(colorNameFile);

	Control* control = new Control(gladePath);

	if (control->getSettings()->isDarkTheme())
	{
		string icon = gladePath->getFirstSearchPath() + "/iconsDark/";
		gtk_icon_theme_prepend_search_path(gtk_icon_theme_get_default(), icon.c_str());
	}

	string icon = gladePath->getFirstSearchPath() + "/icons/";
	gtk_icon_theme_prepend_search_path(gtk_icon_theme_get_default(), icon.c_str());

	MainWindow* win = new MainWindow(gladePath, control);
	control->initWindow(win);

	win->show(nullptr);

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
		Path p = Path::fromGFile(file);
		g_object_unref(file);

		if (!p.isEmpty())
		{
			opened = control->openFile(p, openAtPageNumber);
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
	checkForEmergencySave(control);

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

	// -----------------------------------------------------------------------

	// Check for .../share resources directory relative to binary to support
	// relocatable installations (such as e.g., AppImages)
	Path relative6 = executableDir;
	relative6 /= "../share/xournalpp/";
	relative6 /= searchFile;

	if (relative6.exists())
	{
		return relative6.getParentPath().str();
	}

	// Not found
	return "";
}

void XournalMain::initResourcePath(GladeSearchpath* gladePath, const gchar* relativePathAndFile, bool failIfNotFound)
{
	string uiPath = findResourcePath(relativePathAndFile);	//i.e.  relativePathAndFile = "ui/about.glade"

	if (uiPath != "")
	{
		gladePath->addSearchDirectory(uiPath);
		return;
	}

	// -----------------------------------------------------------------------

#ifdef __APPLE__
	Path p = Stacktrace::getExePath();
	p /= "../Resources";
	p /= relativePathAndFile;

	if (p.exists())
	{
		gladePath->addSearchDirectory(p.getParentPath().str());
		return;
	}

	string msg = FS(_F("Missing the needed UI file:\n{1}\n .app corrupted?\nPath: {2}") % relativePathAndFile % p.str());
	
	if (!failIfNotFound)
	{
		msg += _("\nWill now attempt to run without this file.");
	}
	XojMsgBox::showErrorToUser(nullptr, msg);
#else
	// Check at the target installation directory
	Path absolute = PACKAGE_DATA_DIR;
	absolute /= PROJECT_PACKAGE;
	absolute /= relativePathAndFile;

	if (absolute.exists())
	{
		gladePath->addSearchDirectory(absolute.getParentPath().str());
		return;
	}

	
	string msg = FS(_F("<span foreground='red' size='x-large'>Missing the needed UI file:\n<b>{1}</b></span>\nCould not find them at any location.\n  Not relative\n  Not in the Working Path\n  Not in {2}") % relativePathAndFile % PACKAGE_DATA_DIR);
	
	if (!failIfNotFound)
	{
		msg += _("\n\nWill now attempt to run without this file.");
	}
	XojMsgBox::showErrorToUser(nullptr, msg);
#endif

	if (failIfNotFound)
	{
		exit(12);
	}
}
