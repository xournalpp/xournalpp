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
#include "model/XojPage.h"             // for XojPage


LinkEditor::LinkEditor(XournalView* view): view(view), control(view->getControl()), documentWidget(view->getWidget()) {
    this->createPopover();
    this->control->getZoomControl()->addZoomListener(this);
}

LinkEditor::~LinkEditor() {
    gtk_widget_destroy(GTK_WIDGET(this->linkPopoverLabelHightlight));
    gtk_widget_destroy(GTK_WIDGET(this->linkPopoverHighlight));
    gtk_widget_destroy(GTK_WIDGET(this->linkPopoverLabelSelect));
    gtk_widget_destroy(GTK_WIDGET(this->linkPopoverSelect));
}

void LinkEditor::startEditing(const PageRef& page, const int x, const int y) {
    this->linkElement = nullptr;

    // Find Link element
    for (auto&& e: page->getSelectedLayer()->getElements()) {
        if (e->getType() == ELEMENT_LINK && e->containsPoint(x, y)) {
            this->linkElement = dynamic_cast<Link*>(e.get());
        }
    }

    if (this->linkElement == nullptr) {
        LinkDialog dialog(this->control);
        int response = dialog.show();
        if (response == LinkDialog::CANCEL) {
            return;
        }
        auto linkOwn = std::make_unique<Link>();
        Link* link = linkOwn.get();
        link->setText(dialog.getText());
        link->setUrl(dialog.getURL());
        link->setAlignment(static_cast<PangoAlignment>(dialog.getLayout()));
        link->setFont(dialog.getFont());
        link->setX(x), link->setY(y);
        page->getSelectedLayer()->addElement(std::move(linkOwn));
        page->firePageChanged();
    } else {
        this->linkElement->setHighlighted(true);
        page->firePageChanged();
        LinkDialog dialog(this->control);
        dialog.preset(this->linkElement->getFont(), this->linkElement->getText(), this->linkElement->getUrl(),
                      static_cast<LinkAlignment>(this->linkElement->getAlignment()));
        int response = dialog.show();
        if (response == LinkDialog::CANCEL || response == GTK_RESPONSE_DELETE_EVENT) {
            this->linkElement->setHighlighted(false);
            page->fireElementChanged(this->linkElement);
            return;
        }
        this->linkElement->setText(dialog.getText());
        this->linkElement->setUrl(dialog.getURL());
        this->linkElement->setAlignment(static_cast<PangoAlignment>(dialog.getLayout()));
        this->linkElement->setFont(dialog.getFont());
        this->linkElement->sizeChanged();
        this->linkElement->setHighlighted(false);
        page->firePageChanged();
    }
}

void LinkEditor::select(const PageRef& page, const int x, const int y, const bool controlDown, XojPageView* pageView) {
    bool noSelection = true;
    for (auto&& e: page->getSelectedLayer()->getElements()) {
        if (e->getType() == ELEMENT_LINK && e->containsPoint(x, y)) {
            Link* localLinkElement = dynamic_cast<Link*>(e.get());

            if (controlDown) {
                GError* error = NULL;
                gtk_show_uri_on_window(NULL, localLinkElement->getUrl().c_str(), GDK_CURRENT_TIME, &error);
                if (error != NULL) {
                    GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;
                    GtkWidget* dialog = gtk_message_dialog_new(control->getGtkWindow(), flags, GTK_MESSAGE_ERROR,
                                                               GTK_BUTTONS_CLOSE, "Error opening “%s”: %s",
                                                               localLinkElement->getUrl().c_str(), error->message);
                    gtk_dialog_run(GTK_DIALOG(dialog));
                    gtk_widget_destroy(dialog);
                    g_error_free(error);
                }
                return;
            }

            if (!localLinkElement->isSelected()) {
                localLinkElement->setSelected(true);
                this->positionPopover(pageView, localLinkElement, linkPopoverSelect);
                gtk_label_set_markup(GTK_LABEL(this->linkPopoverLabelSelect),
                                     this->toLinkMarkup(localLinkElement->getUrl()).c_str());
                gtk_widget_show_all(GTK_WIDGET(this->linkPopoverSelect));
                if (localLinkElement->isHighlighted()) {
                    gtk_widget_hide(GTK_WIDGET(this->linkPopoverHighlight));
                }
                gtk_popover_popup(this->linkPopoverSelect);
                this->selectedLink = localLinkElement;
            } else {
                localLinkElement->setSelected(false);
                if (localLinkElement->isHighlighted()) {
                    gtk_widget_hide(GTK_WIDGET(this->linkPopoverSelect));
                    gtk_widget_show(GTK_WIDGET(this->linkPopoverHighlight));
                } else {
                    gtk_popover_popdown(this->linkPopoverSelect);
                }
                this->selectedLink = nullptr;
            }
            page->fireElementChanged(localLinkElement);
            noSelection = false;
        } else if (e->getType() == ELEMENT_LINK) {
            Link* localLinkElement = dynamic_cast<Link*>(e.get());
            localLinkElement->setSelected(false);
            page->firePageChanged();
        }
    }

    if (noSelection) {
        gtk_popover_popdown(this->linkPopoverSelect);
        bool noPreselection = (this->selectedLink == nullptr);
        this->selectedLink = nullptr;
        if (noPreselection) {
            startEditing(page, x, y);
        }
    }
}

