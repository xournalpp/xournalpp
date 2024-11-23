#include "XournalMain.h"

#include <algorithm>  // for copy, sort, max
#include <array>      // for array
#include <chrono>     // for time_point, duration, hours...
#include <clocale>    // for setlocale, LC_NUMERIC
#include <cstdio>     // for printf
#include <cstdlib>    // for exit, size_t
#include <exception>  // for exception
#include <iostream>   // for operator<<, endl, basic_...
#include <locale>     // for locale
#include <memory>     // for unique_ptr, allocator
#include <optional>   // for optional, nullopt
#include <sstream>    // for stringstream
#include <stdexcept>  // for runtime_error
#include <string>     // for string, basic_string
#include <vector>     // for vector

#include <gio/gio.h>      // for GApplication, G_APPLICATION
#include <glib-object.h>  // for G_CALLBACK, g_signal_con...
#include <glib.h>         // for GOptionEntry, gchar, G_O...
#include <gtk/gtk.h>      // for gtk_dialog_add_button
#include <libintl.h>      // for bindtextdomain, textdomain

#include "control/RecentManager.h"           // for RecentManager
#include "control/jobs/BaseExportJob.h"      // for ExportBackgroundType
#include "control/jobs/XournalScheduler.h"   // for XournalScheduler
#include "control/settings/LatexSettings.h"  // for LatexSettings
#include "control/settings/Settings.h"       // for Settings
#include "control/settings/SettingsEnums.h"  // for ICON_THEME_COLOR, ICON_T...
#include "control/xojfile/LoadHandler.h"     // for LoadHandler
#include "gui/GladeSearchpath.h"             // for GladeSearchpath
#include "gui/MainWindow.h"                  // for MainWindow
#include "gui/XournalView.h"                 // for XournalView
#include "model/Document.h"                  // for Document
#include "undo/EmergencySaveRestore.h"       // for EmergencySaveRestore
#include "undo/UndoRedoHandler.h"            // for UndoRedoHandler
#include "util/PathUtil.h"                   // for getConfigFolder, openFil...
#include "util/PlaceholderString.h"          // for PlaceholderString
#include "util/Stacktrace.h"                 // for Stacktrace
#include "util/Util.h"                       // for execInUiThread
#include "util/XojMsgBox.h"                  // for XojMsgBox
#include "util/i18n.h"                       // for _, FS, _F

#include "Control.h"       // for Control
#include "ExportHelper.h"  // for exportImg, exportPdf
#include "config-dev.h"    // for ERRORLOG_DIR
#include "config-git.h"    // for GIT_BRANCH, GIT_ORIGIN_O...
#include "config.h"        // for GETTEXT_PACKAGE, ENABLE_NLS
#include "filesystem.h"    // for path, operator/, exists

namespace {

constexpr auto APP_FLAGS =
        GApplicationFlags(G_APPLICATION_SEND_ENVIRONMENT | G_APPLICATION_NON_UNIQUE | G_APPLICATION_HANDLES_OPEN);

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

void initResourcePath(GladeSearchpath* gladePath, const gchar* relativePathAndFile, bool failIfNotFound = true);

void initCAndCoutLocales() {
    /**
     * Force numbers to be printed out and parsed by C libraries (cairo) in the "classic" locale.
     * This avoids issue with tags when exporting to PDF, see #3551
     */
    setlocale(LC_NUMERIC, "C");

    std::cout.imbue(std::locale());
}

auto migrateSettings() -> MigrateResult {
    const fs::path newConfigPath = Util::getConfigFolder();

    if (!fs::exists(newConfigPath)) {
        const std::array oldPaths = {
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
                constexpr auto msg = "Due to a recent update, Xournal++ has changed where its configuration files are "
                                     "stored.\nThey have been automatically copied from\n\t{1}\nto\n\t{2}";
                return {MigrateStatus::Success, FS(_F(msg) % oldPath.u8string() % newConfigPath.u8string())};
            } catch (const fs::filesystem_error& e) {
                constexpr auto msg =
                        "Due to a recent update, Xournal++ has changed where its configuration files are "
                        "stored.\nHowever, when attempting to copy\n\t{1}\nto\n\t{2}\nmigration failed:\n{3}";
                g_message("Migration failed: %s", e.what());
                return {MigrateStatus::Failure, FS(_F(msg) % oldPath.u8string() % newConfigPath.u8string() % e.what())};
            }
        }
    }
    return {MigrateStatus::NotNeeded, ""};
}

