#include "LinkEditor.h"

#include <iostream>

#include <gdk/gdk.h>  // for GdkRectangle, Gdk...
#include <gtk/gtk.h>  // for GtkWidget, gtk_co...

#include "control/Control.h"           // for Control
#include "control/zoom/ZoomControl.h"  // for ZoomControl
#include "gui/GladeSearchpath.h"       // for GladeSearchPath
#include "gui/PageView.h"              // for PageView
#include "gui/XournalView.h"           // for XournalView
#include "gui/dialog/LinkDialog.h"     // for LinkDialog
#include "model/Document.h"            // for Document
#include "model/XojPage.h"             // for XojPage
#include "undo/InsertUndoAction.h"     // for InsertUndoAction
#include "util/PopupWindowWrapper.h"   // for PopupWindowWrapper
#include "util/XojMsgBox.h"            // for XojMsgBox
#include "util/i18n.h"                 // for FS, _, _F

LinkPopover::LinkPopover(XournalView* view): view(view) {
    this->popover = GTK_POPOVER(gtk_popover_new(view->getWidget()));
    gtk_popover_set_modal(popover, false);
    gtk_popover_set_constrain_to(this->popover, GTK_POPOVER_CONSTRAINT_WINDOW);
    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    this->label = GTK_LABEL(gtk_label_new(""));
    gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(this->label), true, true, POPOVER_PADDING);
    gtk_container_add(GTK_CONTAINER(this->popover), vbox);
}

LinkPopover::~LinkPopover() {
    gtk_widget_destroy(GTK_WIDGET(this->label));
    gtk_widget_destroy(GTK_WIDGET(this->popover));
}

void LinkPopover::show() { gtk_widget_show(GTK_WIDGET(this->popover)); }

void LinkPopover::show_all() { gtk_widget_show_all(GTK_WIDGET(this->popover)); }

void LinkPopover::hide() { gtk_widget_hide(GTK_WIDGET(this->popover)); }

void LinkPopover::popup() { gtk_popover_popup(this->popover); }

void LinkPopover::popdown() { gtk_popover_popdown(this->popover); }

bool LinkPopover::hasLink() { return (this->link != nullptr); }

void LinkPopover::updateLabel(bool markup) {
    if (markup) {
        std::string url = this->link->getUrl();
        std::string str = "<a href=\"" + url + "\"> " + url + "</a> \n" +
                          "<span size=\"smaller\"><i> Double click to edit. CTRL + click to open. </i></span>";
        gtk_label_set_markup(this->label, str.c_str());
    } else {
        gtk_label_set_text(this->label, this->link->getUrl().c_str());
    }
}

void LinkPopover::positionPopover() {
    if (this->link) {
        XojPageView* pageView = view->getViewFor(view->getCurrentPage());
        auto pos = pageView->getPixelPosition();
        GdkRectangle rect{pos.x + int(this->link->getX() * pageView->getZoom()),
                          pos.y + int(this->link->getY() * pageView->getZoom()),
                          int(this->link->getElementWidth() * pageView->getZoom()),
                          int(this->link->getElementHeight() * pageView->getZoom())};
        gtk_popover_set_pointing_to(this->popover, &rect);
    }
}

void LinkPopover::linkTo(Link* link) {
    this->link = link;
    positionPopover();
}

LinkEditor::LinkEditor(XournalView* view): view(view), control(view->getControl()) {
    this->highlightPopover = std::make_unique<LinkPopover>(view);
    this->selectPopover = std::make_unique<LinkPopover>(view);
    this->control->getZoomControl()->addZoomListener(this);
}

LinkEditor::~LinkEditor() { this->control->getZoomControl()->removeZoomListener(this); }

void LinkEditor::startEditing(const PageRef& page, const int x, const int y) {
    this->linkElement = nullptr;

    // Find Link element
    for (auto&& e: page->getSelectedLayer()->getElements()) {
        if (e->getType() == ELEMENT_LINK && e->containsPoint(x, y)) {
            this->linkElement = dynamic_cast<Link*>(e.get());
        }
    }

    if (this->linkElement == nullptr) {
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
        this->linkElement->setHighlighted(true);
        page->firePageChanged();
        auto dialog = xoj::popup::PopupWindowWrapper<LinkDialog>(
                this->control,
                [this, page = page](LinkDialog* dlg) {
                    this->linkElement->setText(dlg->getText());
                    this->linkElement->setUrl(dlg->getURL());
                    this->linkElement->setAlignment(static_cast<PangoAlignment>(dlg->getLayout()));
                    this->linkElement->setFont(dlg->getFont());
                    this->linkElement->sizeChanged();
                    this->linkElement->setHighlighted(false);
                    page->firePageChanged();
                },
                [this, page = page]() {
                    this->linkElement->setHighlighted(false);
                    page->fireElementChanged(this->linkElement);
                });
        dialog.getPopup()->preset(this->linkElement->getFont(), this->linkElement->getText(),
                                  this->linkElement->getUrl(),
                                  static_cast<LinkAlignment>(this->linkElement->getAlignment()));
        dialog.show(control->getGtkWindow());
    }
}

void LinkEditor::select(const PageRef& page, const int x, const int y, const bool controlDown, XojPageView* pageView) {
    bool noSelection = true;
    for (auto&& e: page->getSelectedLayer()->getElements()) {
        if (e->getType() == ELEMENT_LINK && e->containsPoint(x, y)) {
            Link* link = dynamic_cast<Link*>(e.get());  // link on which user clicked

            if (controlDown) {
                GError* error = NULL;
                gtk_show_uri_on_window(NULL, link->getUrl().c_str(), GDK_CURRENT_TIME, &error);
                if (error != NULL) {
                    auto* win = control->getGtkWindow();
                    std::string msg = FS(_F("Error opening “{1}”: {2}") % link->getUrl() % error->message);
                    XojMsgBox::showMessageToUser(win, msg, GTK_MESSAGE_ERROR);
                    g_error_free(error);
                }
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
                this->selectPopover->show_all();
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

void LinkEditor::highlight(const PageRef& page, const int x, const int y, XojPageView* pageView) {
    for (auto&& e: page->getSelectedLayer()->getElements()) {
        if (e->getType() == ELEMENT_LINK && e->containsPoint(x, y)) {
            Link* link = dynamic_cast<Link*>(e.get());
            if (link->isHighlighted()) {
                continue;
            }
            link->setHighlighted(true);
            this->highlightPopover->linkTo(link);
            page->fireElementChanged(link);
            GdkWindow* window = gtk_widget_get_window(view->getWidget());
            GdkCursor* cursor = gdk_cursor_new_from_name(gdk_window_get_display(window), "alias");
            gdk_window_set_cursor(window, cursor);
            this->highlightPopover->updateLabel(false);
            if (!link->isSelected()) {
                this->highlightPopover->show_all();
                this->highlightPopover->popup();
            }
        } else if (e->getType() == ELEMENT_LINK && this->highlightPopover->getLink() == e.get()) {
            Link* link = dynamic_cast<Link*>(e.get());
            link->setHighlighted(false);
            page->fireElementChanged(link);
            GdkWindow* window = gtk_widget_get_window(view->getWidget());
            GdkCursor* cursor = gdk_cursor_new_from_name(gdk_window_get_display(window), "hand2");
            gdk_window_set_cursor(window, cursor);
            if (!this->highlightPopover->getLink()->isSelected()) {
                this->highlightPopover->popdown();
            }
            this->highlightPopover->linkTo(nullptr);
        }
    }
}

void LinkEditor::zoomChanged() {
    this->selectPopover->positionPopover();
    this->highlightPopover->positionPopover();
}
