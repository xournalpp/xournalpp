#include "SidebarPreviewBase.h"

#include <cstdlib>  // for abs, size_t

#include <glib-object.h>  // for g_object_ref, G_CALLBACK, g_sig...
#include <glib.h>         // for g_idle_add

#include "control/Control.h"   // for Control
#include "control/PdfCache.h"  // for PdfCache
#include "gui/Builder.h"       // for Builder
#include "gui/MainWindow.h"    // for MainWindow
#include "model/Document.h"    // for Document
#include "util/Util.h"         // for npos
#include "util/glib_casts.h"   // for wrap_for_once_v

#include "SidebarLayout.h"            // for SidebarLayout
#include "SidebarPreviewBaseEntry.h"  // for SidebarPreviewBaseEntry

constexpr auto XML_FILE = "sidebar.ui";

SidebarPreviewBase::SidebarPreviewBase(Control* control, const char* menuId, const char* toolbarId):
        AbstractSidebarPage(control),
        scrollableBox(gtk_scrolled_window_new(), xoj::util::adopt),
        mainBox(gtk_box_new(GTK_ORIENTATION_VERTICAL, 0), xoj::util::adopt),
        miniaturesContainer(GTK_FIXED(gtk_fixed_new()), xoj::util::adopt) {
    gtk_box_append(GTK_BOX(mainBox.get()), scrollableBox.get());
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollableBox.get()), GTK_POLICY_AUTOMATIC,
                                   GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrollableBox.get()), GTK_WIDGET(miniaturesContainer.get()));
    gtk_widget_set_vexpand(scrollableBox.get(), true);

    Document* doc = this->control->getDocument();
    doc->lock();
    if (doc->getPdfPageCount() != 0) {
        this->cache = std::make_unique<PdfCache>(doc->getPdfDocument(), control->getSettings());
    }
    doc->unlock();

    registerListener(this->control);
    this->control->addChangedDocumentListener(this);

    auto* adj = gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(scrollableBox.get()));
    g_signal_connect(
            adj, "notify::page-size", G_CALLBACK(+[](GObject* adj, GParamSpec*, gpointer d) {
                static_cast<SidebarPreviewBase*>(d)->newWidth(gtk_adjustment_get_page_size(GTK_ADJUSTMENT(adj)));
            }),
            this);

    Builder builder(control->getGladeSearchPath(), XML_FILE);
    GMenuModel* menu = G_MENU_MODEL(builder.getObject(menuId));
    contextMenu.reset(GTK_POPOVER(gtk_popover_menu_new_from_model(menu)), xoj::util::adopt);
    gtk_widget_set_parent(GTK_WIDGET(contextMenu.get()), GTK_WIDGET(miniaturesContainer.get()));

    gtk_box_append(GTK_BOX(mainBox.get()), builder.get(toolbarId));
}

SidebarPreviewBase::~SidebarPreviewBase() { this->control->removeChangedDocumentListener(this); }

void SidebarPreviewBase::enableSidebar() { enabled = true; }

void SidebarPreviewBase::disableSidebar() { enabled = false; }

void SidebarPreviewBase::newWidth(double width) {
    static constexpr double TRIGGER = 20.;

    if (std::abs(lastWidth - width) > TRIGGER) {
        this->layout();
        lastWidth = width;
    }
}

auto SidebarPreviewBase::getZoom() const -> double { return this->zoom; }

auto SidebarPreviewBase::getCache() -> PdfCache* { return this->cache.get(); }

void SidebarPreviewBase::layout() { SidebarLayout::layout(this); }

auto SidebarPreviewBase::hasData() -> bool { return true; }

auto SidebarPreviewBase::getWidget() -> GtkWidget* { return this->mainBox.get(); }

void SidebarPreviewBase::documentChanged(DocumentChangeType type) {
    if (type == DOCUMENT_CHANGE_COMPLETE || type == DOCUMENT_CHANGE_CLEARED) {
        this->cache.reset();

        Document* doc = control->getDocument();
        doc->lock();
        if (doc->getPdfPageCount() != 0) {
            this->cache = std::make_unique<PdfCache>(doc->getPdfDocument(), control->getSettings());
        }
        doc->unlock();
        updatePreviews();
    }
}

auto SidebarPreviewBase::scrollToPreview(SidebarPreviewBase* sidebar) -> bool {
    if (!sidebar->enabled) {
        return false;
    }

    MainWindow* win = sidebar->control->getWindow();
    if (win == nullptr) {
        return false;
    }

    GtkWidget* w = win->get("sidebar");
    if (!gtk_widget_get_visible(w)) {
        return false;
    }

    if (sidebar->selectedEntry != npos && sidebar->selectedEntry < sidebar->previews.size()) {
        auto& p = sidebar->previews[sidebar->selectedEntry];

        // scroll to preview
        GtkAdjustment* vadj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(sidebar->scrollableBox.get()));
        GtkWidget* widget = p->getWidget();

        GtkAllocation allocation;
        gtk_widget_get_allocation(widget, &allocation);
        int x = allocation.x;
        int y = allocation.y;

        if (x == -1) {
            g_idle_add(xoj::util::wrap_for_once_v<scrollToPreview>, sidebar);
            return false;
        }

        gtk_adjustment_clamp_page(vadj, y, y + allocation.height);
    }
    return false;
}

void SidebarPreviewBase::pageDeleted(size_t page) {}

void SidebarPreviewBase::pageInserted(size_t page) {}

void SidebarPreviewBase::openPreviewContextMenu(double x, double y, GtkWidget* entry) {
    double newX, newY;
    gtk_widget_translate_coordinates(entry, GTK_WIDGET(miniaturesContainer.get()), x, y, &newX, &newY);
    GdkRectangle r = {round_cast<int>(newX), round_cast<int>(newY), 0, 0};
    gtk_popover_set_pointing_to(this->contextMenu.get(), &r);
    gtk_popover_popup(this->contextMenu.get());
}
