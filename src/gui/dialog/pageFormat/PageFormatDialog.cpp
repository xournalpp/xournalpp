#include "PageFormatDialog.h"

#include "control/Control.h"

#include "i18n.h"

PageFormatDialog::PageFormatDialog(GladeSearchpath* searchpath, PageRef target, Control* control, Settings* settings,
                                   PageTypeHandler* typeHandler):
        PageTemplateDialog(searchpath, settings, typeHandler), control_(control), target_(target) {}

PageFormatDialog::~PageFormatDialog() = default;

void PageFormatDialog::configureUI() {
    setActionDescription(_("Update a single page"));

    // We're not creating new pages.
    setNewPageOnlySettingsVisible(false);
}

void PageFormatDialog::initModel(PageTemplateSettings& model) {
    model.setPageWidth(target_->getWidth());
    model.setPageHeight(target_->getHeight());
    model.setBackgroundType(target_->getBackgroundType());
    model.setBackgroundColor(target_->getBackgroundColor());
}

void PageFormatDialog::save(const PageTemplateSettings& model) {
    double width = model.getPageWidth();
    double height = model.getPageHeight();

    Document* document = control_->getDocument();

    if (width > 0 && height > 0) {
        document->lock();
        Document::setPageSize(target_, width, height);
        document->unlock();
    }

    target_->setSize(model.getPageWidth(), model.getPageHeight());
    target_->setBackgroundColor(model.getBackgroundColor());
    target_->setBackgroundType(model.getBackgroundType());

    size_t pageNo = document->indexOf(target_);
    if (pageNo != npos && pageNo < document->getPageCount()) {
        control_->firePageSizeChanged(pageNo);
        control_->firePageChanged(pageNo);
    }
}
