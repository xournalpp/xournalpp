#include "Sidebar.h"

#include <cstdint>  // for int64_t
#include <memory>   // for std::make_unique and std::make_shared
#include <string>   // for string

#include <config-features.h>
#include <gdk/gdk.h>      // for gdk_display_get...
#include <glib-object.h>  // for G_CALLBACK, g_s...
#include <gtk/gtk.h>      // for gtk_toggle_tool_button_new...

#include "control/Control.h"                         // for Control
#include "control/settings/Settings.h"               // for Settings
#include "gui/GladeGui.h"                            // for GladeGui
#include "gui/sidebar/AbstractSidebarPage.h"         // for AbstractSidebar...
#include "gui/sidebar/indextree/SidebarIndexPage.h"  // for SidebarIndexPage
#include "model/Document.h"                          // for Document
#include "model/XojPage.h"                           // for XojPage
#include "pdf/base/XojPdfPage.h"                     // for XojPdfPageSPtr
#include "previews/layer/SidebarPreviewLayers.h"     // for SidebarPreviewL...
#include "previews/page/SidebarPreviewPages.h"       // for SidebarPreviewP...
#include "util/Util.h"                               // for npos
#include "util/glib_casts.h"                         // for closure_notify_cb
#include "util/i18n.h"                               // for _, FC, _F

Sidebar::Sidebar(GladeGui* gui, Control* control): control(control) {
    this->tbSelectTab = GTK_BOX(gui->get("bxSidebarTopActions"));
    this->buttonCloseSidebar = gui->get("buttonCloseSidebar");

    this->sidebarContents = gui->get("sidebarContents");

    this->initTabs(sidebarContents);

    GtkPaned* paned = GTK_PANED(gui->get("panedMainContents"));
    // For some reason, putting those in the .ui file doesn't work
    gtk_paned_set_shrink_start_child(paned, false);
    gtk_paned_set_shrink_end_child(paned, false);
    gtk_paned_set_resize_start_child(paned, false);
    gtk_paned_set_resize_end_child(paned, true);

    registerListener(control);
}

void Sidebar::initTabs(GtkWidget* sidebarContents) {
    addTab(std::make_unique<SidebarIndexPage>(this->control));
    addTab(std::make_unique<SidebarPreviewPages>(this->control));
    addTab(std::make_unique<SidebarPreviewLayers>(this->control, false));
    addTab(std::make_unique<SidebarPreviewLayers>(this->control, true));

    // Init toolbar with icons

    size_t i = 0;
    for (auto&& p: this->tabs) {
        GtkWidget* btn = gtk_toggle_button_new();
        p->tabButton = btn;

        gtk_button_set_icon_name(GTK_BUTTON(btn), p->getIconName().c_str());
        g_signal_connect_data(btn, "clicked", G_CALLBACK(&buttonClicked), new SidebarTabButton(this, i, p.get()),
                              xoj::util::closure_notify_cb<SidebarTabButton>, GConnectFlags(0));
        gtk_widget_set_tooltip_text(btn, p->getName().c_str());
        gtk_box_append(tbSelectTab, btn);

        // Add widget to sidebar
        gtk_widget_set_vexpand(p->getWidget(), true);
        gtk_box_append(GTK_BOX(sidebarContents), p->getWidget());

        i++;
    }

    updateVisibleTabs();
}

void Sidebar::buttonClicked(GtkButton* button, SidebarTabButton* buttonData) {
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button))) {
        if (buttonData->sidebar->visibleTab != buttonData->page->getWidget()) {
            buttonData->sidebar->setSelectedTab(buttonData->index);
        }
    } else if (buttonData->sidebar->visibleTab == buttonData->page->getWidget()) {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), true);
    }
}

void Sidebar::addTab(std::unique_ptr<AbstractSidebarPage> tab) { this->tabs.push_back(std::move(tab)); }

Sidebar::~Sidebar() = default;

void Sidebar::selectPageNr(size_t page, size_t pdfPage) {
    for (auto&& p: this->tabs) {
        p->selectPageNr(page, pdfPage);
    }
}

void Sidebar::setSelectedTab(size_t tab) {
    this->visibleTab = nullptr;

    size_t i = 0;
    for (auto&& t: this->tabs) {
        if (tab == i) {
            gtk_widget_show(t->getWidget());
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(t->tabButton), true);
            this->visibleTab = t->getWidget();
            this->currentTabIdx = i;
            t->enableSidebar();
        } else {
            t->disableSidebar();
            gtk_widget_hide(t->getWidget());
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(t->tabButton), false);
        }

        i++;
    }
}

void Sidebar::updateVisibleTabs() {
    size_t i = 0;
    size_t selected = npos;

    for (auto&& p: this->tabs) {
        gtk_widget_set_visible(GTK_WIDGET(p->tabButton), p->hasData());

        if (p->hasData() && selected == npos) {
            selected = i;
        }

        i++;
    }

    setSelectedTab(selected);
}

void Sidebar::setTmpDisabled(bool disabled) {
    gtk_widget_set_sensitive(this->buttonCloseSidebar, !disabled);
    gtk_widget_set_sensitive(GTK_WIDGET(this->tbSelectTab), !disabled);

    for (auto&& t: this->tabs) {
        t->setTmpDisabled(disabled);
    }

    gdk_display_sync(gdk_display_get_default());
}

void Sidebar::saveSize() {
    GtkAllocation alloc;
    gtk_widget_get_allocation(this->sidebarContents, &alloc);

    this->control->getSettings()->setSidebarWidth(alloc.width);
}

size_t Sidebar::getNumberOfTabs() const { return this->tabs.size(); }

size_t Sidebar::getSelectedTab() const { return this->currentTabIdx; }

auto Sidebar::getControl() -> Control* { return this->control; }

void Sidebar::documentChanged(DocumentChangeType type) {
    if (type == DOCUMENT_CHANGE_CLEARED || type == DOCUMENT_CHANGE_COMPLETE || type == DOCUMENT_CHANGE_PDF_BOOKMARKS) {
        updateVisibleTabs();
    }
}

SidebarTabButton::SidebarTabButton(Sidebar* sidebar, size_t index, AbstractSidebarPage* page):
        sidebar(sidebar), index(index), page(page) {}

void Sidebar::updatePageNumberingStyle() {
    auto it = std::find_if(tabs.begin(), tabs.end(),
                           [](const auto& t) { return dynamic_cast<SidebarPreviewPages*>(t.get()); });
    if (it != tabs.end()) {
        static_cast<SidebarPreviewPages*>(it->get())->updatePageNumberingStyle();
    }
}
