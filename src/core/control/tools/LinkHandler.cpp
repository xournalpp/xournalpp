#include "LinkHandler.h"

#include <iostream>

#include <gdk/gdk.h>  // for GdkRectangle, Gdk...
#include <gtk/gtk.h>  // for GtkWindow

#include "control/Control.h"           // for Control
#include "control/zoom/ZoomControl.h"  // for ZoomControl
#include "gui/LinkPopover.h"           // for LinkPopover
#include "gui/PageView.h"              // for PageView
#include "gui/XournalView.h"           // for XournalView
#include "gui/XournalppCursor.h"       // for XournalppCursor
#include "gui/dialog/LinkDialog.h"     // for LinkDialog
#include "model/Document.h"            // for Document
#include "model/Link.h"                // for Link
#include "model/XojPage.h"             // for XojPage
#include "undo/DeleteUndoAction.h"     // for DeleteUndoAction
#include "undo/GroupUndoAction.h"      // for GroupUndoAction
#include "undo/InsertUndoAction.h"     // for InsertUndoAction
#include "util/PopupWindowWrapper.h"   // for PopupWindowWrapper
#include "util/XojMsgBox.h"            // for XojMsgBox
#include "util/i18n.h"                 // for FS, _, _F


LinkHandler::LinkHandler(XournalView* view): view(view), control(view->getControl()) {
    this->highlightPopover = std::make_unique<LinkPopover>(view);
    this->selectPopover = std::make_unique<LinkPopover>(view);
    this->control->getZoomControl()->addZoomListener(this);
}

LinkHandler::~LinkHandler() { this->control->getZoomControl()->removeZoomListener(this); }

void LinkHandler::startEditing(const PageRef& page, const int x, const int y) {
    Link* linkElement = nullptr;

    // Find Link element
    for (auto&& e: page->getSelectedLayer()->getElements()) {
        if (e->getType() == ELEMENT_LINK && e->containsPoint(x, y)) {
            linkElement = dynamic_cast<Link*>(e.get());
        }
    }

    if (linkElement == nullptr) {
        auto dialog = xoj::popup::PopupWindowWrapper<LinkDialog>(
                this->control,
                [x, y, page = page, control = control](LinkDialog* dlg) {
                    auto linkOwn = std::make_unique<Link>();
                    Link* link = linkOwn.get();
                    link->setText(dlg->getText());
                    link->setUrl(dlg->getURL());
                    link->setAlignment(static_cast<PangoAlignment>(dlg->getLayout()));
                    link->setFont(dlg->getFont());
                    link->setTextPos(x, y);
                    Document* doc = control->getDocument();
                    doc->lock();
                    const auto layer = page->getSelectedLayer();
                    layer->addElement(std::move(linkOwn));
                    doc->unlock();
                    page->firePageChanged();
                    const auto undo = control->getUndoRedoHandler();
                    undo->addUndoAction(std::make_unique<InsertUndoAction>(page, layer, link));
                },
                []() {});
        dialog.show(control->getGtkWindow());
    } else {
        linkElement->setHighlighted(true);
        page->fireElementChanged(linkElement);
        auto dialog = xoj::popup::PopupWindowWrapper<LinkDialog>(
                this->control,
                [this, linkElement, page = page](LinkDialog* dlg) {
                    auto linkOwn = std::make_unique<Link>();
                    Link* link = linkOwn.get();
                    link->setText(dlg->getText());
                    link->setUrl(dlg->getURL());
                    link->setAlignment(static_cast<PangoAlignment>(dlg->getLayout()));
                    link->setFont(dlg->getFont());
                    link->setX(linkElement->getX());
                    link->setY(linkElement->getY());
                    page->firePageChanged();

                    const auto undo = control->getUndoRedoHandler();
                    auto groupUndoAction = std::make_unique<GroupUndoAction>();
                    auto deleteUndoAction = std::make_unique<DeleteUndoAction>(page, false);

                    Document* doc = control->getDocument();
                    doc->lock();
                    const auto layer = page->getSelectedLayer();
                    layer->addElement(std::move(linkOwn));
                    auto [orig, elementIndex] = layer->removeElement(linkElement);
                    doc->unlock();

                    if (elementIndex != Element::InvalidIndex) [[likely]] {
                        deleteUndoAction->addElement(layer, std::move(orig), elementIndex);
                    }
                    groupUndoAction->addAction(std::move(deleteUndoAction));
                    auto insertUndoAction = std::make_unique<InsertUndoAction>(page, layer, link);
                    groupUndoAction->addAction(std::move(insertUndoAction));
                    undo->addUndoAction(std::move(groupUndoAction));
                },
                [linkElement, page = page]() {
                    linkElement->setHighlighted(false);
                    page->fireElementChanged(linkElement);
                });
        dialog.getPopup()->preset(linkElement->getFont(), linkElement->getText(), linkElement->getUrl(),
                                  static_cast<LinkAlignment>(linkElement->getAlignment()));
        dialog.show(control->getGtkWindow());
    }
}

