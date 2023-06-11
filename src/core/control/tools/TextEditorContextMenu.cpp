#include "TextEditorContextMenu.h"

#include <iostream>

#include "control/Control.h"                // for Control
#include "gui/GladeSearchpath.h"            // for GladeSearchPath
#include "gui/PageView.h"                   // for PageView
#include "view/overlays/TextEditionView.h"  // for TextEditionView

#include "TextEditor.h"  // for TextEditor

TextEditorContextMenu::TextEditorContextMenu(Control* control, TextEditor* editor, XojPageView* pageView,
                                             GtkWidget* xournalWidget):
        control(control), editor(editor), pageView(pageView), xournalWidget(xournalWidget) {
    // Only for debugging
    std::cout << "TextEditorContextMenu created!" << std::endl;
    this->create();
}

TextEditorContextMenu::~TextEditorContextMenu() {
    // Only for debugging
    std::cout << "TextEditorContextMenu destroyed!" << std::endl;
}

void TextEditorContextMenu::show() {
    this->reposition();
    gtk_widget_show_all(GTK_WIDGET(this->contextMenu));
    gtk_popover_popup(this->contextMenu);
    std::cout << "Popup menu should be shown" << std::endl;
}

void TextEditorContextMenu::hide() {
    gtk_popover_popdown(this->contextMenu);
    std::cout << "Popup menu should be hidden" << std::endl;
}

void TextEditorContextMenu::reposition() {
    // When no text element is selected no context menu should be displayed
    /*if (this->textElement == nullptr) {
        gtk_popover_popdown(this->contextMenu);
        return;
    }*/

    int padding = xoj::view::TextEditionView::PADDING_IN_PIXELS;
    Range r = this->editor->getContentBoundingBox();
    GdkRectangle rect{this->pageView->getX() + int(r.getX() * this->pageView->getZoom()),
                      this->pageView->getY() + int(r.getY() * this->pageView->getZoom()) - padding,
                      int(r.getWidth() * this->pageView->getZoom()), int(r.getHeight() * this->pageView->getZoom())};
    gtk_popover_set_pointing_to(this->contextMenu, &rect);
}

void TextEditorContextMenu::create() {
    auto filepath = this->control->getGladeSearchPath()->findFile("", "textEditorContextMenu.glade");

    GtkBuilder* builder = gtk_builder_new();

    GError* err = NULL;
    if (gtk_builder_add_from_file(builder, filepath.u8string().c_str(), &err) == 0) {
        std::cout << err->message << std::endl;
    }

    this->contextMenu = GTK_POPOVER(gtk_builder_get_object(builder, "textEditorContextMenu"));
    gtk_popover_set_relative_to(this->contextMenu, this->xournalWidget);
    gtk_popover_set_constrain_to(this->contextMenu, GTK_POPOVER_CONSTRAINT_WINDOW);

    GtkButton* btn = GTK_BUTTON(gtk_builder_get_object(builder, "btnFontColor"));

    g_object_unref(G_OBJECT(builder));
}
