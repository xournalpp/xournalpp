#include "Sidebar.h"

#include <cstdint>  // for int64_t
#include <memory>   // for std::make_unique and std::make_shared
#include <string>   // for string

#include <config-features.h>
#include <gdk/gdk.h>      // for gdk_display_get...
#include <glib-object.h>  // for G_CALLBACK, g_s...
#include <gtk/gtk.h>      // for gtk_dialog_add_...

#include "control/Control.h"                          // for Control
#include "control/settings/Settings.h"                // for Settings
#include "gui/GladeGui.h"                             // for GladeGui
#include "gui/sidebar/AbstractSidebarPage.h"          // for AbstractSidebar...
#include "gui/sidebar/indextree/SidebarIndexPage.h"   // for SidebarIndexPage
#include "model/Document.h"                           // for Document
#include "model/XojPage.h"                            // for XojPage
#include "pdf/base/XojPdfPage.h"                      // for XojPdfPageSPtr
#include "previews/layer/SidebarLayersContextMenu.h"  // for SidebarLayersCo...
#include "previews/layer/SidebarPreviewLayers.h"      // for SidebarPreviewL...
#include "previews/page/SidebarPreviewPages.h"        // for SidebarPreviewP...
#include "util/Util.h"                                // for npos
#include "util/i18n.h"                                // for _, FC, _F

Sidebar::Sidebar(GladeGui* gui, Control* control): control(control), gui(gui), toolbar(this, gui) {
    this->tbSelectPage = GTK_TOOLBAR(gui->get("tbSelectSidebarPage"));
    this->buttonCloseSidebar = gui->get("buttonCloseSidebar");

    this->sidebarContents = gui->get("sidebarContents");

    this->initPages(sidebarContents, gui);

    registerListener(control);
}

void Sidebar::initPages(GtkWidget* sidebarContents, GladeGui* gui) {
    addPage(std::make_unique<SidebarIndexPage>(this->control, &this->toolbar));
    addPage(std::make_unique<SidebarPreviewPages>(this->control, this->gui, &this->toolbar));
    auto layersContextMenu = std::make_shared<SidebarLayersContextMenu>(this->gui, &this->toolbar);
    addPage(std::make_unique<SidebarPreviewLayers>(this->control, this->gui, &this->toolbar, false, layersContextMenu));
    addPage(std::make_unique<SidebarPreviewLayers>(this->control, this->gui, &this->toolbar, true, layersContextMenu));

    // Init toolbar with icons

    int i = 0;
    for (auto&& p: this->pages) {
        GtkToolItem* it = gtk_toggle_tool_button_new();
        p->tabButton = it;

        gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(it), gtk_image_new_from_icon_name(p->getIconName().c_str(),
                                                                                          GTK_ICON_SIZE_SMALL_TOOLBAR));
        g_signal_connect(it, "clicked", G_CALLBACK(&buttonClicked), new SidebarPageButton(this, i, p.get()));
        gtk_tool_item_set_tooltip_text(it, p->getName().c_str());
        gtk_tool_button_set_label(GTK_TOOL_BUTTON(it), p->getName().c_str());

        gtk_toolbar_insert(tbSelectPage, it, -1);

        // Add widget to sidebar
        gtk_box_pack_start(GTK_BOX(sidebarContents), p->getWidget(), true, true, 0);

        i++;
    }

    gtk_widget_show_all(GTK_WIDGET(this->tbSelectPage));

    updateVisibleTabs();
}

void Sidebar::buttonClicked(GtkToolButton* toolbutton, SidebarPageButton* buttonData) {
    if (gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(toolbutton))) {
        if (buttonData->sidebar->visiblePage != buttonData->page->getWidget()) {
            buttonData->sidebar->setSelectedPage(buttonData->index);
        }
    } else if (buttonData->sidebar->visiblePage == buttonData->page->getWidget()) {
        gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(toolbutton), true);
    }
}

void Sidebar::addPage(std::unique_ptr<AbstractSidebarPage> page) { this->pages.push_back(std::move(page)); }