void LinkEditor::highlight(const PageRef& page, const int x, const int y, XojPageView* pageView) {
    for (auto&& e: page->getSelectedLayer()->getElements()) {
        if (e->getType() == ELEMENT_LINK && e->containsPoint(x, y)) {
            this->highlightedLink = dynamic_cast<Link*>(e.get());
            this->highlightedLink->setHighlighted(true);
            page->fireElementChanged(this->highlightedLink);
            GdkWindow* window = gtk_widget_get_window(this->documentWidget);
            GdkCursor* cursor = gdk_cursor_new_from_name(gdk_window_get_display(window), "alias");
            gdk_window_set_cursor(window, cursor);
            positionPopover(pageView, this->highlightedLink, this->linkPopoverHighlight);
            gtk_label_set_text(GTK_LABEL(this->linkPopoverLabelHightlight), highlightedLink->getUrl().c_str());
            if (!this->highlightedLink->isSelected()) {
                gtk_widget_show_all(GTK_WIDGET(this->linkPopoverHighlight));
                gtk_popover_popup(this->linkPopoverHighlight);
            }
        } else if (e->getType() == ELEMENT_LINK && this->highlightedLink == e.get()) {
            Link* localLinkElement = dynamic_cast<Link*>(e.get());
            localLinkElement->setHighlighted(false);
            page->fireElementChanged(localLinkElement);
            GdkWindow* window = gtk_widget_get_window(view->getWidget());
            GdkCursor* cursor = gdk_cursor_new_from_name(gdk_window_get_display(window), "hand2");
            gdk_window_set_cursor(window, cursor);
            if (!this->highlightedLink->isSelected()) {
                gtk_popover_popdown(this->linkPopoverHighlight);
            }
            this->highlightedLink = nullptr;
        }
    }
}

void LinkEditor::createPopover() {
    this->linkPopoverHighlight = GTK_POPOVER(gtk_popover_new(view->getWidget()));
    gtk_popover_set_modal(linkPopoverHighlight, false);
    gtk_popover_set_constrain_to(this->linkPopoverHighlight, GTK_POPOVER_CONSTRAINT_WINDOW);
    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    this->linkPopoverLabelHightlight = gtk_label_new("");
    gtk_box_pack_start(GTK_BOX(vbox), this->linkPopoverLabelHightlight, true, true, POPOVER_PADDING);
    gtk_container_add(GTK_CONTAINER(this->linkPopoverHighlight), vbox);

    this->linkPopoverSelect = GTK_POPOVER(gtk_popover_new(view->getWidget()));
    gtk_popover_set_modal(linkPopoverSelect, false);
    gtk_popover_set_constrain_to(this->linkPopoverSelect, GTK_POPOVER_CONSTRAINT_WINDOW);
    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    this->linkPopoverLabelSelect = gtk_label_new("");
    gtk_box_pack_start(GTK_BOX(vbox), this->linkPopoverLabelSelect, true, true, POPOVER_PADDING);
    gtk_container_add(GTK_CONTAINER(this->linkPopoverSelect), vbox);
}

std::string LinkEditor::toLinkMarkup(std::string url) {
    return "<a href=\"" + url + "\"> " + url + "</a> \n" +
           "<span size=\"smaller\"><i> Double click to edit. CTRL + click to open. </i></span>";
}

void LinkEditor::positionPopover(XojPageView* pageView, Link* element, GtkPopover* popover) {
    auto pos = pageView->getPixelPosition();
    GdkRectangle rect{pos.x + int(element->getX() * pageView->getZoom()),
                      pos.y + int(element->getY() * pageView->getZoom()),
                      int(element->getElementWidth() * pageView->getZoom()),
                      int(element->getElementHeight() * pageView->getZoom())};
    gtk_popover_set_pointing_to(popover, &rect);
}

void LinkEditor::zoomChanged() {
    if (this->selectedLink != nullptr) {
        this->positionPopover(view->getViewFor(view->getCurrentPage()), this->selectedLink, this->linkPopoverSelect);
    }

    if (this->highlightedLink != nullptr) {
        this->positionPopover(view->getViewFor(view->getCurrentPage()), this->highlightedLink,
                              this->linkPopoverHighlight);
    }
}
