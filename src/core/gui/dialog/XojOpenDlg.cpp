#include "XojOpenDlg.h"

#include "control/settings/Settings.h"  // for Settings
#include "util/PathUtil.h"              // for fromGFile, toGFile
#include "util/PopupWindowWrapper.h"    // for PopupWindowWrapper
#include "util/Util.h"
#include "util/gtk4_helper.h"         // for gtk_file_chooser_set_current_folder
#include "util/i18n.h"                // for _
#include "util/raii/GObjectSPtr.h"    // for GObjectSPtr
#include "util/raii/GtkWindowUPtr.h"  // for GtkWindowUPtr

#include "FileChooserFiltersHelper.h"
#include "FileDialogWrapper.h"

static void addlastSavePathShortcut(GtkFileChooser* fc, Settings* settings) {
    auto lastSavePath = settings->getLastSavePath();
    if (!lastSavePath.empty()) {
#if GTK_MAJOR_VERSION == 3
        gtk_file_chooser_add_shortcut_folder(fc, lastSavePath.u8string().c_str(), nullptr);
#else
        gtk_file_chooser_add_shortcut_folder(fc, Util::toGFile(lastSavePath.u8string()).get(), nullptr);
#endif
    }
}

static void setCurrentFolderToLastOpenPath(GtkFileChooser* fc, Settings* settings) {
    fs::path currentFolder;
    if (settings && !settings->getLastOpenPath().empty()) {
        currentFolder = settings->getLastOpenPath();
    } else {
        currentFolder = g_get_home_dir();
    }
    gtk_file_chooser_set_current_folder(fc, Util::toGFile(currentFolder).get(), nullptr);
}

template <class... Args>
static std::function<void(fs::path, Args...)> addSetLastSavePathToCallback(
        std::function<void(fs::path, Args...)> callback, Settings* settings) {
    return [cb = std::move(callback), settings](fs::path path, Args... args) {
        if (settings && !path.empty()) {
            settings->setLastOpenPath(path.parent_path());
        }
        cb(std::move(path), std::forward<Args>(args)...);
    };
}

constexpr auto ATTACH_CHOICE_ID = "attachPdfChoice";
static void addAttachChoice(GtkFileChooser* fc) {
    gtk_file_chooser_add_choice(fc, ATTACH_CHOICE_ID, _("Attach file to the journal"), nullptr, nullptr);
    gtk_file_chooser_set_choice(fc, ATTACH_CHOICE_ID, "false");
}



void xoj::OpenDlg::showOpenTemplateDialog(GtkWindow* parent, Settings* settings,
                                          std::function<void(fs::path)> callback) {
    auto popup = xoj::popup::FileDialogWrapper<popup::OpenFileDialog>(_("Open template file"),
                                                         addSetLastSavePathToCallback(std::move(callback), settings));

    auto* fc = GTK_FILE_CHOOSER(popup.getPopup()->getNativeDialog());
    xoj::addFilterAllFiles(fc);
    xoj::addFilterXopt(fc);
    setCurrentFolderToLastOpenPath(fc, settings);

    popup.show(parent);
}


void xoj::OpenDlg::showOpenFileDialog(GtkWindow* parent, Settings* settings, std::function<void(fs::path)> callback) {
    auto popup = xoj::popup::FileDialogWrapper<popup::OpenFileDialog>(_("Open file"),
                                                         addSetLastSavePathToCallback(std::move(callback), settings));

    auto* fc = GTK_FILE_CHOOSER(popup.getPopup()->getNativeDialog());
    xoj::addFilterSupported(fc);
    xoj::addFilterXoj(fc);
    xoj::addFilterXopt(fc);
    xoj::addFilterXopp(fc);
    xoj::addFilterPdf(fc);
    xoj::addFilterAllFiles(fc);

    addlastSavePathShortcut(fc, settings);
    setCurrentFolderToLastOpenPath(fc, settings);

    popup.show(parent);
}

void xoj::OpenDlg::showAnnotatePdfDialog(GtkWindow* parent, Settings* settings,
                                         std::function<void(fs::path, bool)> callback) {
    auto popup = xoj::popup::FileDialogWrapper<popup::OpenFileDialog>(_("Annotate Pdf file"),
                                                         addSetLastSavePathToCallback(std::move(callback), settings));

    auto* fc = GTK_FILE_CHOOSER(popup.getPopup()->getNativeDialog());

    xoj::addFilterPdf(fc);
    xoj::addFilterAllFiles(fc);

    addlastSavePathShortcut(fc, settings);
    setCurrentFolderToLastOpenPath(fc, settings);

    addAttachChoice(fc);

    popup.show(parent);
}

void xoj::OpenDlg::showOpenImageDialog(GtkWindow* parent, Settings* settings,
                                       std::function<void(fs::path, bool)> callback) {
    auto popup = xoj::popup::FileDialogWrapper<popup::OpenFileDialog>(_("Choose image file"),
                                                         [cb = std::move(callback), settings](fs::path p, bool attach) {
                                                             if (auto folder = p.parent_path(); !folder.empty()) {
                                                                 settings->setLastImagePath(folder);
                                                             }
                                                             cb(std::move(p), attach);
                                                         });

    auto* fc = GTK_FILE_CHOOSER(popup.getPopup()->getNativeDialog());

    xoj::addFilterImages(fc);
    xoj::addFilterAllFiles(fc);

    if (!settings->getLastImagePath().empty()) {
        gtk_file_chooser_set_current_folder(fc, Util::toGFile(settings->getLastImagePath()).get(), nullptr);
    }

    addAttachChoice(fc);

    popup.show(parent);
}

void xoj::OpenDlg::showMultiFormatDialog(GtkWindow* parent, std::vector<std::string> formats,
                                         std::function<void(fs::path)> callback) {
    auto popup = xoj::popup::FileDialogWrapper<popup::OpenFileDialog>(_("Open file"), std::move(callback));

    auto* fc = GTK_FILE_CHOOSER(popup.getPopup()->getNativeDialog());

    if (formats.size() > 0) {
        GtkFileFilter* filterSupported = gtk_file_filter_new();
        gtk_file_filter_set_name(filterSupported, _("Supported files"));
        for (std::string format: formats) {
            gtk_file_filter_add_pattern(filterSupported, format.c_str());
        }
        gtk_file_chooser_add_filter(fc, filterSupported);
    }

    popup.show(parent);
}
