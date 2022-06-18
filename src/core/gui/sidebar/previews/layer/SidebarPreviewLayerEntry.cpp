#include "SidebarPreviewLayerEntry.h"

#include <gdk/gdk.h>      // for GdkEvent, GDK_BUTTON_PRESS, GdkEve...
#include <glib-object.h>  // for G_CALLBACK, g_signal_connect, g_si...

#include "gui/Shadow.h"  // for Shadow

#include "SidebarPreviewLayers.h"  // for SidebarPreviewLayers


SidebarPreviewLayerEntry::SidebarPreviewLayerEntry(SidebarPreviewLayers* sidebar, const PageRef& page,
                                                   Layer::Index layerId, const std::string& layerName, size_t index,
                                                   bool stacked):
        SidebarPreviewBaseEntry(sidebar, page),
        sidebar(sidebar),
        index(index),
        layerId(layerId),
        box(gtk_box_new(GTK_ORIENTATION_VERTICAL, 2)),
        stacked(stacked) {

    const auto clickCallback = G_CALLBACK(+[](GtkWidget* widget, GdkEvent* event, SidebarPreviewLayerEntry* self) {
        // Open context menu on right mouse click
        if (event->type == GDK_BUTTON_PRESS) {
            auto mouseEvent = reinterpret_cast<GdkEventButton*>(event);
            if (mouseEvent->button == 3) {
                self->mouseButtonPressCallback();
                self->sidebar->openPreviewContextMenu();
                return true;
            }
        }
        return false;
    });
    g_signal_connect_after(this->widget, "button-press-event", clickCallback, this);

    GtkWidget* toolbar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);

    cbVisible = gtk_check_button_new_with_label(layerName.c_str());

    g_signal_connect(cbVisible, "toggled", G_CALLBACK(+[](GtkToggleButton* source, SidebarPreviewLayerEntry* self) {
                         self->checkboxToggled();
                     }),
                     this);


    // Left padding
    gtk_widget_set_margin_start(cbVisible, Shadow::getShadowTopLeftSize());

    gtk_container_add(GTK_CONTAINER(toolbar), cbVisible);

    gtk_widget_set_vexpand(widget, false);
    gtk_container_add(GTK_CONTAINER(box), widget);

    gtk_widget_set_vexpand(toolbar, false);
    gtk_container_add(GTK_CONTAINER(box), toolbar);

    gtk_widget_show_all(box);

    toolbarHeight = gtk_widget_get_allocated_height(cbVisible) + Shadow::getShadowTopLeftSize() + 20;
}

SidebarPreviewLayerEntry::~SidebarPreviewLayerEntry() {
    gtk_widget_destroy(this->box);
    this->box = nullptr;
}

void SidebarPreviewLayerEntry::checkboxToggled() {
    if (inUpdate) {
        return;
    }

    bool check = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(cbVisible));
    (dynamic_cast<SidebarPreviewLayers*>(sidebar))->layerVisibilityChanged(layerId, check);
}

void SidebarPreviewLayerEntry::mouseButtonPressCallback() {
    (dynamic_cast<SidebarPreviewLayers*>(sidebar))->layerSelected(index);
}

auto SidebarPreviewLayerEntry::getRenderType() -> PreviewRenderType {
    return stacked ? RENDER_TYPE_PAGE_LAYERSTACK : RENDER_TYPE_PAGE_LAYER;
}

auto SidebarPreviewLayerEntry::getHeight() -> int { return getWidgetHeight() + toolbarHeight; }

auto SidebarPreviewLayerEntry::getLayer() const -> Layer::Index { return layerId; }

auto SidebarPreviewLayerEntry::getWidget() -> GtkWidget* { return this->box; }

/**
 * Set the value of the visible checkbox
 */
void SidebarPreviewLayerEntry::setVisibleCheckbox(bool enabled) {
    inUpdate = true;

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cbVisible), enabled);

    inUpdate = false;
}
