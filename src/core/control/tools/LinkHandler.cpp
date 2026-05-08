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
#include "util/DispatchPool.h"         // for DispatchPool
#include "util/PopupWindowWrapper.h"   // for PopupWindowWrapper
#include "util/Rectangle.h"            // for Rectangle
#include "util/XojMsgBox.h"            // for XojMsgBox
#include "util/i18n.h"                 // for FS, _, _F
#include "view/overlays/LinkHighlightView.h"


LinkHandler::LinkHandler(XournalView* view):
        view(view),
        control(view->getControl()),
        highlightPopover(std::make_unique<LinkPopover>(view, false)),
        selectPopover(std::make_unique<LinkPopover>(view, true)),
        viewPool(std::make_shared<xoj::util::DispatchPool<xoj::view::LinkHighlightView>>()) {
    this->control->getZoomControl()->addZoomListener(this);
}

LinkHandler::~LinkHandler() {
    this->control->getZoomControl()->removeZoomListener(this);
    Range rg;
    if (auto rect = this->getHighlightRect(); rect.has_value()) {
        rg = Range(rect.value());
    }
    if (auto rect = this->getSelectRect(); rect.has_value()) {
        rg = rg.unite(Range(rect.value()));
    }
    this->viewPool->dispatchAndClear(xoj::view::LinkHighlightView::FINALIZATION_REQUEST, rg);
}

std::unique_ptr<xoj::view::OverlayView> LinkHandler::createView(xoj::view::Repaintable* parent) const {
    auto view = std::make_unique<xoj::view::LinkHighlightView>(this, parent);
    return view;
}

/*
 * Finds the first link on a given page at given coordinates
 * and returns a pointer to it.
 * Returns nullptr if no link has been found there.
 */
static Link* findLinkAtPos(const PageRef& page, int x, int y) {
    for (auto&& e: page->getSelectedLayer()->getElements()) {
        if (e->getType() == ELEMENT_LINK && e->hasBoundingBoxContaining(x, y)) {
            return dynamic_cast<Link*>(e.get());
        }
    }
    return nullptr;
}

void LinkHandler::startEditing(const PageRef& page, const int x, const int y) {
    Link* linkElement = findLinkAtPos(page, x, y);

    if (linkElement == nullptr) {
        auto dialog = xoj::popup::PopupWindowWrapper<LinkDialog>(
                this->control,
                [x, y, page = page, control = control](LinkDialog* dlg) {
                    auto linkOwn = std::make_unique<Link>();
                    Link* link = linkOwn.get();
                    link->setText(dlg->getText());
                    link->setUrl(dlg->getURL());
                    link->setAlignment(dlg->getLayout());
                    link->setFont(dlg->getFont());
                    link->setTextPos(x, y);
                    Document* doc = control->getDocument();
                    doc->lock();
                    const auto layer = page->getSelectedLayer();
                    layer->addElement(std::move(linkOwn));
                    doc->unlock();
                    page->fireElementChanged(link);
                    const auto undo = control->getUndoRedoHandler();
                    undo->addUndoAction(std::make_unique<InsertUndoAction>(page, layer, link));
                },
                []() {});
        dialog.show(control->getGtkWindow());
    } else {
        this->highlightPopover->linkTo(linkElement);
        auto dialog = xoj::popup::PopupWindowWrapper<LinkDialog>(
                this->control,
                [this, linkElement, page = page](LinkDialog* dlg) {
                    auto linkOwn = std::make_unique<Link>();
                    Link* link = linkOwn.get();
                    link->setText(dlg->getText());
                    link->setUrl(dlg->getURL());
                    link->setAlignment(dlg->getLayout());
                    link->setFont(dlg->getFont());
                    link->setX(linkElement->getX());
                    link->setY(linkElement->getY());

                    const auto undo = control->getUndoRedoHandler();
                    auto groupUndoAction = std::make_unique<GroupUndoAction>();
                    auto deleteUndoAction = std::make_unique<DeleteUndoAction>(page, false);

                    Document* doc = control->getDocument();
                    doc->lock();
                    const auto layer = page->getSelectedLayer();
                    layer->addElement(std::move(linkOwn));
                    auto [orig, elementIndex] = layer->removeElement(linkElement);
                    doc->unlock();
                    Range oldRange(linkElement->getSnappedBounds());
                    Range newRange(link->getSnappedBounds());
                    Range repaintRange = oldRange.unite(newRange);
                    page->fireRangeChanged(repaintRange);

                    this->highlightPopover->linkTo(link);
                    this->selectPopover->linkTo(nullptr);
                    this->selectPopover->popdown();

                    if (elementIndex != Element::InvalidIndex) [[likely]] {
                        deleteUndoAction->addElement(layer, std::move(orig), elementIndex);
                    }

                    groupUndoAction->addAction(std::move(deleteUndoAction));
                    auto insertUndoAction = std::make_unique<InsertUndoAction>(page, layer, link);
                    groupUndoAction->addAction(std::move(insertUndoAction));
                    undo->addUndoAction(std::move(groupUndoAction));
                },
                [this]() { this->highlightPopover->linkTo(nullptr); });
        dialog.getPopup()->preset(linkElement->getFont(), linkElement->getText(), linkElement->getUrl(),
                                  linkElement->getAlignment());
        dialog.show(control->getGtkWindow());
    }
}

void LinkHandler::select(const PageRef& page, const int x, const int y, const bool controlDown, XojPageView* pageView) {
    Link* link = findLinkAtPos(page, x, y);

    if (link == nullptr) {
        this->selectPopover->popdown();
        bool preselection = this->selectPopover->hasLink();
        this->selectPopover->linkTo(nullptr);
        if (!preselection) {
            startEditing(page, x, y);
        }
    } else if (controlDown) {
        XojMsgBox::openURL(nullptr, link->getUrl().c_str());
    } else if (isSelected(link)) {
        this->selectPopover->hide();
        this->highlightPopover->show();
        this->selectPopover->linkTo(nullptr);
    } else {
        if (isHighlighted(link)) {
            this->highlightPopover->hide();
        }
        this->selectPopover->linkTo(link);
        this->selectPopover->popup();
    }
}

void LinkHandler::highlight(const PageRef& page, const int x, const int y, XojPageView* pageView) {
    Link* link = findLinkAtPos(page, x, y);

    if (link == nullptr) {
        view->getControl()->getCursor()->setIsLinkHighlighted(false);
        this->highlightPopover->hide();
        this->highlightPopover->linkTo(nullptr);
    } else if (!isHighlighted(link)) {
        this->highlightPopover->linkTo(link);
        view->getControl()->getCursor()->setIsLinkHighlighted(true);
        if (!isSelected(link)) {
            this->highlightPopover->popup();
        }
    }
}

void LinkHandler::zoomChanged() {
    this->selectPopover->hide();
    this->highlightPopover->hide();
}

std::optional<xoj::util::Rectangle<double>> LinkHandler::getHighlightRect() const {
    if (!this->highlightPopover) {
        return std::nullopt;
    }
    return this->highlightPopover->getRect();
}
std::optional<xoj::util::Rectangle<double>> LinkHandler::getSelectRect() const {
    if (!this->selectPopover) {
        return std::nullopt;
    }
    return this->selectPopover->getRect();
}

bool LinkHandler::isHighlighted(const Link* link) const { return this->highlightPopover->getLink() == link; }
bool LinkHandler::isSelected(const Link* link) const { return this->selectPopover->getLink() == link; }
