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
#include "util/PathUtil.h"            // for fromGFilename, readString
#include "util/PopupWindowWrapper.h"  // for PopupWindowWrapper
#include "util/XojMsgBox.h"           // for XojMsgBox
#include "util/i18n.h"                // for _

#include "FormatDialog.h"  // for FormatDialog
#include "filesystem.h"    // for path

class GladeSearchpath;

constexpr auto UI_FILE = "pageTemplate.ui";
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
    gtk_menu_button_set_popover(GTK_MENU_BUTTON(builder.get("btBackgroundDropdown")),
                                pageTypeSelectionMenu->getPopover());

    pageSizeLabel = GTK_LABEL(builder.get("lbPageSize"));
    backgroundTypeLabel = GTK_LABEL(builder.get("lbBackgroundType"));
    backgroundColorChooser = GTK_COLOR_CHOOSER(builder.get("cbBackgroundButton"));
    copyLastPageButton = GTK_CHECK_BUTTON(builder.get("cbCopyLastPage"));
    copyLastPageSizeButton = GTK_CHECK_BUTTON(builder.get("cbCopyLastPageSize"));


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

    pageTypeSelectionMenu->setSelected(model.getBackgroundType());
    changeCurrentPageBackground(types->getInfoOn(model.getBackgroundType()));

    gtk_check_button_set_active(copyLastPageButton, model.isCopyLastPageSettings());
    gtk_check_button_set_active(copyLastPageSizeButton, model.isCopyLastPageSize());
}

void PageTemplateDialog::changeCurrentPageBackground(const PageTypeInfo* info) {
    model.setBackgroundType(info->page);

    gtk_label_set_text(backgroundTypeLabel, info->name.c_str());
}

void PageTemplateDialog::saveToModel() {
    model.setCopyLastPageSettings(gtk_check_button_get_active(copyLastPageButton));
    model.setCopyLastPageSize(gtk_check_button_get_active(copyLastPageSizeButton));

    GdkRGBA color;
    gtk_color_chooser_get_rgba(backgroundColorChooser, &color);
    model.setBackgroundColor(Util::GdkRGBA_to_argb(color));
}

void PageTemplateDialog::saveToFile() {
    saveToModel();

    GtkWidget* dialog =
            gtk_file_chooser_dialog_new(_("Save File"), this->getWindow(), GTK_FILE_CHOOSER_ACTION_SAVE, _("_Cancel"),
                                        GTK_RESPONSE_CANCEL, _("_Save"), GTK_RESPONSE_OK, nullptr);

    GtkFileFilter* filterXoj = gtk_file_filter_new();
    gtk_file_filter_set_name(filterXoj, _("Xournal++ template"));
    gtk_file_filter_add_mime_type(filterXoj, "application/x-xopt");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filterXoj);

    if (!settings->getLastSavePath().empty()) {
        gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), Util::toGFile(settings->getLastSavePath()).get(),
                                            nullptr);
    }

    time_t curtime = time(nullptr);
    char stime[128];
    strftime(stime, sizeof(stime), "%F-Template-%H-%M.xopt", localtime(&curtime));
    std::string saveFilename = stime;

    gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), saveFilename.c_str());

    class FileDlg final {
    public:
        FileDlg(GtkDialog* dialog, PageTemplateDialog* parent): window(GTK_WINDOW(dialog)), parent(parent) {
            this->signalId = g_signal_connect(
                    dialog, "response", G_CALLBACK((+[](GtkDialog* dialog, int response, gpointer data) {
                        FileDlg* self = static_cast<FileDlg*>(data);
                        if (response == GTK_RESPONSE_OK) {
                            auto file = Util::fromGFile(
                                    xoj::util::GObjectSPtr<GFile>(gtk_file_chooser_get_file(GTK_FILE_CHOOSER(dialog)),
                                                                  xoj::util::adopt)
                                            .get());

                            auto saveTemplate = [self, dialog](const fs::path& file) {
                                // Closing the window causes another "response" signal, which we want to ignore
                                g_signal_handler_disconnect(dialog, self->signalId);
                                gtk_window_close(GTK_WINDOW(dialog));
                                self->parent->settings->setLastSavePath(file.parent_path());

                                std::ofstream out{file};
                                out << self->parent->model.toString();
                            };
                            XojMsgBox::replaceFileQuestion(GTK_WINDOW(dialog), std::move(file),
                                                           std::move(saveTemplate));
                        } else {
                            // Closing the window causes another "response" signal, which we want to ignore
                            g_signal_handler_disconnect(dialog, self->signalId);
                            gtk_window_close(GTK_WINDOW(dialog));  // Deletes self, don't do anything after this
                        }
                    })),
                    this);
        }
        ~FileDlg() = default;

        inline GtkWindow* getWindow() const { return window.get(); }

    private:
        xoj::util::GtkWindowUPtr window;
        PageTemplateDialog* parent;
        gulong signalId;
    };

    auto popup = xoj::popup::PopupWindowWrapper<FileDlg>(GTK_DIALOG(dialog), this);
    popup.show(GTK_WINDOW(this->getWindow()));
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
