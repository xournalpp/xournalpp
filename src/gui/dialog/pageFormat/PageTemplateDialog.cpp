#include "PageTemplateDialog.h"

#include <fstream>
#include <sstream>

#include "control/pagetype/PageTypeHandler.h"
#include "control/settings/Settings.h"
#include "control/stockdlg/XojOpenDlg.h"
#include "gui/dialog/pageFormat/PageSizeDialog.h"
#include "gui/widgets/PopupMenuButton.h"
#include "model/FormatDefinitions.h"

#include "PathUtil.h"
#include "Util.h"
#include "filesystem.h"
#include "i18n.h"
using std::ofstream;

class PageTemplateDialog::Impl {
public:
    Impl(Settings* settings, PageTypeHandler* handler, PageTemplateDialog* publicApi);

    void onBackgroundChanged(PageTypeInfo* info);

    /// Called just before the dialog is shown.
    void preShow();

    void showPageSizeDialog();
    void updatePageSize();

    void saveToFile();
    void loadFromFile();

    void updateDataFromModel();
    void saveToModel();

    bool isSaved() const;
    void onSubmit();

private:
    Settings* const settings_;

    PageTemplateDialog* const pubApi_;
    PageTemplateSettings model_;

    std::unique_ptr<PageTypeMenu> pageMenu_;
    std::unique_ptr<PopupMenuButton> popupMenuButton_;

    // True iff the dialog has been confirmed/saved
    bool saved_ = false;
};

PageTemplateDialog::PageTemplateDialog(GladeSearchpath* gladeSearchPath, Settings* settings, PageTypeHandler* types):
        GladeGui(gladeSearchPath, "pageTemplateDialog.glade", "templateDialog"),
        impl_(std::make_unique<PageTemplateDialog::Impl>(settings, types, this)) {}

PageTemplateDialog::~PageTemplateDialog() = default;

void PageTemplateDialog::changeCurrentPageBackground(PageTypeInfo* info) { impl_->onBackgroundChanged(info); }

auto PageTemplateDialog::isSaved() const -> bool { return impl_->isSaved(); }

void PageTemplateDialog::show(GtkWindow* parent) {
    g_assert(!impl_->isSaved());

    impl_->preShow();

    gtk_window_set_transient_for(GTK_WINDOW(this->window), parent);
    int ret = gtk_dialog_run(GTK_DIALOG(this->window));

    // OK
    if (ret == 1) {
        impl_->onSubmit();
    }

    gtk_widget_hide(this->window);
}

void PageTemplateDialog::setActionDescription(const std::string& description) {
    gtk_label_set_text(GTK_LABEL(get("lbSettingsTarget")), description.c_str());
}

void PageTemplateDialog::setNewPageOnlySettingsVisible(bool visible) {
    gtk_widget_set_visible(get("cbCopyLastPage"), visible);
    gtk_widget_set_visible(get("cbCopyLastPageSize"), visible);
}

PageTemplateDialog::Impl::Impl(Settings* settings, PageTypeHandler* handler, PageTemplateDialog* publicApi):
        settings_(settings),
        pubApi_(publicApi),
        pageMenu_(std::make_unique<PageTypeMenu>(handler, settings, true, false)),
        popupMenuButton_(
                std::make_unique<PopupMenuButton>(pubApi_->get("btBackgroundDropdown"), pageMenu_->getMenu())) {
    pageMenu_->setListener(pubApi_);

    g_signal_connect(pubApi_->get("btChangePaperSize"), "clicked",
                     G_CALLBACK(+[](GtkToggleButton* togglebutton, PageTemplateDialog::Impl* self) {
                         self->showPageSizeDialog();
                     }),
                     this);

    g_signal_connect(
            pubApi_->get("btLoad"), "clicked",
            G_CALLBACK(+[](GtkToggleButton* togglebutton, PageTemplateDialog::Impl* self) { self->loadFromFile(); }),
            this);

    g_signal_connect(
            pubApi_->get("btSave"), "clicked",
            G_CALLBACK(+[](GtkToggleButton* togglebutton, PageTemplateDialog::Impl* self) { self->saveToFile(); }),
            this);
}

void PageTemplateDialog::Impl::preShow() {
    // Initialize and configure the UI.
    pubApi_->initModel(model_);
    updateDataFromModel();
    pubApi_->configureUI();
}

