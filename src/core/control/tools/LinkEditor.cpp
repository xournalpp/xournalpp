#include "LinkEditor.h"

#include <iostream>

#include "model/XojPage.h"  // for XojPage

LinkEditor::LinkEditor(Control* control, GtkWidget* xournalWidget): control(control), documentWidget(xournalWidget) {
    std::cout << "LinkEditor created" << std::endl;
}

LinkEditor::~LinkEditor() { std::cout << "LinkEditor destroyed" << std::endl; }

void LinkEditor::startEditing(const PageRef& page, const int x, const int y) {
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
        Link* link = new Link();
        link->setText("Hello World");
        link->setUrl("http://google.com");
        link->setX(x), link->setY(y);
        page->getSelectedLayer()->addElement(link);
        page->firePageChanged();
    }
}
