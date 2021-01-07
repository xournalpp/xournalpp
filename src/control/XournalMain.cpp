#include "XournalMain.h"

#include <algorithm>
#include <memory>

#include <glib/gstdio.h>
#include <gtk/gtk.h>
#include <gui/toolbarMenubar/model/ToolbarColorNames.h>
#include <libintl.h>

#include "control/jobs/ImageExport.h"
#include "control/jobs/ProgressListener.h"
#include "gui/GladeSearchpath.h"
#include "gui/MainWindow.h"
#include "gui/XournalView.h"
#include "pdf/base/XojPdfExport.h"
#include "pdf/base/XojPdfExportFactory.h"
#include "undo/EmergencySaveRestore.h"
#include "xojfile/LoadHandler.h"

#include "Control.h"
#include "Stacktrace.h"
#include "StringUtils.h"
#include "XojMsgBox.h"
#include "config-dev.h"
#include "config-paths.h"
#include "config.h"
#include "filesystem.h"
#include "i18n.h"

#if __linux__
#include <libgen.h>
#endif
namespace {

constexpr auto APP_FLAGS = GApplicationFlags(G_APPLICATION_SEND_ENVIRONMENT | G_APPLICATION_NON_UNIQUE);

/// Configuration migration status.
enum class MigrateStatus {
    NotNeeded,  ///< No migration was needed.
    Success,    ///< Migration was carried out successfully.
    Failure,    ///< Migration failed. */
};

struct MigrateResult {
    MigrateStatus status{};
    std::string message;  ///< Any additional information about the migration status.
};

auto migrateSettings() -> MigrateResult;

void checkForErrorlog();
void checkForEmergencySave(Control* control);

auto exportPdf(const char* input, const char* output, const char* range, ExportBackgroundType exportBackground,
               bool progressiveMode) -> int;
auto exportImg(const char* input, const char* output, const char* range, int pngDpi, int pngWidth, int pngHeight,
               ExportBackgroundType exportBackground) -> int;

void initResourcePath(GladeSearchpath* gladePath, const gchar* relativePathAndFile, bool failIfNotFound = true);

void initLocalisation() {
#ifdef ENABLE_NLS

#ifdef _WIN32
#undef PACKAGE_LOCALE_DIR
#define PACKAGE_LOCALE_DIR "../share/locale/"
#endif

#ifdef __APPLE__
#undef PACKAGE_LOCALE_DIR
    fs::path p = Stacktrace::getExePath();
    p /= "../Resources/share/locale/";
    const char* PACKAGE_LOCALE_DIR = p.c_str();
#endif

    fs::path localeDir = Util::getGettextFilepath(PACKAGE_LOCALE_DIR);
    bindtextdomain(GETTEXT_PACKAGE, localeDir.u8string().c_str());
    textdomain(GETTEXT_PACKAGE);

#ifdef _WIN32
    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
#endif

#endif  // ENABLE_NLS

    // Not working on GNU g++(mingww) forWindows! Only working on Linux/macOS and with msvc
    try {
        std::locale::global(std::locale(""));  // "" - system default locale
    } catch (std::runtime_error& e) {
        g_warning("XournalMain: System default locale could not be set.\n - Caused by: %s\n - Note that it is not "
                  "supported to set the locale using mingw-w64 on windows.\n - This could be solved by compiling "
                  "xournalpp with msvc",
                  e.what());
    }
    std::cout.imbue(std::locale());
}

auto migrateSettings() -> MigrateResult {
    fs::path newConfigPath = Util::getConfigFolder();

    if (!fs::exists(newConfigPath)) {
        std::array oldPaths = {
                Util::getConfigFolder().parent_path() /= "com.github.xournalpp.xournalpp",
                Util::getConfigFolder().parent_path() /= "com.github.xournalpp.xournalpp.exe",
                fs::u8path(g_get_home_dir()) /= ".xournalpp",
        };
        for (auto const& oldPath: oldPaths) {
            if (!fs::is_directory(oldPath)) {
                continue;
            }
            g_message("Migrating configuration from %s to %s", oldPath.string().c_str(),
                      newConfigPath.string().c_str());
            Util::ensureFolderExists(newConfigPath.parent_path());
            try {
                fs::copy(oldPath, newConfigPath, fs::copy_options::recursive);
                constexpr auto msg = "Due to a recent update, Xournal++ has changed where it's configuration files are "
                                     "stored.\nThey have been automatically copied from\n\t{1}\nto\n\t{2}";
                return {MigrateStatus::Success, FS(_F(msg) % oldPath.u8string() % newConfigPath.u8string())};
            } catch (fs::filesystem_error const& except) {
                constexpr auto msg =
                        "Due to a recent update, Xournal++ has changed where it's configuration files are "
                        "stored.\nHowever, when attempting to copy\n\t{1}\nto\n\t{2}\nmigration failed:\n{3}";
                g_message("Migration failed: %s", except.what());
                return {MigrateStatus::Failure,
                        FS(_F(msg) % oldPath.u8string() % newConfigPath.u8string() % except.what())};
            }
        }
    }
    return {MigrateStatus::NotNeeded, ""};
}

void checkForErrorlog() {
    fs::path errorDir = Util::getCacheSubfolder(ERRORLOG_DIR);
    if (!fs::exists(errorDir)) {
        return;
    }

    std::vector<fs::path> errorList;
    for (auto const& f: fs::directory_iterator(errorDir)) {
        if (f.is_regular_file() && f.path().stem() == "errorlog") {
            errorList.emplace_back(f);
        }
    }

    if (errorList.empty()) {
        return;
    }

    std::sort(errorList.begin(), errorList.end());
    string msg =
            errorList.size() == 1 ?
                    _("There is an errorlogfile from Xournal++. Please send a Bugreport, so the bug may be fixed.") :
                    _("There are errorlogfiles from Xournal++. Please send a Bugreport, so the bug may be fixed.");
    msg += "\n";
    msg += FS(_F("You're using \"{1}/{2}\" branch. Send Bugreport will direct you to this repo's issue tracker.") %
              GIT_ORIGIN_OWNER % GIT_BRANCH);
    msg += "\n";
    msg += FS(_F("The most recent log file name: {1}") % errorList[0].string());

    GtkWidget* dialog = gtk_message_dialog_new(nullptr, GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE, "%s",
                                               msg.c_str());

    gtk_dialog_add_button(GTK_DIALOG(dialog), _("Send Bugreport"), 1);
    gtk_dialog_add_button(GTK_DIALOG(dialog), _("Open Logfile"), 2);
    gtk_dialog_add_button(GTK_DIALOG(dialog), _("Open Logfile directory"), 3);
    gtk_dialog_add_button(GTK_DIALOG(dialog), _("Delete Logfile"), 4);
    gtk_dialog_add_button(GTK_DIALOG(dialog), _("Cancel"), 5);

    int res = gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);

