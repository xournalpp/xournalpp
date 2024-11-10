#include "XojSaveDlg.h"

#include <optional>

#include "control/settings/Settings.h"
#include "util/PathUtil.h"            // for fromGFile, toGFile
#include "util/PopupWindowWrapper.h"  // for PopupWindowWrapper
#include "util/Util.h"
#include "util/XojMsgBox.h"
#include "util/gtk4_helper.h"         // for gtk_file_chooser_set_current_folder
#include "util/i18n.h"                // for _
#include "util/raii/GObjectSPtr.h"    // for GObjectSPtr
#include "util/raii/GtkWindowUPtr.h"  // for GtkWindowUPtr

#include "FileChooserFiltersHelper.h"

static GtkFileChooserNative* makeFileChooserNative(GtkWindow* parent, Settings* settings, fs::path suggestedPath, const char* windowTitle,
                                                   const char* buttonLabel) {
    GtkFileChooserNative* native =
            gtk_file_chooser_native_new(windowTitle, parent, GTK_FILE_CHOOSER_ACTION_SAVE, buttonLabel, _("_Cancel"));

    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(native), Util::toGFile(suggestedPath.parent_path()).get(),
                                        nullptr);
    gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(native), Util::toGFilename(suggestedPath.filename()).c_str());
    gtk_file_chooser_add_shortcut_folder(GTK_FILE_CHOOSER(native), Util::toGFile(settings->getLastOpenPath()).get(),
                                         nullptr);

    return native;
}

xoj::SaveExportDialog::SaveExportDialog(GtkWindow* parent, Settings* settings, fs::path suggestedPath, const char* windowTitle,
                                        const char* buttonLabel,
                                        std::function<bool(fs::path&, const char* filterName)> pathValidation,
                                        std::function<void(std::optional<fs::path>)> callback):
        fileChooserNative(makeFileChooserNative(parent, settings, std::move(suggestedPath), windowTitle, buttonLabel)),
        callback(std::move(callback)),
        pathValidation(std::move(pathValidation)) {
    this->signalId = g_signal_connect(
            fileChooserNative.get(), "response", G_CALLBACK(+[](GtkNativeDialog* win, int response, gpointer data) {
                auto* self = static_cast<SaveExportDialog*>(data);
                auto* fc = GTK_FILE_CHOOSER(win);
                if (response == GTK_RESPONSE_ACCEPT) {
                    auto file = Util::fromGFile(
                            xoj::util::GObjectSPtr<GFile>(gtk_file_chooser_get_file(fc), xoj::util::adopt).get());

                    self->pathValidation(file, gtk_file_filter_get_name(gtk_file_chooser_get_filter(fc)));
                    
                    self->close(file);
                } else {
                    self->close(std::nullopt);
                }
            }),
            this);
}

void xoj::SaveExportDialog::close(std::optional<fs::path> path) {
    auto cb = std::move(this->callback);

    // Closing the window causes another "response" signal, which we want to ignore
    g_signal_handler_disconnect(fileChooserNative.get(), signalId);
    g_object_unref(fileChooserNative.get());

    cb(std::move(path));
}

static bool xoppPathValidation(fs::path& p, const char*) {
    Util::clearExtensions(p);
    p += ".xopp";
    return true;
}

void xoj::SaveExportDialog::showSaveFileDialog(GtkWindow* parent, Settings* settings, fs::path suggestedPath,
                                               std::function<void(std::optional<fs::path>)> callback) {
    auto popup = xoj::popup::PopupWindowWrapper<SaveExportDialog>(parent, settings, std::move(suggestedPath), _("Save File"),
                                                                  _("Save"), xoppPathValidation, std::move(callback));

    auto* fc = GTK_FILE_CHOOSER(popup.getPopup()->getWindow());
    xoj::addFilterXopp(fc);

    popup.show(parent);
}