static void deleteFile(const fs::path& file) {
    std::error_code error;
    if (!fs::remove(file, error)) {
        std::stringstream msg;
        msg << FS(_F("Failed to delete file: {1}") % file.u8string()) << std::endl;
        msg << error << std::endl << error.message() << std::endl;
        msg << FS(_F("Please delete the file manually"));
        XojMsgBox::showErrorToUser(nullptr, msg.str());
    }
}

void checkForErrorlog() {
    std::vector<fs::path> errorList;

    try {
        const fs::path errorDir = Util::getCacheSubfolder(ERRORLOG_DIR);
        if (!fs::exists(errorDir)) {
            return;
        }

        // Todo(cpp20): replace std::chrono::hours(168) with std::chrono::weeks(1)
        const auto oldestModificationDate = fs::file_time_type::clock::now() - std::chrono::hours(168);
        for (auto const& f: fs::directory_iterator(errorDir)) {
            if (f.is_regular_file() && f.path().filename().string().substr(0, 8) == "errorlog") {
                if (f.last_write_time() > oldestModificationDate) {
                    errorList.emplace_back(f);
                }
            }
        }
    } catch (fs::filesystem_error& e) {
        g_warning("Filesystem error while looking for crash logs:\n"
                  "   %s\n"
                  "   %s\n",
                  e.path1().c_str(), e.what());
        return;
    }

    if (errorList.empty()) {
        return;
    }

    std::sort(errorList.begin(), errorList.end());
    std::string msg = errorList.size() == 1 ? _("There is a recent errorlogfile from Xournal++. Please file a "
                                                "Bugreport, so the bug may be fixed.") :
                                              _("There are recent errorlogfiles from Xournal++. Please file a "
                                                "Bugreport, so the bug may be fixed.");
    msg += "\n";
    msg += FS(_F("The most recent log file name: {1}") % errorList[0].string());
    msg += "\n\n";
    msg += FS(_F("To prevent this popup from reappearing, simply delete the errorlogfiles."));

    GtkWidget* dialog = gtk_message_dialog_new(nullptr, GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE, "%s",
                                               msg.c_str());

    enum Responses { FILE_REPORT = 1, OPEN_FILE, OPEN_DIR, DELETE_FILE, CANCEL };
    gtk_dialog_add_button(GTK_DIALOG(dialog), _("Send Bugreport"), FILE_REPORT);
    gtk_dialog_add_button(GTK_DIALOG(dialog), _("Open Logfile"), OPEN_FILE);
    gtk_dialog_add_button(GTK_DIALOG(dialog), _("Open Logfile directory"), OPEN_DIR);
    gtk_dialog_add_button(GTK_DIALOG(dialog), _("Delete Logfile"), DELETE_FILE);
    gtk_dialog_add_button(GTK_DIALOG(dialog), _("Cancel"), CANCEL);

    const int response = gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);

    auto const& errorlogPath = fs::path(errorList.front());
    if (response == FILE_REPORT) {
        Util::openFileWithDefaultApplication(PROJECT_BUGREPORT);
        Util::openFileWithDefaultApplication(errorlogPath);
    } else if (response == OPEN_FILE) {
        Util::openFileWithDefaultApplication(errorlogPath);
    } else if (response == OPEN_DIR) {
        Util::openFileWithDefaultApplication(errorlogPath.parent_path());
    } else if (response == DELETE_FILE) {
        deleteFile(errorlogPath);
    }
}

