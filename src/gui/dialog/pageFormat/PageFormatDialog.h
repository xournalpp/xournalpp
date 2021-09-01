/*
 * Xournal++
 *
 * Dialog to configure a single page.
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "model/PageRef.h"

#include "PageTemplateDialog.h"

class Control;

class PageFormatDialog: public PageTemplateDialog {
public:
    PageFormatDialog(GladeSearchpath* gladeSearchPath, PageRef target, Control* control, Settings* settings,
                     PageTypeHandler* types);
    virtual ~PageFormatDialog();

protected:
    virtual void configureUI() override;
    virtual void initModel(PageTemplateSettings& model) override;
    virtual void save(const PageTemplateSettings& model) override;

private:
    PageRef target_;
    Control* const control_;
};
