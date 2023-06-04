#include "LinkEditor.h"

#include <iostream>

#include <gdk/gdk.h>  // for GdkRectangle, Gdk...
#include <gtk/gtk.h>  // for GtkWidget, gtk_co...

#include "control/Control.h"        // for Control
#include "gui/GladeSearchpath.h"    // for GladeSearchPath
#include "gui/PageView.h"           // for PageView
#include "gui/XournalView.h"        // for XournalView
#include "gui/dialog/LinkDialog.h"  // for LinkDialog
#include "model/XojPage.h"          // for XojPage


LinkEditor::LinkEditor(XournalView* view): view(view), control(view->getControl()), documentWidget(view->getWidget()) {
    std::cout << "LinkEditor created" << std::endl;
    this->createPopover();
}

LinkEditor::~LinkEditor() { std::cout << "LinkEditor destroyed" << std::endl; }

void LinkEditor::startEditing(const PageRef& page, const int x, const int y) {
    std::cout << "LinkEditor starts editing" << std::endl;
    this->linkElement = nullptr;

    // Find Link element
    for (Element* e: page->getSelectedLayer()->getElements()) {
        if (e->getType() == ELEMENT_LINK && e->containsPoint(x, y)) {
            this->linkElement = dynamic_cast<Link*>(e);
            std::cout << "LinkElement already exist at position: " << x << "/" << y << std::endl;
        }
    }

    if (this->linkElement == nullptr) {
        std::cout << "New LinkElement to be created!" << std::endl;

        LinkDialog dialog(this->control);
        int response = dialog.show();
        if (response == LinkDialog::CANCEL) {
            return;
        }
        Link* link = new Link();
        link->setText(dialog.getText());
        link->setUrl(dialog.getURL());
        link->setAlignment(static_cast<PangoAlignment>(dialog.getLayout()));
        link->setFont(dialog.getFont());
        link->setX(x), link->setY(y);
        page->getSelectedLayer()->addElement(link);
        page->firePageChanged();
    } else {
        std::cout << "Existing LinkElement to be edited!" << std::endl;
        this->linkElement->setHighlighted(true);
        page->firePageChanged();
        LinkDialog dialog(this->control);
        dialog.preset(this->linkElement->getFont(), this->linkElement->getText(), this->linkElement->getUrl(),
                      static_cast<LinkAlignment>(this->linkElement->getAlignment()));
        int response = dialog.show();
        std::cout << "Dialog closed: " << response << std::endl;
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
    std::cout << "LinkEditor select" << std::endl;

    bool noSelection = true;
    for (Element* e: page->getSelectedLayer()->getElements()) {
        if (e->getType() == ELEMENT_LINK && e->containsPoint(x, y)) {
            Link* localLinkElement = dynamic_cast<Link*>(e);

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
                GdkRectangle rect{pageView->getX() + int(localLinkElement->getX() * pageView->getZoom()),
                                  pageView->getY() + int(localLinkElement->getY() * pageView->getZoom()),
                                  int(localLinkElement->getElementWidth() * pageView->getZoom()),
                                  int(localLinkElement->getElementHeight() * pageView->getZoom())};
                gtk_popover_set_pointing_to(this->linkPopoverSelect, &rect);
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
            Link* localLinkElement = dynamic_cast<Link*>(e);
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
    for (Element* e: page->getSelectedLayer()->getElements()) {
        if (e->getType() == ELEMENT_LINK && e->containsPoint(x, y)) {
            this->highlightedLink = dynamic_cast<Link*>(e);
            this->highlightedLink->setHighlighted(true);
            page->fireElementChanged(this->highlightedLink);
            GdkWindow* window = gtk_widget_get_window(this->documentWidget);
            GdkCursor* cursor = gdk_cursor_new_from_name(gdk_window_get_display(window), "alias");
            gdk_window_set_cursor(window, cursor);
            GdkRectangle rect{pageView->getX() + int(highlightedLink->getX() * pageView->getZoom()),
                              pageView->getY() + int(highlightedLink->getY() * pageView->getZoom()),
                              int(highlightedLink->getElementWidth() * pageView->getZoom()),
                              int(highlightedLink->getElementHeight() * pageView->getZoom())};
            gtk_popover_set_pointing_to(this->linkPopoverHighlight, &rect);
            gtk_label_set_text(GTK_LABEL(this->linkPopoverLabelHightlight), highlightedLink->getUrl().c_str());
            if (!this->highlightedLink->isSelected()) {
                gtk_widget_show_all(GTK_WIDGET(this->linkPopoverHighlight));
                gtk_popover_popup(this->linkPopoverHighlight);
            }
        } else if (e->getType() == ELEMENT_LINK && this->highlightedLink == e) {
            Link* localLinkElement = dynamic_cast<Link*>(e);
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
