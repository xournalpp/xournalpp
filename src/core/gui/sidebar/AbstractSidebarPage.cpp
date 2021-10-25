#include "AbstractSidebarPage.h"

AbstractSidebarPage::AbstractSidebarPage(Control* control, SidebarToolbar* toolbar):
        control(control), toolbar(toolbar) {}

AbstractSidebarPage::~AbstractSidebarPage() {
    this->control = nullptr;
    this->toolbar = nullptr;
}

void AbstractSidebarPage::selectPageNr(size_t page, size_t pdfPage) {}

auto AbstractSidebarPage::getControl() -> Control* { return this->control; }

void AbstractSidebarPage::setTmpDisabled(bool disabled) {
    GdkCursor* cursor = nullptr;
    if (disabled) {
        cursor = gdk_cursor_new_from_name("watch", nullptr);
    }

    if (auto* surface = gtk_native_get_surface(gtk_widget_get_native(this->getWidget())); surface) {
        gdk_surface_set_cursor(surface, cursor);
    }

    gtk_widget_set_sensitive(this->getWidget(), !disabled);


    if (cursor) {
        g_object_unref(cursor);
    }
}
