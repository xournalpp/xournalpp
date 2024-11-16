#include "AbstractSidebarPage.h"

#include <gdk/gdk.h>      // for gdk_cursor_new_for_display, gdk_display_get...
#include <glib-object.h>  // for g_object_unref

AbstractSidebarPage::AbstractSidebarPage(Control* control): control(control) {}

AbstractSidebarPage::~AbstractSidebarPage() = default;

void AbstractSidebarPage::selectPageNr(size_t page, size_t pdfPage) {}

auto AbstractSidebarPage::getControl() -> Control* { return this->control; }

void AbstractSidebarPage::setTmpDisabled(bool disabled) {
    if (disabled) {
        gtk_widget_set_cursor_from_name(this->getWidget(), "wait");
    } else {
        gtk_widget_set_cursor(this->getWidget(), nullptr);
    }

    gtk_widget_set_sensitive(this->getWidget(), !disabled);
}
