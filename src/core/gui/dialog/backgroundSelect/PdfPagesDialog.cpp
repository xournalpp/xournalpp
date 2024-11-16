#include "PdfPagesDialog.h"

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

#include <glib-object.h>

#include "gui/dialog/backgroundSelect/BackgroundSelectDialogBase.h"
#include "gui/dialog/backgroundSelect/BaseElementView.h"
#include "model/Document.h"
#include "model/PageRef.h"
#include "model/PageType.h"
#include "model/XojPage.h"
#include "pdf/base/XojPdfPage.h"
#include "util/Assert.h"
#include "util/Util.h"  // for npos
#include "util/i18n.h"  // for FC

#include "PdfElementView.h"

class GladeSearchpath;
class Settings;

PdfPagesDialog::PdfPagesDialog(GladeSearchpath* gladeSearchPath, Document* doc, Settings* settings,
                               std::function<void(size_t)> callback):
        BackgroundSelectDialogBase(gladeSearchPath, doc, settings, _("Select PDF Page")),
        callback(std::move(callback)) {
    doc->lock();
    for (size_t i = 0; i < doc->getPdfPageCount(); i++) {
        XojPdfPageSPtr p = doc->getPdfPage(i);
        entries.emplace_back(std::make_unique<PdfElementView>(entries.size(), p, this));
    }

    size_t used = 0;
    for (size_t i = 0; i < doc->getPageCount(); i++) {
        PageRef p = doc->getPage(i);

        if (p->getBackgroundType().isPdfPage()) {
            auto pdfPage = p->getPdfPageNr();
            if (pdfPage >= 0 && pdfPage < entries.size()) {
                auto* pv = (static_cast<PdfElementView*>(entries[pdfPage].get()));
                if (!pv->isUsed()) {
                    pv->setUsed(true);
                    used++;
                }
            }
        }
    }
    xoj_assert(used <= doc->getPdfPageCount());
    auto unused = doc->getPdfPageCount() - used;
    doc->unlock();

    cbUnusedOnly = GTK_CHECK_BUTTON(gtk_check_button_new_with_label(
            (unused == 1 ? _("Show only not used pages (one unused page)") :
                           FC(_F("Show only not used pages ({1} unused pages)") % unused))));

    gtk_box_prepend(vbox, GTK_WIDGET(cbUnusedOnly));

    g_signal_connect(cbUnusedOnly, "toggled", G_CALLBACK(onlyNotUsedCallback), this);

    g_signal_connect_swapped(okButton, "clicked", G_CALLBACK(+[](PdfPagesDialog* self) {
                                 if (self->selected < self->entries.size()) {
                                     self->callback(self->selected);
                                 }
                                 gtk_window_close(self->window.get());
                             }),
                             this);

    populate();
}

PdfPagesDialog::~PdfPagesDialog() = default;

void PdfPagesDialog::updateOkButton() {
    bool valid = selected < entries.size() && gtk_widget_get_visible(this->entries[this->selected]->getWidget());
    gtk_widget_set_sensitive(okButton, valid);
}

void PdfPagesDialog::onlyNotUsedCallback(GtkToggleButton* tb, PdfPagesDialog* dlg) {
    bool hideIfUsed = gtk_toggle_button_get_active(tb);
    for (const auto& p: dlg->entries) {
        auto* pv = static_cast<PdfElementView*>(p.get());
        pv->setHideIfUsed(hideIfUsed);
    }

    dlg->layout();
    dlg->updateOkButton();
}
