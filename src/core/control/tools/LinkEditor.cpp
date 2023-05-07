#include "LinkEditor.h"

#include <iostream>

#include "control/Control.h"        // for Control
#include "gui/GladeSearchpath.h"    // for GladeSearchPath
#include "gui/dialog/LinkDialog.h"  // for LinkDialog
#include "model/XojPage.h"          // for XojPage

LinkEditor::LinkEditor(Control* control, GtkWidget* xournalWidget): control(control), documentWidget(xournalWidget) {
    std::cout << "LinkEditor created" << std::endl;
}

LinkEditor::~LinkEditor() { std::cout << "LinkEditor destroyed" << std::endl; }

void LinkEditor::startEditing(const PageRef& page, const int x, const int y, const bool controlDown) {
    std::cout << "LinkEditor starts editing" << std::endl;

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
        link->setX(x), link->setY(y);
        page->getSelectedLayer()->addElement(link);
        page->firePageChanged();
    } else {
        if (controlDown) {
            GError* error = NULL;
            gtk_show_uri_on_window(NULL, this->linkElement->getUrl().c_str(), GDK_CURRENT_TIME, &error);
            if (error != NULL) {
                GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;
                GtkWidget* dialog = gtk_message_dialog_new(control->getGtkWindow(), flags, GTK_MESSAGE_ERROR,
                                                           GTK_BUTTONS_CLOSE, "Error opening “%s”: %s",
                                                           this->linkElement->getUrl().c_str(), error->message);
                gtk_dialog_run(GTK_DIALOG(dialog));
                gtk_widget_destroy(dialog);
                g_error_free(error);
            }
        } else {
            std::cout << "Existing LinkElement to be edited!" << std::endl;
            this->linkElement->setInEditing(true);
            page->firePageChanged();
            LinkDialog dialog(this->control);
            dialog.preset(this->linkElement->getText(), this->linkElement->getUrl());
            int response = dialog.show();
            if (response == LinkDialog::CANCEL) {
                this->linkElement->setInEditing(false);
                page->fireElementChanged(this->linkElement);
                return;
            }
            this->linkElement->setText(dialog.getText());
            this->linkElement->setUrl(dialog.getURL());
            this->linkElement->sizeChanged();
            this->linkElement->setInEditing(false);
            page->firePageChanged();
        }
    }
}
