#include "XojOpenDlg.h"

#include "control/settings/Settings.h"  // for Settings
#include "util/PathUtil.h"              // for fromGFile, toGFile
#include "util/PopupWindowWrapper.h"    // for PopupWindowWrapper
#include "util/Util.h"
#include "util/gtk4_helper.h"         // for gtk_file_chooser_set_current_folder
#include "util/i18n.h"                // for _
#include "util/raii/GObjectSPtr.h"    // for GObjectSPtr
#include "util/raii/GtkWindowUPtr.h"  // for GtkWindowUPtr

static void addFilterAllFiles(GtkFileChooser* fc) {
    GtkFileFilter* filterAll = gtk_file_filter_new();
    gtk_file_filter_set_name(filterAll, _("All files"));
    gtk_file_filter_add_pattern(filterAll, "*");
    gtk_file_chooser_add_filter(fc, filterAll);
}

static void addFilterSupported(GtkFileChooser* fc) {
    GtkFileFilter* filterSupported = gtk_file_filter_new();
    gtk_file_filter_set_name(filterSupported, _("Supported files"));
    gtk_file_filter_add_mime_type(filterSupported, "application/x-xojpp");
    gtk_file_filter_add_mime_type(filterSupported, "application/x-xopp");
    gtk_file_filter_add_mime_type(filterSupported, "application/x-xopt");
    gtk_file_filter_add_mime_type(filterSupported, "application/pdf");
    gtk_file_filter_add_pattern(filterSupported, "*.moj");  // MrWriter
    gtk_file_chooser_add_filter(fc, filterSupported);
}

static void addFilterPdf(GtkFileChooser* fc) {
    GtkFileFilter* filterPdf = gtk_file_filter_new();
    gtk_file_filter_set_name(filterPdf, _("PDF files"));
    gtk_file_filter_add_mime_type(filterPdf, "application/pdf");
    gtk_file_chooser_add_filter(fc, filterPdf);
}

static void addFilterXoj(GtkFileChooser* fc) {
    GtkFileFilter* filterXoj = gtk_file_filter_new();
    gtk_file_filter_set_name(filterXoj, _("Xournal files"));
    gtk_file_filter_add_mime_type(filterXoj, "application/x-xojpp");
    gtk_file_chooser_add_filter(fc, filterXoj);
}

static void addFilterXopp(GtkFileChooser* fc) {
    GtkFileFilter* filterXopp = gtk_file_filter_new();
    gtk_file_filter_set_name(filterXopp, _("Xournal++ files"));
    gtk_file_filter_add_mime_type(filterXopp, "application/x-xopp");
    gtk_file_chooser_add_filter(fc, filterXopp);
}

static void addFilterXopt(GtkFileChooser* fc) {
    GtkFileFilter* filterXopt = gtk_file_filter_new();
    gtk_file_filter_set_name(filterXopt, _("Xournal++ template"));
    gtk_file_filter_add_mime_type(filterXopt, "application/x-xopt");
    gtk_file_chooser_add_filter(fc, filterXopt);
}

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

// Helper class, for a single dialog
class FileDlg {
public:
    /**
     * Creates an open file dialog. The callback is only called if a file is actually chosen
     * @param callback(path, attachPdf)
     */
    FileDlg(Settings* settings, std::function<void(fs::path, bool)> callback);
    ~FileDlg() = default;

    inline GtkWindow* getWindow() const { return window.get(); }

private:
    xoj::util::GtkWindowUPtr window;

    Settings* settings{};
    std::function<void(fs::path, bool)> callback;
    gulong signalId{};
};
constexpr auto ATTACH_PDF_CHOICE_ID = "attachPdfChoice";

