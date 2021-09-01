/*
 * Xournal++
 *
 * Dialog to configure the template for new pages.
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "PageTemplateDialog.h"

class NewPageTemplateDialog: public PageTemplateDialog {
public:
    NewPageTemplateDialog(GladeSearchpath* gladeSearchPath, Settings* settings, PageTypeHandler* types);
    virtual ~NewPageTemplateDialog();

protected:
    virtual void configureUI() override;
    virtual void initModel(PageTemplateSettings& model) override;
    virtual void save(const PageTemplateSettings& model) override;

private:
    Settings* const settings_;
};
