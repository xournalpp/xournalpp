#include "XojSaveDlg.h"

#include <optional>

#include "control/settings/Settings.h"
#include "util/PathUtil.h"  // for fromGFile, toGFile
#include "util/Util.h"
#include "util/XojMsgBox.h"
#include "util/gtk4_helper.h"         // for gtk_file_chooser_set_current_folder
#include "util/i18n.h"                // for _
#include "util/raii/GObjectSPtr.h"    // for GObjectSPtr
#include "util/raii/GtkWindowUPtr.h"  // for GtkWindowUPtr

#include "FileChooserFiltersHelper.h"
#include "FileDialogWrapper.h"  // for FileDialogWrapper

static bool xoppPathValidation(fs::path& p, const char*) {
    Util::clearExtensions(p);
    p += ".xopp";
    return true;
}

void xoj::SaveDlg::showSaveFileDialog(GtkWindow* parent, Settings* settings, fs::path suggestedPath,
                                               std::function<void(std::optional<fs::path>)> callback) {
    auto* popup = new xoj::popup::FileDialogWrapper(settings, std::move(suggestedPath), _("Save File"),
                                                                  _("Save"), xoppPathValidation, std::move(callback));

    auto* fc = GTK_FILE_CHOOSER(popup->getNativeDialog());
    xoj::addFilterXopp(fc);

    popup->show(parent);
}
