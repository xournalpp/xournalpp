#include "BackgroundSelectDialogBase.h"

#include <algorithm>  // for max
#include <utility>    // for move

#include <gdk/gdk.h>      // for GDK_EXPOSURE_MASK
#include <glib-object.h>  // for G_CALLBACK, g_signal_connect

#include "util/Util.h"  // for paintBackgroundWhite

#include "BaseElementView.h"  // for BaseElementView

class GladeSearchpath;


BackgroundSelectDialogBase::BackgroundSelectDialogBase(GladeSearchpath* gladeSearchPath, Document* doc,
                                                       Settings* settings, const std::string& glade,
                                                       const std::string& mainWnd):
        GladeGui(gladeSearchPath, glade, std::move(mainWnd)), settings(settings), doc(doc) {
    this->layoutContainer = gtk_layout_new(nullptr, nullptr);
    gtk_widget_show(this->layoutContainer);
    this->scrollPreview = get("scrollContents");
    gtk_container_add(GTK_CONTAINER(scrollPreview), layoutContainer);

    gtk_widget_set_events(this->layoutContainer, GDK_EXPOSURE_MASK);
    g_signal_connect(this->layoutContainer, "draw", G_CALLBACK(Util::paintBackgroundWhite), nullptr);

    g_signal_connect(this->window, "size-allocate", G_CALLBACK(sizeAllocate), this);
    gtk_window_set_default_size(GTK_WINDOW(this->window), 800, 600);
}

BackgroundSelectDialogBase::~BackgroundSelectDialogBase() {
    for (BaseElementView* e: elements) { delete e; }
    elements.clear();
}

void BackgroundSelectDialogBase::sizeAllocate(GtkWidget* widget, GtkRequisition* requisition,
                                              BackgroundSelectDialogBase* dlg) {
    GtkAllocation alloc = {0};
    gtk_widget_get_allocation(dlg->scrollPreview, &alloc);
    if (dlg->lastWidth == alloc.width) {
        return;
    }
    dlg->lastWidth = alloc.width;
    dlg->layout();
}

auto BackgroundSelectDialogBase::getSettings() -> Settings* { return this->settings; }

void BackgroundSelectDialogBase::layout() {
    double x = 0;
    double y = 0;
    double row_height = 0;
    double max_row_width = 0;

    GtkAllocation alloc = {0};
    gtk_widget_get_allocation(this->scrollPreview, &alloc);

    for (BaseElementView* p: this->elements) {
        if (!gtk_widget_get_visible(p->getWidget())) {
            continue;
        }

        if (x + p->getWidth() > alloc.width) {
            y += row_height;
            x = 0;
            row_height = 0;
        }
        max_row_width = std::max(max_row_width, x + static_cast<double>(p->getWidth()));

        gtk_layout_move(GTK_LAYOUT(this->layoutContainer), p->getWidget(), x, y);

        row_height =
                std::max(row_height,
                         static_cast<double>(p->getHeight()));  // TODO(fabian): page height should be double as well

        x += p->getWidth();
    }

    gtk_layout_set_size(GTK_LAYOUT(this->layoutContainer), max_row_width, y + row_height);
}

void BackgroundSelectDialogBase::show(GtkWindow* parent) {
    for (BaseElementView* e: elements) { gtk_layout_put(GTK_LAYOUT(this->layoutContainer), e->getWidget(), 0, 0); }

    if (!elements.empty()) {
        setSelected(0);
    }

    layout();

    gtk_window_set_transient_for(GTK_WINDOW(this->window), parent);
    gtk_dialog_run(GTK_DIALOG(this->window));
    gtk_widget_hide(this->window);
}

void BackgroundSelectDialogBase::setSelected(int selected) {
    if (this->selected == selected) {
        return;
    }

    int lastSelected = this->selected;
    if (lastSelected >= 0 && lastSelected < static_cast<int>(elements.size())) {
        elements[lastSelected]->setSelected(false);
    }

    if (selected >= 0 && selected < static_cast<int>(elements.size())) {
        elements[selected]->setSelected(true);
        this->selected = selected;
    }
}