static GtkWindow* makeWindow(Settings* settings) {
    // Todo(maybe)
    // Restore previews using https://discourse.gnome.org/t/file-chooser-gtk-4-image-preview/11510/2
    auto* win = gtk_file_chooser_dialog_new(_("Open file"), nullptr, GTK_FILE_CHOOSER_ACTION_OPEN, _("_Cancel"),
                                            GTK_RESPONSE_CANCEL, _("_Open"), GTK_RESPONSE_OK, nullptr);
    fs::path currentFolder;
    if (!settings->getLastOpenPath().empty()) {
        currentFolder = settings->getLastOpenPath();
    } else {
        g_warning("lastOpenPath is not set!");
        currentFolder = g_get_home_dir();
    }
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(win), Util::toGFile(currentFolder).get(), nullptr);
    return GTK_WINDOW(win);
}

FileDlg::FileDlg(Settings* settings, std::function<void(fs::path, bool)> callback):
        window(makeWindow(settings)), settings(settings), callback(std::move(callback)) {
    this->signalId = g_signal_connect(
            window.get(), "response", G_CALLBACK(+[](GtkDialog* win, int response, gpointer data) {
                auto* self = static_cast<FileDlg*>(data);

                if (response == GTK_RESPONSE_OK) {
                    auto path =
                            Util::fromGFile(xoj::util::GObjectSPtr<GFile>(
                                                    gtk_file_chooser_get_file(GTK_FILE_CHOOSER(win)), xoj::util::adopt)
                                                    .get());
                    if (!path.empty()) {
                        self->settings->setLastOpenPath(path.parent_path());
                    }

                    bool attachPdf = false;
                    if (const char* choice = gtk_file_chooser_get_choice(GTK_FILE_CHOOSER(win), ATTACH_PDF_CHOICE_ID);
                        choice) {
                        attachPdf = std::strcmp(choice, "true") == 0;
                    }

                    // Let this dialog close before proceeding: if callback() calls on another dialog, then this one
                    // would stay on.
                    Util::execInUiThread([cb = std::move(self->callback), path = std::move(path), attachPdf]() {
                        cb(std::move(path), attachPdf);
                    });
                }

                // Closing the window causes another "response" signal, which we want to ignore
                g_signal_handler_disconnect(win, self->signalId);
                gtk_window_close(reinterpret_cast<GtkWindow*>(win));
            }),
            this);
}

void XojOpenDlg::showOpenTemplateDialog(GtkWindow* parent, Settings* settings, std::function<void(fs::path)> callback) {
    auto popup = xoj::popup::PopupWindowWrapper<FileDlg>(
            settings, [cb = std::move(callback)](fs::path path, bool) { cb(std::move(path)); });

    auto* fc = GTK_FILE_CHOOSER(popup.getPopup()->getWindow());
    addFilterAllFiles(fc);
    addFilterXopt(fc);

    popup.show(parent);
}


void XojOpenDlg::showOpenFileDialog(GtkWindow* parent, Settings* settings, std::function<void(fs::path)> callback) {
    auto popup = xoj::popup::PopupWindowWrapper<FileDlg>(
            settings, [cb = std::move(callback)](fs::path path, bool) { cb(std::move(path)); });

    auto* fc = GTK_FILE_CHOOSER(popup.getPopup()->getWindow());
    addFilterSupported(fc);
    addFilterXoj(fc);
    addFilterXopt(fc);
    addFilterXopp(fc);
    addFilterPdf(fc);
    addFilterAllFiles(fc);

    addlastSavePathShortcut(fc, settings);

    popup.show(parent);
}

void XojOpenDlg::showAnnotatePdfDialog(GtkWindow* parent, Settings* settings,
                                       std::function<void(fs::path, bool)> callback) {
    auto popup = xoj::popup::PopupWindowWrapper<FileDlg>(settings, std::move(callback));

    auto* fc = GTK_FILE_CHOOSER(popup.getPopup()->getWindow());

    addFilterPdf(fc);
    addFilterAllFiles(fc);

    addlastSavePathShortcut(fc, settings);

    gtk_file_chooser_add_choice(fc, ATTACH_PDF_CHOICE_ID, _("Attach file to the journal"), nullptr, nullptr);
    gtk_file_chooser_set_choice(fc, ATTACH_PDF_CHOICE_ID, "false");

    popup.show(parent);
}