void LinkHandler::select(const PageRef& page, const int x, const int y, const bool controlDown, XojPageView* pageView) {
    bool noSelection = true;
    for (auto&& e: page->getSelectedLayer()->getElements()) {
        if (e->getType() == ELEMENT_LINK && e->containsPoint(x, y)) {
            Link* link = dynamic_cast<Link*>(e.get());  // link on which user clicked

            if (controlDown) {
                XojMsgBox::openURL(nullptr, link->getUrl().c_str());
                return;
            }

            if (!link->isSelected()) {
                if (link->isHighlighted()) {
                    this->highlightPopover->hide();
                }
                link->setSelected(true);
                // FixMe !!
                this->selectPopover->linkTo(link);
                this->selectPopover->updateLabel(true);
                this->selectPopover->popup();
            } else {
                link->setSelected(false);
                if (link->isHighlighted()) {
                    this->selectPopover->hide();
                    this->highlightPopover->show();
                } else {
                    this->selectPopover->popdown();
                }
                this->selectPopover->linkTo(nullptr);
            }
            page->fireElementChanged(link);
            noSelection = false;
        } else if (e->getType() == ELEMENT_LINK) {
            Link* link = dynamic_cast<Link*>(e.get());
            if (link->isSelected()) {
                link->setSelected(false);
                page->fireElementChanged(link);
            }
        }
    }

    if (noSelection) {
        this->selectPopover->popdown();
        bool preselection = this->selectPopover->hasLink();
        this->selectPopover->linkTo(nullptr);
        if (!preselection) {
            startEditing(page, x, y);
        }
    }
}

void LinkHandler::highlight(const PageRef& page, const int x, const int y, XojPageView* pageView) {
    for (auto&& e: page->getSelectedLayer()->getElements()) {
        if (e->getType() == ELEMENT_LINK && e->containsPoint(x, y)) {
            Link* link = dynamic_cast<Link*>(e.get());
            if (link->isHighlighted()) {
                continue;
            }
            link->setHighlighted(true);
            this->highlightPopover->linkTo(link);
            page->fireElementChanged(link);
            view->getControl()->getCursor()->setIsLinkHighlighted(true);
            this->highlightPopover->updateLabel(false);
            if (!link->isSelected()) {
                this->highlightPopover->popup();
            }
        } else if (e->getType() == ELEMENT_LINK && this->highlightPopover->getLink() == e.get()) {
            Link* link = dynamic_cast<Link*>(e.get());
            link->setHighlighted(false);
            page->fireElementChanged(link);
            view->getControl()->getCursor()->setIsLinkHighlighted(false);
            if (!this->highlightPopover->getLink()->isSelected()) {
                this->highlightPopover->popdown();
            }
            this->highlightPopover->linkTo(nullptr);
        }
    }
}

void LinkHandler::zoomChanged() {
    this->selectPopover->hide();
    this->highlightPopover->hide();
}