    auto const& errorlogPath = Util::getCacheSubfolder(ERRORLOG_DIR) / errorList[0];
    if (res == 1)  // Send Bugreport
    {
        Util::openFileWithDefaultApplication(PROJECT_BUGREPORT);
        Util::openFileWithDefaultApplication(errorlogPath);
    } else if (res == 2)  // Open Logfile
    {
        Util::openFileWithDefaultApplication(errorlogPath);
    } else if (res == 3)  // Open Logfile directory
    {
        Util::openFileWithFilebrowser(errorlogPath.parent_path());
    } else if (res == 4)  // Delete Logfile
    {
        if (!fs::exists(errorlogPath)) {
            msg = FS(_F("Errorlog cannot be deleted. You have to do it manually.\nLogfile: {1}") %
                     errorlogPath.u8string());
            XojMsgBox::showErrorToUser(nullptr, msg);
        }
    } else if (res == 5)  // Cancel
    {
        // Nothing to do
    }
}

void checkForEmergencySave(Control* control) {
    auto file = Util::getConfigFile("emergencysave.xopp");

    if (!fs::exists(file)) {
        return;
    }

    string msg = _("Xournal++ crashed last time. Would you like to restore the last edited file?");

    GtkWidget* dialog = gtk_message_dialog_new(nullptr, GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE, "%s",
                                               msg.c_str());

    gtk_dialog_add_button(GTK_DIALOG(dialog), _("Delete file"), 1);
    gtk_dialog_add_button(GTK_DIALOG(dialog), _("Restore file"), 2);

    int res = gtk_dialog_run(GTK_DIALOG(dialog));

    if (res == 1)  // Delete file
    {
        fs::remove(file);
    } else if (res == 2)  // Open File
    {
        if (control->openFile(file, -1, true)) {
            control->getDocument()->setFilepath("");

            // Make sure the document is changed, there is a question to ask for save
            control->getUndoRedoHandler()->addUndoAction(std::make_unique<EmergencySaveRestore>());
            control->updateWindowTitle();
            fs::remove(file);
        }
    }

    gtk_widget_destroy(dialog);
}

