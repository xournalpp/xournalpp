#include "NewPageTemplateDialog.h"

#include "control/settings/Settings.h"

#include "i18n.h"

NewPageTemplateDialog::NewPageTemplateDialog(GladeSearchpath* searchpath, Settings* settings,
                                             PageTypeHandler* typeHandler):
        PageTemplateDialog(searchpath, settings, typeHandler), settings_(settings) {}

NewPageTemplateDialog::~NewPageTemplateDialog() = default;

void NewPageTemplateDialog::configureUI() {
    setActionDescription(_("These settings will be used for new pages"));
    setNewPageOnlySettingsVisible(true);
}

void NewPageTemplateDialog::initModel(PageTemplateSettings& model) { model.parse(settings_->getPageTemplate()); }

void NewPageTemplateDialog::save(const PageTemplateSettings& model) { settings_->setPageTemplate(model.toString()); }
