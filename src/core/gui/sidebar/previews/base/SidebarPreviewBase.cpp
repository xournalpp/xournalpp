#include "SidebarPreviewBase.h"

#include <cstdlib>  // for abs, size_t

#include <glib-object.h>  // for g_object_ref, G_CALLBACK, g_sig...
#include <glib.h>         // for g_idle_add, GSourceFunc

#include "control/Control.h"   // for Control
#include "control/PdfCache.h"  // for PdfCache
#include "gui/MainWindow.h"    // for MainWindow
#include "model/Document.h"    // for Document
#include "util/Util.h"         // for npos

#include "SidebarLayout.h"            // for SidebarLayout
#include "SidebarPreviewBaseEntry.h"  // for SidebarPreviewBaseEntry

class GladeGui;


SidebarPreviewBase::SidebarPreviewBase(Control* control, GladeGui* gui, SidebarToolbar* toolbar):
        AbstractSidebarPage(control, toolbar) {
    this->layoutmanager = new SidebarLayout();

    Document* doc = this->control->getDocument();
    doc->lock();
    if (doc->getPdfPageCount() != 0) {
        this->cache = std::make_unique<PdfCache>(doc->getPdfDocument(), control->getSettings());
    }
    doc->unlock();

    this->iconViewPreview = gtk_layout_new(nullptr, nullptr);
    g_object_ref(this->iconViewPreview);

    this->scrollPreview = gtk_scrolled_window_new(nullptr, nullptr);
    g_object_ref(this->scrollPreview);

    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(this->scrollPreview), GTK_POLICY_AUTOMATIC,
                                   GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(this->scrollPreview), GTK_SHADOW_IN);

    gtk_container_add(GTK_CONTAINER(this->scrollPreview), this->iconViewPreview);
    gtk_widget_show(this->scrollPreview);

    gtk_widget_show(this->iconViewPreview);

    registerListener(this->control);
    this->control->addChangedDocumentListener(this);

    g_signal_connect(this->scrollPreview, "size-allocate", G_CALLBACK(sizeChanged), this);

    gtk_widget_show_all(this->scrollPreview);
}

SidebarPreviewBase::~SidebarPreviewBase() {
    gtk_widget_destroy(this->iconViewPreview);
    this->iconViewPreview = nullptr;

    delete this->layoutmanager;
    this->layoutmanager = nullptr;

    this->scrollPreview = nullptr;

    this->control->removeChangedDocumentListener(this);

    this->previews.clear();
}

void SidebarPreviewBase::enableSidebar() { enabled = true; }

void SidebarPreviewBase::disableSidebar() { enabled = false; }

void SidebarPreviewBase::sizeChanged(GtkWidget* widget, GtkAllocation* allocation, SidebarPreviewBase* sidebar) {
    static int lastWidth = -1;

    if (lastWidth == -1) {
        lastWidth = allocation->width;
    }

    if (std::abs(lastWidth - allocation->width) > 20) {
        sidebar->layout();
        lastWidth = allocation->width;
    }
}

auto SidebarPreviewBase::getZoom() const -> double { return this->zoom; }

auto SidebarPreviewBase::getCache() -> PdfCache* { return this->cache.get(); }

void SidebarPreviewBase::layout() { SidebarLayout::layout(this); }

auto SidebarPreviewBase::hasData() -> bool { return true; }

auto SidebarPreviewBase::getWidget() -> GtkWidget* { return this->scrollPreview; }

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
        GtkAdjustment* hadj = gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(sidebar->scrollPreview));
        GtkAdjustment* vadj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(sidebar->scrollPreview));
        GtkWidget* widget = p->getWidget();

        GtkAllocation allocation;
        gtk_widget_get_allocation(widget, &allocation);
        int x = allocation.x;
        int y = allocation.y;

        if (x == -1) {
            g_idle_add(reinterpret_cast<GSourceFunc>(scrollToPreview), sidebar);
            return false;
        }

        gtk_adjustment_clamp_page(vadj, y, y + allocation.height);
        gtk_adjustment_clamp_page(hadj, x, x + allocation.width);
    }
    return false;
}

void SidebarPreviewBase::pageDeleted(size_t page) {}

void SidebarPreviewBase::pageInserted(size_t page) {}
