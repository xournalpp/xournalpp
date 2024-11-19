#include "XojSaveDlg.h"

#include <future>
#include <optional>

#include "control/settings/Settings.h"
#include "util/FileDialogWrapper.h"  // for FileDialogWrapper
#include "util/PathUtil.h"           // for fromGFile, toGFile
#include "util/Util.h"
#include "util/XojMsgBox.h"
#include "util/gtk4_helper.h"         // for gtk_file_chooser_set_current_folder
#include "util/i18n.h"                // for _
#include "util/raii/GObjectSPtr.h"    // for GObjectSPtr
#include "util/raii/GtkWindowUPtr.h"  // for GtkWindowUPtr

#include "FileChooserFiltersHelper.h"


static auto makeSaveFileChooserNative(Settings* settings, fs::path suggestedPath, const char* windowTitle,
                                      const char* buttonLabel) {
    GtkFileChooserNative* native =
            gtk_file_chooser_native_new(windowTitle, nullptr, GTK_FILE_CHOOSER_ACTION_SAVE, buttonLabel, _("_Cancel"));

    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(native), Util::toGFile(suggestedPath.parent_path()).get(),
                                        nullptr);
    gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(native), Util::toGFilename(suggestedPath.filename()).c_str());
    gtk_file_chooser_add_shortcut_folder(GTK_FILE_CHOOSER(native), Util::toGFile(settings->getLastOpenPath()).get(),
                                         nullptr);

    gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(native), false);

    return xoj::util::GObjectSPtr<GtkNativeDialog>(GTK_NATIVE_DIALOG(native), xoj::util::adopt);
}

xoj::SaveDlg::SaveFileDialog::SaveFileDialog(Settings* settings, fs::path suggestedPath, const char* windowTitle,
                                             const char* buttonLabel,
                                             std::function<bool(fs::path&, const char* filterName)> pathValidation,
                                             std::function<void(std::optional<fs::path>)> callback):
        fileChooserNative(makeSaveFileChooserNative(settings, std::move(suggestedPath), windowTitle, buttonLabel)),
        callback(std::move(callback)),
        pathValidation(std::move(pathValidation)) {}

bool xoj::SaveDlg::SaveFileDialog::onAccept() {
    auto* fc = GTK_FILE_CHOOSER(getNativeDialog());
    auto file = Util::fromGFile(xoj::util::GObjectSPtr<GFile>(gtk_file_chooser_get_file(fc), xoj::util::adopt).get());

    if (pathValidation(file, gtk_file_filter_get_name(gtk_file_chooser_get_filter(fc)))) {
        XojMsgBox::replaceFileQuestion(
                nullptr, std::move(file),
                [cb = std::move(callback)](auto&& file) {
                    cb(std::forward<decltype(file)>(file));
                },
                [](auto&& file) {
                    // not implmented should return false
                });
    }

    return true;
}

void xoj::SaveDlg::SaveFileDialog::onCancel() const { callback(std::nullopt); }

static bool xoppPathValidation(fs::path& p, const char*) {
    Util::clearExtensions(p);
    p += ".xopp";
    return true;
}

void xoj::SaveDlg::showSaveFileDialog(GtkWindow* parent, Settings* settings, fs::path suggestedPath,
                                      std::function<void(std::optional<fs::path>)> callback) {
    auto popup = xoj::popup::FileDialogWrapper<SaveFileDialog>(settings, std::move(suggestedPath), _("Save File"),
                                                               _("Save"), xoppPathValidation, std::move(callback));

    auto* fc = GTK_FILE_CHOOSER(popup.getPopup()->getNativeDialog());
    xoj::addFilterXopp(fc);

    popup.show(parent);
}
