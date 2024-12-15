#include "PageTemplateDialog.h"

#include <cstdio>    // for sprintf
#include <ctime>     // for localtime, strftime, time
#include <fstream>   // for ofstream, basic_ostream
#include <memory>    // for allocator, unique_ptr
#include <optional>  // for optional
#include <string>    // for string, operator<<

#include <gdk/gdk.h>      // for GdkRGBA
#include <glib-object.h>  // for G_CALLBACK, g_signal_c...

#include "control/pagetype/PageTypeHandler.h"  // for PageTypeInfo, PageType...
#include "control/settings/Settings.h"         // for Settings
#include "gui/Builder.h"                       // for Builder
#include "gui/dialog/XojOpenDlg.h"             // for XojOpenDlg
#include "gui/menus/popoverMenus/PageTypeSelectionPopoverGridOnly.h"
#include "gui/toolbarMenubar/ToolMenuHandler.h"
#include "model/FormatDefinitions.h"  // for FormatUnits, XOJ_UNITS
#include "model/PageType.h"           // for PageType
#include "util/Color.h"               // for GdkRGBA_to_argb, rgb_t...
#include "util/FileDialogWrapper.h"   // for FileDialogWrapper
#include "util/MimeTypes.h"
#include "util/PathUtil.h"            // for fromGFilename, readString
#include "util/PopupWindowWrapper.h"  // for PopupWindowWrapper
#include "util/SaveNameUtils.h"
#include "util/XojMsgBox.h"  // for XojMsgBox
#include "util/i18n.h"       // for _

#include "FormatDialog.h"  // for FormatDialog
#include "XojSaveDlg.h"
#include "filesystem.h"  // for path

class GladeSearchpath;

constexpr auto UI_FILE = "pageTemplate.glade";
constexpr auto UI_DIALOG_NAME = "templateDialog";

using namespace xoj::popup;

PageTemplateDialog::PageTemplateDialog(GladeSearchpath* gladeSearchPath, Settings* settings, ToolMenuHandler* toolmenu,
                                       PageTypeHandler* types):
        gladeSearchPath(gladeSearchPath), settings(settings), toolMenuHandler(toolmenu), types(types) {
    model.parse(settings->getPageTemplate());

    Builder builder(gladeSearchPath, UI_FILE);
    window.reset(GTK_WINDOW(builder.get(UI_DIALOG_NAME)));

    // Needs to be initialized after this->window
    pageTypeSelectionMenu = std::make_unique<PageTypeSelectionPopoverGridOnly>(types, settings, this);
    gtk_menu_button_set_popup(GTK_MENU_BUTTON(builder.get("btBackgroundDropdown")),
                              pageTypeSelectionMenu->getPopover());

    pageSizeLabel = GTK_LABEL(builder.get("lbPageSize"));
    backgroundTypeLabel = GTK_LABEL(builder.get("lbBackgroundType"));
    backgroundColorChooser = GTK_COLOR_CHOOSER(builder.get("cbBackgroundButton"));
    copyLastPageButton = GTK_TOGGLE_BUTTON(builder.get("cbCopyLastPage"));
    copyLastPageSizeButton = GTK_TOGGLE_BUTTON(builder.get("cbCopyLastPageSize"));


    g_signal_connect_swapped(builder.get("btChangePaperSize"), "clicked",
                             G_CALLBACK(+[](PageTemplateDialog* self) { self->showPageSizeDialog(); }), this);

    g_signal_connect_swapped(builder.get("btLoad"), "clicked",
                             G_CALLBACK(+[](PageTemplateDialog* self) { self->loadFromFile(); }), this);

    g_signal_connect_swapped(builder.get("btSave"), "clicked",
                             G_CALLBACK(+[](PageTemplateDialog* self) { self->saveToFile(); }), this);

    g_signal_connect_swapped(builder.get("btCancel"), "clicked", G_CALLBACK(gtk_window_close), this->getWindow());
    g_signal_connect_swapped(builder.get("btOk"), "clicked", G_CALLBACK(+[](PageTemplateDialog* self) {
                                 self->saveToModel();
                                 self->settings->setPageTemplate(self->model.toString());
                                 self->toolMenuHandler->setDefaultNewPageType(self->model.getPageInsertType());
                                 self->toolMenuHandler->setDefaultNewPaperSize(
                                         self->model.isCopyLastPageSize() ? std::nullopt :
                                                                            std::optional(PaperSize(self->model)));
                                 gtk_window_close(self->getWindow());
                             }),
                             this);

    updateDataFromModel();
}