/**
 * @brief Export the input file as a bunch of image files (one per page)
 * @param input Path to the input file
 * @param output Path to the output file(s)
 * @param range Page range to be parsed. If range=nullptr, exports the whole file
 * @param pngDpi Set dpi for Png files. Non positive values are ignored
 * @param pngWidth Set the width for Png files. Non positive values are ignored
 * @param pngHeight Set the height for Png files. Non positive values are ignored
 * @param exportBackground If EXPORT_BACKGROUND_NONE, the exported image file has transparent background
 *
 *  The priority is: pngDpi overwrites pngWidth overwrites pngHeight
 *
 * @return 0 on success, -2 on failure opening the input file, -3 on export failure
 */
auto exportImg(const char* input, const char* output, const char* range, int pngDpi, int pngWidth, int pngHeight,
               ExportBackgroundType exportBackground) -> int {
    LoadHandler loader;

    Document* doc = loader.loadDocument(input);
    if (doc == nullptr) {
        g_error("%s", loader.getLastError().c_str());
    }

    fs::path const path(output);

    ExportGraphicsFormat format = EXPORT_GRAPHICS_PNG;

    if (path.extension() == ".svg") {
        format = EXPORT_GRAPHICS_SVG;
    }

    PageRangeVector exportRange;
    if (range) {
        exportRange = PageRange::parse(range, int(doc->getPageCount()));
    } else {
        exportRange.push_back(new PageRangeEntry(0, int(doc->getPageCount() - 1)));
    }

    DummyProgressListener progress;

    ImageExport imgExport(doc, path, format, exportBackground, exportRange);

    if (format == EXPORT_GRAPHICS_PNG) {
        if (pngDpi > 0) {
            imgExport.setQualityParameter(EXPORT_QUALITY_DPI, pngDpi);
        } else if (pngWidth > 0) {
            imgExport.setQualityParameter(EXPORT_QUALITY_WIDTH, pngWidth);
        } else if (pngHeight > 0) {
            imgExport.setQualityParameter(EXPORT_QUALITY_HEIGHT, pngHeight);
        }
    }

    imgExport.exportGraphics(&progress);

    for (PageRangeEntry* e: exportRange) {
        delete e;
    }
    exportRange.clear();

    string errorMsg = imgExport.getLastErrorMsg();
    if (!errorMsg.empty()) {
        g_message("Error exporting image: %s\n", errorMsg.c_str());
    }

    g_message("%s", _("Image file successfully created"));

    return 0;  // no error
}

/**
 * @brief Export the input file as pdf
 * @param input Path to the input file
 * @param output Path to the output file
 * @param range Page range to be parsed. If range=nullptr, exports the whole file
 * @param exportBackground If EXPORT_BACKGROUND_NONE, the exported pdf file has white background
 * @param progressiveMode If true, then for each xournalpp page, instead of rendering one PDF page, the page layers are
 * rendered one by one to produce as many pages as there are layers.
 *
 * @return 0 on success, -2 on failure opening the input file, -3 on export failure
 */