void Sidebar::askInsertPdfPage(size_t pdfPage) {
    GtkWidget* dialog = gtk_message_dialog_new(control->getGtkWindow(), GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION,
                                               GTK_BUTTONS_NONE, "%s",
                                               FC(_F("Your current document does not contain PDF Page no {1}\n"
                                                     "Would you like to insert this page?\n\n"
                                                     "Tip: You can select Journal → Paper Background → PDF Background "
                                                     "to insert a PDF page.") %
                                                  static_cast<int64_t>(pdfPage + 1)));

    using Responses = enum { CANCEL = 1, AFTER = 2, END = 3 };

    gtk_dialog_add_button(GTK_DIALOG(dialog), _("Cancel"), Responses::CANCEL);
    gtk_dialog_add_button(GTK_DIALOG(dialog), _("Insert after current page"), Responses::AFTER);
    gtk_dialog_add_button(GTK_DIALOG(dialog), _("Insert at end"), Responses::END);

    gtk_window_set_transient_for(GTK_WINDOW(dialog), control->getGtkWindow());
    int res = gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    if (res != Responses::AFTER && res != Responses::END) {
        return;
    }


    Document* doc = control->getDocument();

    size_t position = 0;
    doc->lock();
    if (res == Responses::AFTER) {
        position = control->getCurrentPageNo() + 1;
    } else if (res == Responses::END) {
        position = doc->getPageCount();
    }
    XojPdfPageSPtr pdf = doc->getPdfPage(pdfPage);
    doc->unlock();

    if (pdf) {
        auto page = std::make_shared<XojPage>(pdf->getWidth(), pdf->getHeight());
        page->setBackgroundPdfPageNr(pdfPage);
        control->insertPage(page, position);
    }
}

Sidebar::~Sidebar() = default;

/**
 * Called when an action is performed
 */
void Sidebar::actionPerformed(SidebarActions action) {
    if (this->currentPageIdx >= this->pages.size()) {
        return;
    }

    this->pages.at(this->currentPageIdx)->actionPerformed(action);
}

void Sidebar::selectPageNr(size_t page, size_t pdfPage) {
    for (auto&& p: this->pages) {
        p->selectPageNr(page, pdfPage);
    }
}

void Sidebar::setSelectedPage(size_t page) {
    this->visiblePage = nullptr;

    size_t i = 0;
    for (auto&& p: this->pages) {
        if (page == i) {
            gtk_widget_show(p->getWidget());
            gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(p->tabButton), true);
            this->visiblePage = p->getWidget();
            this->currentPageIdx = i;
            p->enableSidebar();
        } else {
            p->disableSidebar();
            gtk_widget_hide(p->getWidget());
            gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(p->tabButton), false);
        }

        i++;
    }
}

void Sidebar::updateVisibleTabs() {
    size_t i = 0;
    size_t selected = npos;

    for (auto&& p: this->pages) {
        gtk_widget_set_visible(GTK_WIDGET(p->tabButton), p->hasData());

        if (p->hasData() && selected == npos) {
            selected = i;
        }

        i++;
    }

    setSelectedPage(selected);
}

void Sidebar::setTmpDisabled(bool disabled) {
    gtk_widget_set_sensitive(this->buttonCloseSidebar, !disabled);
    gtk_widget_set_sensitive(GTK_WIDGET(this->tbSelectPage), !disabled);

    for (auto&& p: this->pages) {
        p->setTmpDisabled(disabled);
    }

    gdk_display_sync(gdk_display_get_default());
}

void Sidebar::saveSize() {
    GtkAllocation alloc;
    gtk_widget_get_allocation(this->sidebarContents, &alloc);

    this->control->getSettings()->setSidebarWidth(alloc.width);
}

auto Sidebar::getToolbar() -> SidebarToolbar* { return &this->toolbar; }

auto Sidebar::getControl() -> Control* { return this->control; }

void Sidebar::documentChanged(DocumentChangeType type) {
    if (type == DOCUMENT_CHANGE_CLEARED || type == DOCUMENT_CHANGE_COMPLETE || type == DOCUMENT_CHANGE_PDF_BOOKMARKS) {
        updateVisibleTabs();
    }
}

SidebarPageButton::SidebarPageButton(Sidebar* sidebar, int index, AbstractSidebarPage* page):
        sidebar(sidebar), index(index), page(page) {}

void Sidebar::layout() {
    for (auto&& page: this->pages) {
        page->layout();
    }
}