PageTemplateDialog::~PageTemplateDialog() = default;

void PageTemplateDialog::updateDataFromModel() {
    GdkRGBA color = Util::rgb_to_GdkRGBA(model.getBackgroundColor());
    gtk_color_chooser_set_rgba(backgroundColorChooser, &color);

    updatePageSize();

    pageTypeSelectionMenu->setSelectedPT(model.getBackgroundType());
    changeCurrentPageBackground(types->getInfoOn(model.getBackgroundType()));

    gtk_toggle_button_set_active(copyLastPageButton, model.isCopyLastPageSettings());
    gtk_toggle_button_set_active(copyLastPageSizeButton, model.isCopyLastPageSize());
}

void PageTemplateDialog::changeCurrentPageBackground(const PageTypeInfo* info) {
    model.setBackgroundType(info->page);

    gtk_label_set_text(backgroundTypeLabel, info->name.c_str());
}

void PageTemplateDialog::saveToModel() {
    model.setCopyLastPageSettings(gtk_toggle_button_get_active(copyLastPageButton));
    model.setCopyLastPageSize(gtk_toggle_button_get_active(copyLastPageSizeButton));

    GdkRGBA color;
    gtk_color_chooser_get_rgba(backgroundColorChooser, &color);
    model.setBackgroundColor(Util::GdkRGBA_to_argb(color));
}


void PageTemplateDialog::loadFromFile() {
    xoj::OpenDlg::showOpenTemplateDialog(this->getWindow(), settings, [this](fs::path path) {
        auto contents = Util::readString(path);
        if (!contents.has_value()) {
            return;
        }
        model.parse(*contents);

        updateDataFromModel();
    });
}
void PageTemplateDialog::saveToFile() {
    saveToModel();

    auto callback = [this](std::optional<fs::path> file) {
        if (file && !file->empty()) {
            settings->setLastSavePath(file->parent_path());

            std::ofstream out{*file};
            out << model.toString();
        }
    };

    time_t curtime = time(nullptr);
    char stime[128];
    strftime(stime, sizeof(stime), "%F-Template-%H-%M.xopt", localtime(&curtime));

    fs::path suggestedPath = settings->getLastSavePath() / std::string(stime);

    auto popup = xoj::popup::FileDialogWrapper<SaveDlg::SaveFileDialog>(
            settings, std::move(suggestedPath), _("Save File"), _("Save"), [](fs::path&, const char*) { return true; },
            std::move(callback));

    auto* fc = GTK_FILE_CHOOSER(popup.getPopup()->getNativeDialog());

    GtkFileFilter* filterXoj = gtk_file_filter_new();
    gtk_file_filter_set_name(filterXoj, _("Xournal++ template"));

    Mime::XOPT.addToFilter(filterXoj);

    gtk_file_chooser_add_filter(fc, filterXoj);

    popup.show(GTK_WINDOW(this->getWindow()));
}

void PageTemplateDialog::updatePageSize() {
    const FormatUnits* formatUnit = &XOJ_UNITS[settings->getSizeUnitIndex()];

    char buffer[64];
    sprintf(buffer, "%0.2lf", model.getPageWidth() / formatUnit->scale);
    std::string pageSize = buffer;
    pageSize += formatUnit->name;
    pageSize += " x ";

    sprintf(buffer, "%0.2lf", model.getPageHeight() / formatUnit->scale);
    pageSize += buffer;
    pageSize += formatUnit->name;

    gtk_label_set_text(pageSizeLabel, pageSize.c_str());
}

void PageTemplateDialog::showPageSizeDialog() {
    auto popup = xoj::popup::PopupWindowWrapper<xoj::popup::FormatDialog>(gladeSearchPath, settings,
                                                                          model.getPageWidth(), model.getPageHeight(),
                                                                          [dlg = this](double width, double height) {
                                                                              dlg->model.setPageWidth(width);
                                                                              dlg->model.setPageHeight(height);

                                                                              dlg->updatePageSize();
                                                                          });
    popup.show(this->getWindow());
}

/**
 * The dialog was confirmed / saved
 */
auto PageTemplateDialog::isSaved() const -> bool { return saved; }