auto exportPdf(const char* input, const char* output, const char* range, ExportBackgroundType exportBackground,
               bool progressiveMode) -> int {
    LoadHandler loader;

    Document* doc = loader.loadDocument(input);
    if (doc == nullptr) {
        g_error("%s", loader.getLastError().c_str());
    }

    GFile* file = g_file_new_for_commandline_arg(output);

    XojPdfExport* pdfe = XojPdfExportFactory::createExport(doc, nullptr);
    pdfe->setExportBackground(exportBackground);
    char* cpath = g_file_get_path(file);
    string path = cpath;
    g_free(cpath);
    g_object_unref(file);

    bool exportSuccess;  // Return of the export job

    if (range) {
        // Parse the range
        PageRangeVector exportRange = PageRange::parse(range, doc->getPageCount());
        // Do the export
        exportSuccess = pdfe->createPdf(path, exportRange, progressiveMode);
        // Clean up
        for (PageRangeEntry* e: exportRange) {
            delete e;
        }
        exportRange.clear();
    } else {
        exportSuccess = pdfe->createPdf(path, progressiveMode);
    }

    if (!exportSuccess) {
        g_error("%s", pdfe->getLastError().c_str());
        // delete pdfe; Unreachable. Todo: use std::unique_ptr
    }
    delete pdfe;

    g_message("%s", _("PDF file successfully created"));

    return 0;  // no error
}

struct XournalMainPrivate {
    XournalMainPrivate() = default;
    XournalMainPrivate(XournalMainPrivate&&) = delete;
    XournalMainPrivate(XournalMainPrivate const&) = delete;
    auto operator=(XournalMainPrivate&&) -> XournalMainPrivate = delete;
    auto operator=(XournalMainPrivate const&) -> XournalMainPrivate = delete;

    ~XournalMainPrivate() {
        g_strfreev(optFilename);
        g_free(pdfFilename);
        g_free(imgFilename);
    }

    gchar** optFilename{};
    gchar* pdfFilename{};
    gchar* imgFilename{};
    gboolean showVersion = false;
    int openAtPageNumber = 0;  // when no --page is used, the document opens at the page specified in the metadata file
    gchar* exportRange{};
    int exportPngDpi = -1;
    int exportPngWidth = -1;
    int exportPngHeight = -1;
    gboolean exportNoBackground = false;
    gboolean exportNoRuling = false;
    gboolean progressiveMode = false;
    std::unique_ptr<GladeSearchpath> gladePath;
    std::unique_ptr<Control> control;
    std::unique_ptr<MainWindow> win;
};
using XMPtr = XournalMainPrivate*;

/// Checks for input method compatibility and ensures it
void ensure_input_model_compatibility() {
    const char* imModule = g_getenv("GTK_IM_MODULE");
    if (imModule != nullptr && strcmp(imModule, "xim") == 0) {
        g_setenv("GTK_IM_MODULE", "ibus", true);
        g_warning("Unsupported input method: xim, changed to: ibus");
    }
}

/**
 * Find a file in a resource folder, and return the resource folder path
 * Return an empty string, if the folder was not found
 */
auto findResourcePath(const fs::path& searchFile) -> fs::path {
    auto search_for = [&searchFile](fs::path start) -> std::optional<fs::path> {
        constexpr auto* postfix = "share/xournalpp";
        /// 1. relative install
        /// 2. windows install
        /// 3. build dir
        for (int i = 0; i < 3; ++i, start = start.parent_path()) {
            if (auto target = start / searchFile; fs::exists(target)) {
                return target.parent_path();
            }

            if (auto folder = start / postfix / searchFile; fs::exists(folder)) {
                return folder.parent_path();
            }
        }
        return std::nullopt;
    };
    /*    /// relative execution path
        if (auto path = search_for(fs::path{}); path) {
            return *path;
        }*/
    /// real execution path
    if (auto path = search_for(Stacktrace::getExePath().parent_path()); path) {
        return *path;
    }
    // Not found
    return {};
}