void checkForEmergencySave(Control* control) {
    auto file = Util::getConfigFile("emergencysave.xopp");

    if (!fs::exists(file)) {
        return;
    }

    const std::string msg = _("Xournal++ crashed last time. Would you like to restore the last edited file?");

    GtkWidget* dialog = gtk_message_dialog_new(nullptr, GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE, "%s",
                                               msg.c_str());

    gtk_dialog_add_button(GTK_DIALOG(dialog), _("Delete file"), 1);
    gtk_dialog_add_button(GTK_DIALOG(dialog), _("Restore file"), 2);

    const int res = gtk_dialog_run(GTK_DIALOG(dialog));

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

namespace {
void exitOnMissingPdfFileName(const LoadHandler& loader) {
    if (!loader.getMissingPdfFilename().empty()) {
        auto msg =
                FS(_F("The background file \"{1}\" could not be found. It might have been moved, renamed or deleted.") %
                   loader.getMissingPdfFilename());
        std::cerr << msg << std::endl;
        exit(-2);
    }
}
}  // namespace


/**
 * @brief Export the input file as a bunch of image files (one per page)
 * @param input Path to the input file
 * @param output Path to the output file(s)
 * @param range Page range to be parsed. If range=nullptr, exports the whole file
 * @param layerRange Layer range to be parsed. Will only export those layers, for every exported page.
 *                  If a number is too high for the number of layers on a given page, it is just ignored.
 *                  If range=nullptr, exports all layers.
 * @param pngDpi Set dpi for Png files. Non positive values are ignored
 * @param pngWidth Set the width for Png files. Non positive values are ignored
 * @param pngHeight Set the height for Png files. Non positive values are ignored
 * @param exportBackground If EXPORT_BACKGROUND_NONE, the exported image file has transparent background
 *
 *  The priority is: pngDpi overwrites pngWidth overwrites pngHeight
 *
 * @return 0 on success, -2 on failure opening the input file, -3 on export failure
 */
auto exportImg(const char* input, const char* output, const char* range, const char* layerRange, int pngDpi,
               int pngWidth, int pngHeight, ExportBackgroundType exportBackground) -> int {
    LoadHandler loader;
    Document* doc = loader.loadDocument(input);
    if (doc == nullptr) {
        g_error("%s", loader.getLastError().c_str());
    }

    exitOnMissingPdfFileName(loader);

    return ExportHelper::exportImg(doc, output, range, layerRange, pngDpi, pngWidth, pngHeight, exportBackground);
}

/**
 * @brief Export the input file as pdf
 * @param input Path to the input file
 * @param output Path to the output file
 * @param layerRange Layer range to be parsed. Will only export those layers, for every exported page.
 *                  If a number is too high for the number of layers on a given page, it is just ignored.
 *                  If range=nullptr, exports all layers.
 * @param range Page range to be parsed. If range=nullptr, exports the whole file
 * @param exportBackground If EXPORT_BACKGROUND_NONE, the exported pdf file has white background
 * @param progressiveMode If true, then for each xournalpp page, instead of rendering one PDF page, the page layers are
 * rendered one by one to produce as many pages as there are layers.
 *
 * @return 0 on success, -2 on failure opening the input file, -3 on export failure
 */
auto exportPdf(const char* input, const char* output, const char* range, const char* layerRange,
               ExportBackgroundType exportBackground, bool progressiveMode) -> int {
    LoadHandler loader;
    Document* doc = loader.loadDocument(input);
    if (doc == nullptr) {
        g_error("%s", loader.getLastError().c_str());
    }

    exitOnMissingPdfFileName(loader);

    return ExportHelper::exportPdf(doc, output, range, layerRange, exportBackground, progressiveMode);
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
    gchar* exportLayerRange{};
    int exportPngDpi = -1;
    int exportPngWidth = -1;
    int exportPngHeight = -1;
    gboolean exportNoBackground = false;
    gboolean exportNoRuling = false;
    gboolean progressiveMode = false;
    gboolean disableAudio = false;
    std::unique_ptr<GladeSearchpath> gladePath;
    std::unique_ptr<Control> control;
    std::unique_ptr<MainWindow> win;
};
using XMPtr = XournalMainPrivate*;

/// Checks for input method compatibility and ensures it
void ensure_input_model_compatibility() {
    const char* imModule = g_getenv("GTK_IM_MODULE");
    if (imModule != nullptr) {
        const std::string imModuleString{imModule};
        if (imModuleString == "xim") {
            g_warning("Unsupported input method: %s", imModule);
        }
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

    fs::path p = Util::getDataPath();
    p /= relativePathAndFile;

    if (fs::exists(p)) {
        gladePath->addSearchDirectory(p.parent_path());
        return;
    }

    std::string msg =
            FS(_F("<span foreground='red' size='x-large'>Missing the needed UI file:\n<b>{1}</b></span>\nCould "
                  "not find them at any location.\n  Not relative\n  Not in the Working Path\n  Not in {2}") %
               relativePathAndFile % Util::getDataPath().string());

    if (!failIfNotFound) {
        msg += _("\n\nWill now attempt to run without this file.");
    }
    XojMsgBox::showErrorToUser(nullptr, msg);

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

void on_open_files(GApplication* application, GFile** files, gint numFiles, gchar* hint, XMPtr app_data) {
    if (numFiles != 1) {
        const std::string msg = _("Sorry, Xournal++ can only open one file at once.\n"
                                  "Others are ignored.");
        XojMsgBox::showErrorToUser(GTK_WINDOW(app_data->win->getWindow()), msg);
    }

    const fs::path p = Util::fromGFilename(g_file_get_path(files[0]), false);

    try {
        if (fs::exists(p)) {
            app_data->control->openFile(p);
        } else {
            const std::string msg = FS(_F("File {1} does not exist.") % p.u8string());
            XojMsgBox::showErrorToUser(GTK_WINDOW(app_data->win->getWindow()), msg);
        }
    } catch (const fs::filesystem_error& e) {
        const std::string msg = FS(_F("Filesystem error: {1}\n"
                                      "Sorry, Xournal++ cannot open the file: {2}\n"
                                      "Consider copying the file to a local directory.") %
                                   e.what() % p.u8string());
        XojMsgBox::showErrorToUser(GTK_WINDOW(app_data->win->getWindow()), msg);
    }
    gtk_window_present(GTK_WINDOW(app_data->win->getWindow()));
}

void on_startup(GApplication* application, XMPtr app_data) {
    XournalMain::initLocalisation();
    ensure_input_model_compatibility();
    const MigrateResult migrateResult = migrateSettings();

    app_data->gladePath = std::make_unique<GladeSearchpath>();
    initResourcePath(app_data->gladePath.get(), "ui/about.glade");
    initResourcePath(app_data->gladePath.get(), "ui/xournalpp.css", false);

    app_data->control = std::make_unique<Control>(application, app_data->gladePath.get(), app_data->disableAudio);

    auto& globalLatexTemplatePath = app_data->control->getSettings()->latexSettings.globalTemplatePath;
    if (globalLatexTemplatePath.empty()) {
        globalLatexTemplatePath = findResourcePath("resources/") / "default_template.tex";
        g_message("Using default latex template in %s", globalLatexTemplatePath.string().c_str());
        app_data->control->getSettings()->save();
    }

    app_data->win = std::make_unique<MainWindow>(app_data->gladePath.get(), app_data->control.get(),
                                                 GTK_APPLICATION(application));
    app_data->control->initWindow(app_data->win.get());

    if (migrateResult.status != MigrateStatus::NotNeeded) {
        Util::execInUiThread(
                [=]() { XojMsgBox::showErrorToUser(app_data->control->getGtkWindow(), migrateResult.message); });
    }

    app_data->win->show(nullptr);

    bool opened = false;
    if (app_data->optFilename) {
        if (g_strv_length(app_data->optFilename) != 1) {
            const std::string msg = _("Sorry, Xournal++ can only open one file at once.\n"
                                      "Others are ignored.");
            XojMsgBox::showErrorToUser(GTK_WINDOW(app_data->win->getWindow()), msg);
        }

        const fs::path p = Util::fromGFilename(app_data->optFilename[0], false);

        try {
            if (fs::exists(p)) {
                opened = app_data->control->openFile(p,
                                                     app_data->openAtPageNumber - 1);  // First page for user is page 1
            } else {
                opened = app_data->control->newFile("", p);
            }
        } catch (const fs::filesystem_error& e) {
            const std::string msg = FS(_F("Filesystem error: {1}\n"
                                          "Sorry, Xournal++ cannot open the file: {2}\n"
                                          "Consider copying the file to a local directory.") %
                                       e.what() % p.u8string());
            XojMsgBox::showErrorToUser(GTK_WINDOW(app_data->win->getWindow()), msg);
            opened = app_data->control->newFile("", p);
        }
    } else if (app_data->control->getSettings()->isAutoloadMostRecent()) {
        auto most_recent = RecentManager::getMostRecent();
        if (most_recent) {
            if (auto p = Util::fromUri(gtk_recent_info_get_uri(most_recent.get()))) {
                opened = app_data->control->openFile(*p);
            }
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
    initCAndCoutLocales();

    auto print_version = [&] {
        if (!std::string(GIT_COMMIT_ID).empty()) {
            std::cout << PROJECT_NAME << " " << PROJECT_VERSION << " (" << GIT_COMMIT_ID << ")" << std::endl;
        } else {
            std::cout << PROJECT_NAME << " " << PROJECT_VERSION << std::endl;
        }
        std::cout << "└──libgtk: " << gtk_get_major_version() << "."  //
                  << gtk_get_minor_version() << "."                   //
                  << gtk_get_micro_version() << std::endl;            //
    };

    auto exec_guarded = [&](auto&& fun, auto&& s) {
        try {
            return fun();
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            std::cerr << "In: " << s << std::endl;
            print_version();
            return (1);
        } catch (...) {
            std::cerr << "Error: Unknown exception" << std::endl;
            std::cerr << "In: " << s << std::endl;
            print_version();
            return (1);
        }
    };

    if (app_data->showVersion) {
        print_version();
        return (0);
    }

    if (app_data->pdfFilename && app_data->optFilename && *app_data->optFilename) {
        return exec_guarded(
                [&] {
                    return exportPdf(*app_data->optFilename, app_data->pdfFilename, app_data->exportRange,
                                     app_data->exportLayerRange,
                                     app_data->exportNoBackground ? EXPORT_BACKGROUND_NONE :
                                     app_data->exportNoRuling     ? EXPORT_BACKGROUND_UNRULED :
                                                                    EXPORT_BACKGROUND_ALL,
                                     app_data->progressiveMode);
                },
                "exportPdf");
    }
    if (app_data->imgFilename && app_data->optFilename && *app_data->optFilename) {
        return exec_guarded(
                [&] {
                    return exportImg(*app_data->optFilename, app_data->imgFilename, app_data->exportRange,
                                     app_data->exportLayerRange, app_data->exportPngDpi, app_data->exportPngWidth,
                                     app_data->exportPngHeight,
                                     app_data->exportNoBackground ? EXPORT_BACKGROUND_NONE :
                                     app_data->exportNoRuling     ? EXPORT_BACKGROUND_UNRULED :
                                                                    EXPORT_BACKGROUND_ALL);
                },
                "exportImg");
    }
    return -1;
}

void on_shutdown(GApplication*, XMPtr app_data) {
    app_data->control->saveSettings();
    app_data->win->getXournal()->clearSelection();
    app_data->control->getScheduler()->stop();
}

}  // namespace


void XournalMain::initLocalisation() {
#ifdef ENABLE_NLS
    fs::path localeDir = Util::getGettextFilepath(Util::getLocalePath());

#ifdef _WIN32
    wbindtextdomain(GETTEXT_PACKAGE, localeDir.wstring().c_str());
#else
    bindtextdomain(GETTEXT_PACKAGE, localeDir.u8string().c_str());
#endif

    textdomain(GETTEXT_PACKAGE);

#ifdef _WIN32
    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
#endif

#endif  // ENABLE_NLS

    // Not working on GNU g++(mingww) forWindows! Only working on Linux/macOS and with msvc
    try {
        std::locale::global(std::locale(""));  // "" - system default locale
    } catch (const std::runtime_error& e) {
        g_warning("XournalMain: System default locale could not be set.\n - Caused by: %s\n - Note that it is not "
                  "supported to set the locale using mingw-w64 on windows.\n - This could be solved by compiling "
                  "xournalpp with msvc",
                  e.what());
    }

    initCAndCoutLocales();
}

auto XournalMain::run(int argc, char** argv) -> int {

    XournalMainPrivate app_data;
    GtkApplication* app = gtk_application_new("com.github.xournalpp.xournalpp", APP_FLAGS);
    g_object_set(G_OBJECT(app), "register-session", true, NULL);  // Needed for opening files on MacOS from Finder
    g_set_prgname("com.github.xournalpp.xournalpp");
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
                          GOptionEntry{"disable-audio", 0, 0, G_OPTION_ARG_NONE, &app_data.disableAudio,
                                       _("Disable audio for this session"), nullptr},
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
                         "RANGE"},
            GOptionEntry{"export-layer-range", 0, 0, G_OPTION_ARG_STRING, &app_data.exportLayerRange,
                         _("Only export the layers specified by RANGE (e.g. \"2-3,5,7-\")\n"
                           "                                 No effect without -p/--create-pdf or -i/--create-img"),
                         "RANGE"},
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
