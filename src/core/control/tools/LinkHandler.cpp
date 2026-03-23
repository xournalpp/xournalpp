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
                    link->setAlignment(static_cast<PangoAlignment>(dlg->getLayout()));
                    link->setFont(dlg->getFont());
                    link->setX(linkElement->getX());
                    link->setY(linkElement->getY());
                    page->fireElementChanged(link);

                    const auto undo = control->getUndoRedoHandler();
                    auto groupUndoAction = std::make_unique<GroupUndoAction>();
                    auto deleteUndoAction = std::make_unique<DeleteUndoAction>(page, false);

                    Document* doc = control->getDocument();
                    doc->lock();
                    const auto layer = page->getSelectedLayer();
                    layer->addElement(std::move(linkOwn));
                    auto [orig, elementIndex] = layer->removeElement(linkElement);
                    doc->unlock();
                    page->fireElementChanged(linkElement);  // rerender region around previous element

                    this->highlightPopover->linkTo(link);
                    this->selectPopover->linkTo(nullptr);
                    this->selectPopover->popdown();
                    page->fireElementChanged(link);

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

            if (!isSelected(link)) {
                if (isHighlighted(link)) {
                    this->highlightPopover->hide();
                }
                this->selectPopover->linkTo(link);
                this->selectPopover->popup();
            } else {
                if (isHighlighted(link)) {
                    this->selectPopover->hide();
                    this->highlightPopover->show();
                } else {
                    this->selectPopover->popdown();
                }
                this->selectPopover->linkTo(nullptr);
            }
            noSelection = false;
        } else if (e->getType() == ELEMENT_LINK) {
            Link* link = dynamic_cast<Link*>(e.get());
            if (isSelected(link)) {
                this->selectPopover->linkTo(nullptr);
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
            if (isHighlighted(link)) {
                continue;
            }
            this->highlightPopover->linkTo(link);
            view->getControl()->getCursor()->setIsLinkHighlighted(true);
            if (!isSelected(link)) {
                this->highlightPopover->popup();
            }
        } else if (e->getType() == ELEMENT_LINK && isHighlighted(dynamic_cast<Link*>(e.get()))) {
            view->getControl()->getCursor()->setIsLinkHighlighted(false);
            if (!isSelected(this->highlightPopover->getLink())) {
                this->highlightPopover->hide();
            }
            this->highlightPopover->linkTo(nullptr);
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