void initResourcePath(GladeSearchpath* gladePath, const gchar* relativePathAndFile, bool failIfNotFound) {
    auto uiPath = findResourcePath(relativePathAndFile);  // i.e.  relativePathAndFile = "ui/about.glade"

    if (!uiPath.empty()) {
        gladePath->addSearchDirectory(uiPath);
        return;
    }

    // -----------------------------------------------------------------------

#ifdef __APPLE__
    fs::path p = Stacktrace::getExePath();
    p /= "../Resources";
    p /= relativePathAndFile;

    if (fs::exists(p)) {
        gladePath->addSearchDirectory(p.parent_path());
        return;
    }

    string msg = FS(_F("Missing the needed UI file:\n{1}\n .app corrupted?\nPath: {2}") % relativePathAndFile %
                    p.u8string());

    if (!failIfNotFound) {
        msg += _("\nWill now attempt to run without this file.");
    }
    XojMsgBox::showErrorToUser(nullptr, msg);
#else
    // Check at the target installation directory
    fs::path absolute = PACKAGE_DATA_DIR;
    absolute /= PROJECT_PACKAGE;
    absolute /= relativePathAndFile;

    if (fs::exists(absolute)) {
        gladePath->addSearchDirectory(absolute.parent_path());
        return;
    }

    string msg = FS(_F("<span foreground='red' size='x-large'>Missing the needed UI file:\n<b>{1}</b></span>\nCould "
                       "not find them at any location.\n  Not relative\n  Not in the Working Path\n  Not in {2}") %
                    relativePathAndFile % PACKAGE_DATA_DIR);

    if (!failIfNotFound) {
        msg += _("\n\nWill now attempt to run without this file.");
    }
    XojMsgBox::showErrorToUser(nullptr, msg);
#endif

    if (failIfNotFound) {
        exit(12);
    }
}

void on_activate(GApplication*, XMPtr) {}

void on_command_line(GApplication*, GApplicationCommandLine*, XMPtr) {
    g_message("XournalMain::on_command_line: This should never happen, please file a bugreport with a detailed "
              "description how to reproduce this message");
    // Todo: implement this, if someone files the bug report
}

void on_open_files(GApplication*, gpointer, gint, gchar*, XMPtr) {
    g_message("XournalMain::on_open_files: This should never happen, please file a bugreport with a detailed "
              "description how to reproduce this message");
    // Todo: implement this, if someone files the bug report
}

void on_startup(GApplication* application, XMPtr app_data) {
    initLocalisation();
    ensure_input_model_compatibility();
    MigrateResult migrateResult = migrateSettings();

    app_data->gladePath = std::make_unique<GladeSearchpath>();
    initResourcePath(app_data->gladePath.get(), "ui/about.glade");
    initResourcePath(app_data->gladePath.get(), "ui/xournalpp.css", false);

    // init singleton
    // ToolbarColorNames::getInstance();
    app_data->control = std::make_unique<Control>(application, app_data->gladePath.get());
    {
        auto icon = app_data->gladePath->getFirstSearchPath() / "icons";
        gtk_icon_theme_prepend_search_path(gtk_icon_theme_get_default(), icon.u8string().c_str());
    }

    if (app_data->control->getSettings()->isDarkTheme()) {
        auto icon = app_data->gladePath->getFirstSearchPath() / "iconsDark";
        gtk_icon_theme_prepend_search_path(gtk_icon_theme_get_default(), icon.u8string().c_str());
    }

    auto& globalLatexTemplatePath = app_data->control->getSettings()->latexSettings.globalTemplatePath;
    if (globalLatexTemplatePath.empty()) {
        globalLatexTemplatePath = findResourcePath("resources/") / "default_template.tex";
        g_message("Using default latex template in %s", globalLatexTemplatePath.string().c_str());
        app_data->control->getSettings()->save();
    }

    app_data->win = std::make_unique<MainWindow>(app_data->gladePath.get(), app_data->control.get());
    app_data->control->initWindow(app_data->win.get());

    if (migrateResult.status != MigrateStatus::NotNeeded) {
        Util::execInUiThread(
                [=]() { XojMsgBox::showErrorToUser(app_data->control->getGtkWindow(), migrateResult.message); });
    }

    app_data->win->show(nullptr);

    bool opened = false;
    if (app_data->optFilename) {
        if (g_strv_length(app_data->optFilename) != 1) {
            string msg = _("Sorry, Xournal++ can only open one file at once.\n"
                           "Others are ignored.");
            XojMsgBox::showErrorToUser(static_cast<GtkWindow*>(*app_data->win), msg);
        }

        fs::path p = Util::fromGFilename(app_data->optFilename[0], false);

        try {
            if (fs::exists(p)) {
                opened = app_data->control->openFile(p,
                                                     app_data->openAtPageNumber - 1);  // First page for user is page 1
            } else {
                opened = app_data->control->newFile("", p);
            }
        } catch (fs::filesystem_error const& e) {
            string msg = FS(_F("Sorry, Xournal++ cannot open remote files at the moment.\n"
                               "You have to copy the file to a local directory.") %
                            p.u8string() % e.what());
            XojMsgBox::showErrorToUser(static_cast<GtkWindow*>(*app_data->win), msg);
            opened = app_data->control->newFile("", p);
        }
    }

    app_data->control->getScheduler()->start();

    if (!opened) {
        app_data->control->newFile();
    }

    checkForErrorlog();
    checkForEmergencySave(app_data->control.get());

    // There is a timing issue with the layout
    // This fixes it, see #405
    Util::execInUiThread([=]() { app_data->control->getWindow()->getXournal()->layoutPages(); });
    gtk_application_add_window(GTK_APPLICATION(application), GTK_WINDOW(app_data->win->getWindow()));
}

