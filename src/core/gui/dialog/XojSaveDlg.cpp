#include "XojSaveDlg.h"

#include <memory>
#include <optional>
#include <utility>

#include "control/settings/Settings.h"
#include "gui/dialog/FileChooserFiltersHelper.h"
#include "gui/dialog/NativeFileChooserHelper.h"
#include "util/PathUtil.h"            // for fromGFile, toGFile
#include "util/PopupWindowWrapper.h"  // for PopupWindowWrapper
#include "util/Util.h"
#include "util/XojMsgBox.h"
#include "util/gtk4_helper.h"         // for gtk_file_chooser_set_current_folder
#include "util/i18n.h"                // for _
#include "util/raii/GObjectSPtr.h"    // for GObjectSPtr
#include "util/raii/GtkWindowUPtr.h"  // for GtkWindowUPtr

static GtkWindow* makeWindow(Settings* settings, fs::path suggestedPath, const char* windowTitle,
                             const char* buttonLabel) {
    GtkWidget* dialog = gtk_file_chooser_dialog_new(windowTitle, nullptr, GTK_FILE_CHOOSER_ACTION_SAVE, _("_Cancel"),
                                                    GTK_RESPONSE_CANCEL, buttonLabel, GTK_RESPONSE_OK, nullptr);

    if (!suggestedPath.empty()) {
        gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), Util::toGFile(suggestedPath.parent_path()).get(),
                                            nullptr);
        gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog),
                                          Util::toGFilename(suggestedPath.filename()).c_str());
    }
    if (settings) {
        gtk_file_chooser_add_shortcut_folder(GTK_FILE_CHOOSER(dialog), Util::toGFile(settings->getLastOpenPath()).get(),
                                             nullptr);
    }

    return GTK_WINDOW(dialog);
}

static void setSuggestedSavePath(GtkFileChooser* fc, const fs::path& suggestedPath) {
    if (!suggestedPath.empty() && suggestedPath.has_parent_path()) {
        gtk_file_chooser_set_current_folder(fc, Util::toGFile(suggestedPath.parent_path()).get(), nullptr);
    }
    if (!suggestedPath.empty()) {
        gtk_file_chooser_set_current_name(fc, Util::toGFilename(suggestedPath.filename()).c_str());
    }
}

xoj::SaveExportDialog::SaveExportDialog(Settings* settings, fs::path suggestedPath, const char* windowTitle,
                                        const char* buttonLabel,
                                        std::function<bool(fs::path&, const char* filterName)> pathValidation,
                                        std::function<void(std::optional<fs::path>)> callback):
        window(makeWindow(settings, std::move(suggestedPath), windowTitle, buttonLabel)),
        callback(std::move(callback)),
        pathValidation(std::move(pathValidation)) {
    this->signalId = g_signal_connect(
            window.get(), "response", G_CALLBACK(+[](GtkDialog* win, int response, gpointer data) {
                auto* self = static_cast<SaveExportDialog*>(data);
                auto* fc = GTK_FILE_CHOOSER(win);
                if (response == GTK_RESPONSE_OK) {
                    auto file = Util::fromGFile(
                            xoj::util::GObjectSPtr<GFile>(gtk_file_chooser_get_file(fc), xoj::util::adopt).get());

                    if (self->pathValidation(file, gtk_file_filter_get_name(gtk_file_chooser_get_filter(fc)))) {
                        XojMsgBox::replaceFileQuestion(
                                GTK_WINDOW(win), std::move(file),
                                std::bind(&SaveExportDialog::close, self, std::placeholders::_1));
                    }  // else the dialog stays on until a suitable destination is found or cancel is hit.
                } else {
                    self->close(std::nullopt);
                }
            }),
            this);
}

void xoj::SaveExportDialog::close(std::optional<fs::path> path) {
    // We need to call gtk_window_close() before invoking the callback, because if the callback pops up another dialog,
    // the first one won't close...
    // But since gtk_window_close() triggers the destruction of *this, we first move the callback
    auto cb = std::move(this->callback);

    // Closing the window causes another "response" signal, which we want to ignore
    g_signal_handler_disconnect(window.get(), signalId);
    gtk_window_close(window.get());  // Destroys *this. Don't do anything after this call

    cb(std::move(path));
}

static bool xoppPathValidation(fs::path& p, const char*) {
    Util::clearExtensions(p);
    p += ".xopp";
    return true;
}

void xoj::SaveExportDialog::showSaveDialog(GtkWindow* parent, Settings* settings, fs::path suggestedPath,
                                           const char* windowTitle, const char* buttonLabel,
                                           ConfigureCallback configure, PathValidationCallback pathValidation,
                                           FileSelectedCallback callback) {
    if (xoj::NativeFileChooser::isAvailable()) {
        auto showDialog = std::make_shared<std::function<void()>>();
        std::weak_ptr<std::function<void()>> weakShowDialog = showDialog;
        auto currentSuggestedPath = std::make_shared<fs::path>(std::move(suggestedPath));
        *showDialog = [parent, currentSuggestedPath, windowTitle, buttonLabel, configure, pathValidation, callback,
                       weakShowDialog]() {
            auto showDialog = weakShowDialog.lock();
            if (!showDialog) {
                return;
            }
            xoj::NativeFileChooser::show(
                    parent, windowTitle, GTK_FILE_CHOOSER_ACTION_SAVE, buttonLabel,
                    [currentSuggestedPath, configure](GtkFileChooser* fc) {
                        setSuggestedSavePath(fc, *currentSuggestedPath);
                        configure(fc);
                    },
                    [parent, currentSuggestedPath, pathValidation, callback, showDialog](
                            std::optional<fs::path> path, std::optional<std::string> filterName) {
                        if (!path || path->empty()) {
                            callback(std::nullopt);
                            return;
                        }

                        auto file = std::move(*path);
                        const char* filter = filterName ? filterName->c_str() : nullptr;
                        if (!pathValidation(file, filter)) {
                            *currentSuggestedPath = file;
                            (*showDialog)();
                            return;
                        }

                        *currentSuggestedPath = file;
                        XojMsgBox::replaceFileQuestion(
                                parent, std::move(file), [callback](const fs::path& path) { callback(path); },
                                [showDialog] { (*showDialog)(); });
                    });
        };
        (*showDialog)();
        return;
    }

    auto popup = xoj::popup::PopupWindowWrapper<SaveExportDialog>(settings, std::move(suggestedPath), windowTitle,
                                                                  buttonLabel, std::move(pathValidation),
                                                                  std::move(callback));

    auto* fc = GTK_FILE_CHOOSER(popup.getPopup()->getWindow());
    configure(fc);

    popup.show(parent);
}

void xoj::SaveExportDialog::showSaveFileDialog(GtkWindow* parent, Settings* settings, fs::path suggestedPath,
                                               std::function<void(std::optional<fs::path>)> callback) {
    showSaveDialog(
            parent, settings, std::move(suggestedPath), _("Save File"), _("Save"),
            [](GtkFileChooser* fc) {
                if (xoj::NativeFileChooser::isAvailable()) {
                    xoj::addFilterXoppByExtension(fc);
                } else {
                    xoj::addFilterXopp(fc);
                }
            },
            xoppPathValidation, std::move(callback));
}
