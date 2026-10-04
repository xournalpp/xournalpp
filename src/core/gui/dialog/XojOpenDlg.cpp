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

static void addlastSavePathShortcut(GtkFileChooser* fc, Settings* settings) {
    auto lastSavePath = settings->getLastSavePath();
    if (!lastSavePath.empty()) {
        gtk_file_chooser_add_shortcut_folder(fc, Util::toGFile(lastSavePath).get(), nullptr);
    }
}

static void setCurrentFolderToLastOpenPath(GtkFileChooser* fc, Settings* settings) {
    xoj::util::GObjectSPtr<GFile> currentFolder;
    if (settings && !settings->getLastOpenPath().empty()) {
        currentFolder = Util::toGFile(settings->getLastOpenPath());
    } else {
        currentFolder.reset(g_file_new_for_path(g_get_home_dir()), xoj::util::adopt);
    }
    gtk_file_chooser_set_current_folder(fc, currentFolder.get(), nullptr);
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

// Helper class, for a single open dialog
class FileDlg {
public:
    enum class Type { FILE, FOLDER };
    /**
     * Creates an open file dialog. The callback is only called if a file is actually chosen
     * @param callback(path, attachPdf)
     */
    FileDlg(Type type, const char* title, std::function<void(fs::path, bool)> callback);
    /**
     * Creates an open file dialog. The callback is only called if a file is actually chosen
     * @param callback(path)
     */
    FileDlg(Type type, const char* title, std::function<void(fs::path)> callback);
    ~FileDlg() = default;

    inline GtkFileChooser* getFileChooser() const { return GTK_FILE_CHOOSER(window.get()); }
    inline GtkNativeDialog* getNativeDialog() const { return GTK_NATIVE_DIALOG(window.get()); }

private:
    xoj::util::GtkNativeDialogUPtr window;

    std::function<void(fs::path, bool)> callback;
    gulong signalId{};
};

static GtkNativeDialog* makeWindow(FileDlg::Type type, const char* title) {
    // Todo(maybe)
    // Restore previews using https://discourse.gnome.org/t/file-chooser-gtk-4-image-preview/11510/2
    return GTK_NATIVE_DIALOG(gtk_file_chooser_native_new(
            title, nullptr,
            type == FileDlg::Type::FILE ? GTK_FILE_CHOOSER_ACTION_OPEN : GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
            type == FileDlg::Type::FILE ? _("_Open") : _("Select folder"),
            _("_Cancel")));
}

FileDlg::FileDlg(Type type, const char* title, std::function<void(fs::path, bool)> callback):
        window(makeWindow(type, title)), callback(std::move(callback)) {
    this->signalId = g_signal_connect(
            window.get(), "response", G_CALLBACK(+[](GtkDialog* win, int response, gpointer data) {
                auto* self = static_cast<FileDlg*>(data);

                if (response == GTK_RESPONSE_ACCEPT) {
                    auto path =
                            Util::fromGFile(xoj::util::GObjectSPtr<GFile>(
                                                    gtk_file_chooser_get_file(GTK_FILE_CHOOSER(win)), xoj::util::adopt)
                                                    .get());

                    bool attach = false;
                    if (const char* choice = gtk_file_chooser_get_choice(GTK_FILE_CHOOSER(win), ATTACH_CHOICE_ID);
                        choice) {
                        attach = std::strcmp(choice, "true") == 0;
                    }

                    // We need to call gtk_window_close() before invoking the callback, because if the callback pops up
                    // another dialog, the first one won't close...
                    // So we postpone the callback
                    Util::execInUiThread([cb = std::move(self->callback), path = std::move(path), attach]() {
                        cb(std::move(path), attach);
                    });
                }
                self->window.reset();  // Dropping the ref will destroy it all
                delete self;
            }),
            this);
}

FileDlg::FileDlg(Type type, const char* title, std::function<void(fs::path)> callback):
        FileDlg(type, title, [cb = std::move(callback)](fs::path path, bool) { cb(std::move(path)); }) {}


void xoj::OpenDlg::showOpenTemplateDialog(GtkWindow* parent, Settings* settings,
                                          std::function<void(fs::path)> callback) {
    auto popup = xoj::popup::PopupWindowWrapper<FileDlg>(FileDlg::Type::FILE, _("Open template file"),
                                                         addSetLastSavePathToCallback(std::move(callback), settings));

    auto* fc = popup.getPopup()->getFileChooser();
    xoj::addFilterAllFiles(fc);
    xoj::addFilterXopt(fc);
    setCurrentFolderToLastOpenPath(fc, settings);

    popup.showNative(parent);
}


void xoj::OpenDlg::showOpenFileDialog(GtkWindow* parent, Settings* settings, std::function<void(fs::path)> callback) {
    auto popup = xoj::popup::PopupWindowWrapper<FileDlg>(FileDlg::Type::FILE, _("Open file"),
                                                         addSetLastSavePathToCallback(std::move(callback), settings));

    auto* fc = popup.getPopup()->getFileChooser();
    xoj::addFilterSupported(fc);
    xoj::addFilterXoj(fc);
    xoj::addFilterXopt(fc);
    xoj::addFilterXopp(fc);
    xoj::addFilterPdf(fc);
    xoj::addFilterAllFiles(fc);

    addlastSavePathShortcut(fc, settings);
    setCurrentFolderToLastOpenPath(fc, settings);

    popup.showNative(parent);
}

void xoj::OpenDlg::showAnnotatePdfDialog(GtkWindow* parent, Settings* settings,
                                         std::function<void(fs::path, bool)> callback) {
    auto popup = xoj::popup::PopupWindowWrapper<FileDlg>(FileDlg::Type::FILE, _("Annotate Pdf file"),
                                                         addSetLastSavePathToCallback(std::move(callback), settings));

    auto* fc = popup.getPopup()->getFileChooser();

    xoj::addFilterPdf(fc);
    xoj::addFilterAllFiles(fc);

    addlastSavePathShortcut(fc, settings);
    setCurrentFolderToLastOpenPath(fc, settings);

    addAttachChoice(fc);

    popup.showNative(parent);
}

void xoj::OpenDlg::showOpenImageDialog(GtkWindow* parent, Settings* settings,
                                       std::function<void(fs::path, bool)> callback) {
    auto popup = xoj::popup::PopupWindowWrapper<FileDlg>(FileDlg::Type::FILE, _("Choose image file"),
                                                         [cb = std::move(callback), settings](fs::path p, bool attach) {
                                                             if (auto folder = p.parent_path(); !folder.empty()) {
                                                                 settings->setLastImagePath(folder);
                                                             }
                                                             cb(std::move(p), attach);
                                                         });

    auto* fc = popup.getPopup()->getFileChooser();

    xoj::addFilterImages(fc);
    xoj::addFilterAllFiles(fc);

    if (!settings->getLastImagePath().empty()) {
        gtk_file_chooser_set_current_folder(fc, Util::toGFile(settings->getLastImagePath()).get(), nullptr);
    }

    addAttachChoice(fc);

    popup.showNative(parent);
}

void xoj::OpenDlg::showMultiFormatDialog(GtkWindow* parent, std::vector<std::string> formats,
                                         std::function<void(fs::path)> callback) {
    auto popup = xoj::popup::PopupWindowWrapper<FileDlg>(FileDlg::Type::FILE, _("Open file"), std::move(callback));

    auto* fc = popup.getPopup()->getFileChooser();

    if (formats.size() > 0) {
        GtkFileFilter* filterSupported = gtk_file_filter_new();
        gtk_file_filter_set_name(filterSupported, _("Supported files"));
        for (std::string format: formats) {
            gtk_file_filter_add_pattern(filterSupported, format.c_str());
        }
        gtk_file_chooser_add_filter(fc, filterSupported);
    }

    popup.showNative(parent);
}

void xoj::OpenDlg::showOpenTexDialog(GtkWindow* parent, const fs::path& preset,
                                     std::function<void(fs::path)> callback) {
    auto popup = xoj::popup::PopupWindowWrapper<FileDlg>(FileDlg::Type::FILE, _("Choose Latex template file"),
                                                         std::move(callback));

    auto* fc = popup.getPopup()->getFileChooser();
    xoj::addFilterTex(fc);
    xoj::addFilterAllFiles(fc);
    gtk_file_chooser_set_current_folder(fc, Util::toGFile(preset.parent_path()).get(), nullptr);

    popup.showNative(parent);
}

void xoj::OpenDlg::showSelectFolderDialog(GtkWindow* parent, const char* title, const fs::path& preset,
                                          std::function<void(fs::path)> callback) {
    auto popup = xoj::popup::PopupWindowWrapper<FileDlg>(FileDlg::Type::FOLDER, title, std::move(callback));

    auto* fc = popup.getPopup()->getFileChooser();
    if (!preset.empty()) {
        gtk_file_chooser_set_current_folder(fc, Util::toGFile(preset).get(), nullptr);
    }

    popup.showNative(parent);
}
