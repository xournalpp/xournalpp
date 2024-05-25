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


// Helper class, for a single open dialog
class FileDlg {
public:
    /**
     * Creates an open file dialog. The callback is only called if a file is actually chosen
     * @param callback(path, attachPdf)
     */
    FileDlg(const char* title, std::function<void(fs::path, bool)> callback);
    /**
     * Creates an open file dialog. The callback is only called if a file is actually chosen
     * @param callback(path)
     */
    FileDlg(const char* title, std::function<void(fs::path)> callback);
    ~FileDlg() = default;

    inline GtkWindow* getWindow() const { return window.get(); }

private:
    xoj::util::GtkWindowUPtr window;

    std::function<void(fs::path, bool)> callback;
    gulong signalId{};
};
constexpr auto ATTACH_PDF_CHOICE_ID = "attachPdfChoice";

static GtkWindow* makeWindow(const char* title) {
    // Todo(maybe)
    // Restore previews using https://discourse.gnome.org/t/file-chooser-gtk-4-image-preview/11510/2
    auto* win = gtk_file_chooser_dialog_new(title, nullptr, GTK_FILE_CHOOSER_ACTION_OPEN, _("_Cancel"),
                                            GTK_RESPONSE_CANCEL, _("_Open"), GTK_RESPONSE_OK, nullptr);
    return GTK_WINDOW(win);
}

FileDlg::FileDlg(const char* title, std::function<void(fs::path, bool)> callback):
        window(makeWindow(title)), callback(std::move(callback)) {
    this->signalId = g_signal_connect(
            window.get(), "response", G_CALLBACK(+[](GtkDialog* win, int response, gpointer data) {
                auto* self = static_cast<FileDlg*>(data);

                if (response == GTK_RESPONSE_OK) {
                    auto path =
                            Util::fromGFile(xoj::util::GObjectSPtr<GFile>(
                                                    gtk_file_chooser_get_file(GTK_FILE_CHOOSER(win)), xoj::util::adopt)
                                                    .get());

                    bool attachPdf = false;
                    if (const char* choice = gtk_file_chooser_get_choice(GTK_FILE_CHOOSER(win), ATTACH_PDF_CHOICE_ID);
                        choice) {
                        attachPdf = std::strcmp(choice, "true") == 0;
                    }

                    // We need to call gtk_window_close() before invoking the callback, because if the callback pops up
                    // another dialog, the first one won't close...
                    // So we postpone the callback
                    Util::execInUiThread([cb = std::move(self->callback), path = std::move(path), attachPdf]() {
                        cb(std::move(path), attachPdf);
                    });
                }
                // Closing the window causes another "response" signal, which we want to ignore
                g_signal_handler_disconnect(win, self->signalId);
                gtk_window_close(self->getWindow());  // Destroys *self. Beware!
            }),
            this);
}

FileDlg::FileDlg(const char* title, std::function<void(fs::path)> callback):
        FileDlg(title, [cb = std::move(callback)](fs::path path, bool) { cb(std::move(path)); }) {}


void xoj::OpenDlg::showOpenTemplateDialog(GtkWindow* parent, Settings* settings,
                                          std::function<void(fs::path)> callback) {
    auto popup = xoj::popup::PopupWindowWrapper<FileDlg>(_("Open template file"),
                                                         addSetLastSavePathToCallback(std::move(callback), settings));

    auto* fc = GTK_FILE_CHOOSER(popup.getPopup()->getWindow());
    xoj::addFilterAllFiles(fc);
    xoj::addFilterXopt(fc);
    setCurrentFolderToLastOpenPath(fc, settings);

    popup.show(parent);
}


void xoj::OpenDlg::showOpenFileDialog(GtkWindow* parent, Settings* settings, std::function<void(fs::path)> callback) {
    auto popup = xoj::popup::PopupWindowWrapper<FileDlg>(_("Open file"),
                                                         addSetLastSavePathToCallback(std::move(callback), settings));

    auto* fc = GTK_FILE_CHOOSER(popup.getPopup()->getWindow());
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
    auto popup = xoj::popup::PopupWindowWrapper<FileDlg>(_("Annotate Pdf file"),
                                                         addSetLastSavePathToCallback(std::move(callback), settings));

    auto* fc = GTK_FILE_CHOOSER(popup.getPopup()->getWindow());

    xoj::addFilterPdf(fc);
    xoj::addFilterAllFiles(fc);

    addlastSavePathShortcut(fc, settings);
    setCurrentFolderToLastOpenPath(fc, settings);

    gtk_file_chooser_add_choice(fc, ATTACH_PDF_CHOICE_ID, _("Attach file to the journal"), nullptr, nullptr);
    gtk_file_chooser_set_choice(fc, ATTACH_PDF_CHOICE_ID, "false");

    popup.show(parent);
}
