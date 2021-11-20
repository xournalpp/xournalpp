#include "PageTemplateDialog.h"

#include <fstream>
#include <memory>
#include <sstream>

#include "control/pagetype/PageTypeHandler.h"
#include "control/stockdlg/XojOpenDlg.h"
#include "gui/widgets/PopupMenuButton.h"
#include "model/FormatDefinitions.h"
#include "util/PathUtil.h"
#include "util/Util.h"
#include "util/i18n.h"

#include "FormatDialog.h"
#include "GtkDialogUtil.h"
#include "filesystem.h"
using std::ofstream;

PageTemplateDialog::PageTemplateDialog(GladeSearchpath* gladeSearchPath, Settings* settings, PageTypeHandler* types):
        GladeGui(gladeSearchPath, "pageTemplate.glade", "templateDialog"),
        settings(settings),
        pageMenu(new PageTypeMenu(types, settings, true, false)),
        popupMenuButton(new PopupMenuButton(get("btBackgroundDropdown"),
                                            gtk_popover_menu_new_from_model(G_MENU_MODEL(pageMenu->getMenu())))) {
    model.parse(settings->getPageTemplate());

    pageMenu->setListener(this);

    g_signal_connect(
            get("btChangePaperSize"), "clicked",
            G_CALLBACK(+[](GtkToggleButton* togglebutton, PageTemplateDialog* self) { self->showPageSizeDialog(); }),
            this);

    g_signal_connect(get("btLoad"), "clicked",
                     G_CALLBACK(+[](GtkToggleButton* togglebutton, PageTemplateDialog* self) { self->loadFromFile(); }),
                     this);

    g_signal_connect(get("btSave"), "clicked",
                     G_CALLBACK(+[](GtkToggleButton* togglebutton, PageTemplateDialog* self) { self->saveToFile(); }),
                     this);

    updateDataFromModel();
}

PageTemplateDialog::~PageTemplateDialog() = default;

void PageTemplateDialog::updateDataFromModel() {
    GdkRGBA color = Util::rgb_to_GdkRGBA(model.getBackgroundColor());
    gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(get("cbBackgroundButton")), &color);

    updatePageSize();

    pageMenu->setSelected(model.getBackgroundType());

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(get("cbCopyLastPage")), model.isCopyLastPageSettings());
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(get("cbCopyLastPageSize")), model.isCopyLastPageSize());
}

void PageTemplateDialog::changeCurrentPageBackground(PageTypeInfo* info) {
    model.setBackgroundType(info->page);

    gtk_label_set_text(GTK_LABEL(get("lbBackgroundType")), info->name.c_str());
}

void PageTemplateDialog::saveToModel() {
    model.setCopyLastPageSettings(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(get("cbCopyLastPage"))));
    model.setCopyLastPageSize(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(get("cbCopyLastPageSize"))));

    GdkRGBA color;
    gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(get("cbBackgroundButton")), &color);
    model.setBackgroundColor(Util::GdkRGBA_to_argb(color));
}

void PageTemplateDialog::saveToFile() {
    saveToModel();

    GtkWidget* dialog =
            gtk_file_chooser_dialog_new(_("Save File"), GTK_WINDOW(this->getWindow()), GTK_FILE_CHOOSER_ACTION_SAVE,
                                        _("_Cancel"), GTK_RESPONSE_CANCEL, _("_Save"), GTK_RESPONSE_OK, nullptr);


    GtkFileFilter* filterXoj = gtk_file_filter_new();
    gtk_file_filter_set_name(filterXoj, _("Xournal++ template"));
    gtk_file_filter_add_pattern(filterXoj, "*.xopt");
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

    gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(this->getWindow()));
    if (wait_for_gtk_dialog_result(GTK_DIALOG(dialog)) != GTK_RESPONSE_OK) {
        gtk_window_destroy(GTK_WINDOW(dialog));
        return;
    }

    auto filepath = Util::fromGFile(Util::GOwned<GFile>(gtk_file_chooser_get_file(GTK_FILE_CHOOSER(dialog))).get());
    settings->setLastSavePath(filepath.parent_path());
    gtk_window_destroy(GTK_WINDOW(dialog));

    std::ofstream out{filepath};
    out << model.toString();
}

void PageTemplateDialog::loadFromFile() {
    XojOpenDlg dlg(GTK_WINDOW(this->getWindow()), this->settings);
    fs::path file = dlg.showOpenTemplateDialog();

    auto contents = Util::readString(file);
    if (!contents.has_value()) {
        return;
    }
    model.parse(*contents);

    updateDataFromModel();
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

    gtk_label_set_text(GTK_LABEL(get("lbPageSize")), pageSize.c_str());
}

void PageTemplateDialog::showPageSizeDialog() {
    auto dlg =
            std::make_unique<FormatDialog>(getGladeSearchPath(), settings, model.getPageWidth(), model.getPageHeight());
    dlg->show(GTK_WINDOW(this->window));

    const double width = dlg->getWidth();
    const double height = dlg->getHeight();

    if (width > 0) {
        model.setPageWidth(width);
        model.setPageHeight(height);

        updatePageSize();
    }
}

/**
 * The dialog was confirmed / saved
 */
auto PageTemplateDialog::isSaved() const -> bool { return saved; }

void PageTemplateDialog::show(GtkWindow* parent) {
    gtk_window_set_transient_for(GTK_WINDOW(this->window), parent);
    int ret = wait_for_gtk_dialog_result(GTK_DIALOG(this->window));

    if (ret == 1)  // OK
    {
        saveToModel();
        settings->setPageTemplate(model.toString());

        this->saved = true;
    }

    gtk_widget_hide(this->window);
}