auto on_handle_local_options(GApplication*, GVariantDict*, XMPtr app_data) -> gint {
    if (app_data->showVersion) {
        std::cout << PROJECT_NAME << " " << PROJECT_VERSION << std::endl;
        std::cout << "└──libgtk: " << gtk_get_major_version() << "."  //
                  << gtk_get_minor_version() << "."                   //
                  << gtk_get_micro_version() << std::endl;            //
        return 0;
    }

    if (app_data->pdfFilename && app_data->optFilename && *app_data->optFilename) {
        return exportPdf(*app_data->optFilename, app_data->pdfFilename, app_data->exportRange,
                         app_data->exportNoBackground ? EXPORT_BACKGROUND_NONE :
                         app_data->exportNoRuling     ? EXPORT_BACKGROUND_UNRULED :
                                                        EXPORT_BACKGROUND_ALL,
                         app_data->progressiveMode);
    }
    if (app_data->imgFilename && app_data->optFilename && *app_data->optFilename) {
        return exportImg(*app_data->optFilename, app_data->imgFilename, app_data->exportRange, app_data->exportPngDpi,
                         app_data->exportPngWidth, app_data->exportPngHeight,
                         app_data->exportNoBackground ? EXPORT_BACKGROUND_NONE :
                         app_data->exportNoRuling     ? EXPORT_BACKGROUND_UNRULED :
                                                        EXPORT_BACKGROUND_ALL);
    }
    return -1;
}

void on_shutdown(GApplication*, XMPtr app_data) {
    app_data->control->saveSettings();
    app_data->win->getXournal()->clearSelection();
    app_data->control->getScheduler()->stop();
    ToolbarColorNames::getInstance().save();
}

}  // namespace

