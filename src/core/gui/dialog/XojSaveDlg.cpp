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

static GtkNativeDialog* makeWindow(Settings* settings, fs::path suggestedPath, const char* windowTitle,
                             const char* buttonLabel) {
    auto* dialog = gtk_file_chooser_native_new(windowTitle, nullptr, GTK_FILE_CHOOSER_ACTION_SAVE, buttonLabel, _("_Cancel"));

#if GTK_MAJOR_VERSION == 3
    // On GTK4, this is enabled by default and can no longer be configured.
    gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), true);
#endif

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

    return GTK_NATIVE_DIALOG(dialog);
}

xoj::SaveExportDialog::SaveExportDialog(Settings* settings, fs::path suggestedPath, const char* windowTitle,
                                        const char* buttonLabel,
                                        xoj::util::move_only_function<void(std::optional<fs::path>)> callback):
        window(makeWindow(settings, std::move(suggestedPath), windowTitle, buttonLabel)),
        callback(std::move(callback)) {
    this->signalId = g_signal_connect(
            window.get(), "response", G_CALLBACK(+[](GtkDialog* win, int response, gpointer data) {
                auto* self = static_cast<SaveExportDialog*>(data);
                auto* fc = GTK_FILE_CHOOSER(win);
                if (response == GTK_RESPONSE_ACCEPT) {
                    auto file = Util::fromGFile(
                            xoj::util::GObjectSPtr<GFile>(gtk_file_chooser_get_file(fc), xoj::util::adopt).get());
                    self->callback(file);
                } else {
                    self->callback(std::nullopt);
                }
                self->window.reset();  // Dropping the ref will destroy it all
                delete self;
            }),
            this);
}

xoj::SaveExportDialog::SaveExportDialog(Settings* settings, fs::path suggestedPath, const char* windowTitle,
                                        const char* buttonLabel, const FileType& filetype,
                                        xoj::util::move_only_function<void(std::optional<fs::path>)> callback):
        SaveExportDialog(settings, std::move(suggestedPath), windowTitle, buttonLabel, std::move(callback)) {
    xoj::addFilterForFile(getFileChooser(), filetype);
}

void xoj::SaveExportDialog::showSaveFileDialog(GtkWindow* parent, Settings* settings, fs::path suggestedPath,
                                               xoj::util::move_only_function<void(std::optional<fs::path>)> callback) {
    auto popup = xoj::popup::PopupWindowWrapper<SaveExportDialog>(settings, std::move(suggestedPath), _("Save File"),
                                                                  _("Save"), xoj::FileTypes::XOPP, std::move(callback));
    popup.showNative(parent);
}

void xoj::SaveExportDialog::showExportFileDialog(
        GtkWindow* parent, Settings* settings, fs::path suggestedPath, const FileType& filetype,
        xoj::util::move_only_function<void(std::optional<fs::path>)> callback) {
    auto popup = xoj::popup::PopupWindowWrapper<SaveExportDialog>(settings, std::move(suggestedPath), _("Export File"),
                                                                  _("Export"), filetype, std::move(callback));
    popup.showNative(parent);
}