void PageTemplateDialog::Impl::updateDataFromModel() {
    GdkRGBA color = Util::rgb_to_GdkRGBA(model_.getBackgroundColor());
    gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(pubApi_->get("cbBackgroundButton")), &color);

    updatePageSize();

    pageMenu_->setSelected(model_.getBackgroundType());

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pubApi_->get("cbCopyLastPage")), model_.isCopyLastPageSettings());
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pubApi_->get("cbCopyLastPageSize")), model_.isCopyLastPageSize());
}

void PageTemplateDialog::Impl::onBackgroundChanged(PageTypeInfo* info) {
    model_.setBackgroundType(info->page);

    gtk_label_set_text(GTK_LABEL(pubApi_->get("lbBackgroundType")), info->name.c_str());
}

void PageTemplateDialog::Impl::saveToModel() {
    model_.setCopyLastPageSettings(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(pubApi_->get("cbCopyLastPage"))));
    model_.setCopyLastPageSize(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(pubApi_->get("cbCopyLastPageSize"))));

    GdkRGBA color;
    gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(pubApi_->get("cbBackgroundButton")), &color);
    model_.setBackgroundColor(Util::GdkRGBA_to_argb(color));
}

void PageTemplateDialog::Impl::saveToFile() {
    saveToModel();

    GtkWidget* dialog =
            gtk_file_chooser_dialog_new(_("Save File"), GTK_WINDOW(pubApi_->getWindow()), GTK_FILE_CHOOSER_ACTION_SAVE,
                                        _("_Cancel"), GTK_RESPONSE_CANCEL, _("_Save"), GTK_RESPONSE_OK, nullptr);

    gtk_file_chooser_set_local_only(GTK_FILE_CHOOSER(dialog), true);

    GtkFileFilter* filterXoj = gtk_file_filter_new();
    gtk_file_filter_set_name(filterXoj, _("Xournal++ template"));
    gtk_file_filter_add_pattern(filterXoj, "*.xopt");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filterXoj);

    if (!settings_->getLastSavePath().empty()) {
        gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog),
                                            Util::toGFilename(settings_->getLastSavePath()).c_str());
    }

    time_t curtime = time(nullptr);
    char stime[128];
    strftime(stime, sizeof(stime), "%F-Template-%H-%M.xopt", localtime(&curtime));
    std::string saveFilename = stime;

    gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), saveFilename.c_str());
    gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), true);

    gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(pubApi_->getWindow()));
    if (gtk_dialog_run(GTK_DIALOG(dialog)) != GTK_RESPONSE_OK) {
        gtk_widget_destroy(dialog);
        return;
    }

    auto filepath = Util::fromGFilename(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog)));
    settings_->setLastSavePath(filepath.parent_path());
    gtk_widget_destroy(dialog);

    std::ofstream out{filepath};
    out << model_.toString();
}

void PageTemplateDialog::Impl::loadFromFile() {
    XojOpenDlg dlg(GTK_WINDOW(pubApi_->getWindow()), settings_);
    fs::path file = dlg.showOpenTemplateDialog();

    auto contents = Util::readString(file);
    if (!contents.has_value()) {
        return;
    }
    model_.parse(*contents);

    updateDataFromModel();
}

void PageTemplateDialog::Impl::updatePageSize() {
    const FormatUnits* formatUnit = &XOJ_UNITS[settings_->getSizeUnitIndex()];

    char buffer[64];
    sprintf(buffer, "%0.2lf", model_.getPageWidth() / formatUnit->scale);
    std::string pageSize = buffer;
    pageSize += formatUnit->name;
    pageSize += " x ";

    sprintf(buffer, "%0.2lf", model_.getPageHeight() / formatUnit->scale);
    pageSize += buffer;
    pageSize += formatUnit->name;

    gtk_label_set_text(GTK_LABEL(pubApi_->get("lbPageSize")), pageSize.c_str());
}

void PageTemplateDialog::Impl::showPageSizeDialog() {
    auto dlg = std::make_unique<PageSizeDialog>(pubApi_->getGladeSearchPath(), settings_, model_.getPageWidth(),
                                                model_.getPageHeight());
    dlg->show(GTK_WINDOW(pubApi_->window));

    const double width = dlg->getWidth();
    const double height = dlg->getHeight();

    if (width > 0) {
        model_.setPageWidth(width);
        model_.setPageHeight(height);

        updatePageSize();
    }
}

auto PageTemplateDialog::Impl::isSaved() const -> bool { return saved_; }

void PageTemplateDialog::Impl::onSubmit() {
    saveToModel();
    pubApi_->save(model_);
    saved_ = true;
}