auto XournalMain::run(int argc, char** argv) -> int {

    XournalMainPrivate app_data;
    GtkApplication* app = gtk_application_new("com.github.xournalpp.xournalpp", APP_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(&on_activate), &app_data);
    g_signal_connect(app, "command-line", G_CALLBACK(&on_command_line), &app_data);
    g_signal_connect(app, "open", G_CALLBACK(&on_open_files), &app_data);
    g_signal_connect(app, "startup", G_CALLBACK(&on_startup), &app_data);
    g_signal_connect(app, "shutdown", G_CALLBACK(&on_shutdown), &app_data);
    g_signal_connect(app, "handle-local-options", G_CALLBACK(&on_handle_local_options), &app_data);

    std::array options = {GOptionEntry{"page", 'n', 0, G_OPTION_ARG_INT, &app_data.openAtPageNumber,
                                       _("Jump to Page (first Page: 1)"), "N"},
                          GOptionEntry{G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_FILENAME_ARRAY, &app_data.optFilename,
                                       "<input>", nullptr},
                          GOptionEntry{"version", 0, 0, G_OPTION_ARG_NONE, &app_data.showVersion,
                                       _("Get version of xournalpp"), nullptr},
                          GOptionEntry{nullptr}};  // Must be terminated by a nullptr. See gtk doc
    g_application_add_main_option_entries(G_APPLICATION(app), options.data());

    /**
     * Export related options
     */
    std::array exportOptions = {
            GOptionEntry{"create-pdf", 'p', G_OPTION_FLAG_IN_MAIN, G_OPTION_ARG_FILENAME, &app_data.pdfFilename,
                         _("Export FILE as PDF"), "PDFFILE"},
            GOptionEntry{"create-img", 'i', G_OPTION_FLAG_IN_MAIN, G_OPTION_ARG_FILENAME, &app_data.imgFilename,
                         _("Export FILE as image files (one per page)\n"
                           "                                 Guess the output format from the extension of IMGFILE\n"
                           "                                 Supported formats: .png, .svg"),
                         "IMGFILE"},
            GOptionEntry{"export-no-background", 0, 0, G_OPTION_ARG_NONE, &app_data.exportNoBackground,
                         _("Export without background\n"
                           "                                 The exported file has transparent or white background,\n"
                           "                                 depending on what its format supports\n"),
                         0},
            GOptionEntry{"export-no-ruling", 0, 0, G_OPTION_ARG_NONE, &app_data.exportNoRuling,
                         _("Export without ruling\n"
                           "                                 The exported file has no paper ruling\n"),
                         0},
            GOptionEntry{"export-layers-progressively", 0, 0, G_OPTION_ARG_NONE, &app_data.progressiveMode,
                         _("Export layers progressively\n"
                           "                                 In PDF export, Render layers progressively one by one.\n"
                           "                                 This results in N export pages per page with N layers,\n"
                           "                                 building up the layer stack progressively.\n"
                           "                                 The resulting PDF file can be used for a presentation.\n"),
                         0},
            GOptionEntry{"export-range", 0, 0, G_OPTION_ARG_STRING, &app_data.exportRange,
                         _("Only export the pages specified by RANGE (e.g. \"2-3,5,7-\")\n"
                           "                                 No effect without -p/--create-pdf or -i/--create-img"),
                         nullptr},
            GOptionEntry{"export-png-dpi", 0, 0, G_OPTION_ARG_INT, &app_data.exportPngDpi,
                         _("Set DPI for PNG exports. Default is 300\n"
                           "                                 No effect without -i/--create-img=foo.png"),
                         "N"},
            GOptionEntry{"export-png-width", 0, 0, G_OPTION_ARG_INT, &app_data.exportPngWidth,
                         _("Set page width for PNG exports\n"
                           "                                 No effect without -i/--create-img=foo.png\n"
                           "                                 Ignored if --export-png-dpi is used"),
                         "N"},
            GOptionEntry{
                    "export-png-height", 0, 0, G_OPTION_ARG_INT, &app_data.exportPngHeight,
                    _("Set page height for PNG exports\n"
                      "                                 No effect without -i/--create-img=foo.png\n"
                      "                                 Ignored if --export-png-dpi or --export-png-width is used"),
                    "N"},
            GOptionEntry{nullptr}};  // Must be terminated by a nullptr. See gtk doc
    GOptionGroup* exportGroup = g_option_group_new("export", _("Advanced export options"),
                                                   _("Display advanced export options"), nullptr, nullptr);
    g_option_group_add_entries(exportGroup, exportOptions.data());
    g_application_add_option_group(G_APPLICATION(app), exportGroup);

    auto rv = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return rv;
}
