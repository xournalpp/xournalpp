#include "SidebarPreviewLayerEntry.h"

#include <gdk/gdk.h>      // for GdkEvent, GDK_BUTTON_PRESS, GdkEve...
#include <glib-object.h>  // for G_CALLBACK, g_signal_connect, g_si...

#include "SidebarPreviewLayers.h"  // for SidebarPreviewLayers


SidebarPreviewLayerEntry::SidebarPreviewLayerEntry(SidebarPreviewLayers* sidebar, const PageRef& page,
                                                   Layer::Index layerId, const std::string& layerName, bool stacked):
        SidebarPreviewBaseEntry(sidebar, page),
        sidebar(sidebar),
        layerId(layerId),
        box(gtk_box_new(GTK_ORIENTATION_VERTICAL, 4), xoj::util::adopt),
        stacked(stacked) {
#if GTK_CHECK_VERSION(4, 8, 0)
    cbVisible = gtk_check_button_new();
    GtkWidget* lbl = gtk_label_new(layerName.c_str());
    gtk_label_set_ellipsize(GTK_LABEL(lbl), PANGO_ELLIPSIZE_END);
    gtk_check_button_set_child(GTK_CHECK_BUTTON(cbVisible), lbl);
#else
    cbVisible = gtk_check_button_new_with_label(layerName.c_str());
#endif

    callbackId = g_signal_connect(
            cbVisible, "toggled", G_CALLBACK(+[](GtkCheckButton* btn, gpointer d) {
                auto* self = static_cast<SidebarPreviewLayerEntry*>(d);
                bool check = gtk_check_button_get_active(btn);
                (dynamic_cast<SidebarPreviewLayers*>(self->sidebar))->layerVisibilityChanged(self->layerId, check);
            }),
            this);

    gtk_widget_set_margin_start(cbVisible, 2);
    gtk_box_append(GTK_BOX(box.get()), this->button.get());
    gtk_box_append(GTK_BOX(box.get()), cbVisible);
}

SidebarPreviewLayerEntry::~SidebarPreviewLayerEntry() {
    GtkWidget* w = this->getWidget();
    gtk_fixed_remove(GTK_FIXED(gtk_widget_get_parent(w)), w);
}

void SidebarPreviewLayerEntry::mouseButtonPressCallback() {
    (dynamic_cast<SidebarPreviewLayers*>(sidebar))->layerSelected(layerId);
}

auto SidebarPreviewLayerEntry::getRenderType() const -> PreviewRenderType {
    return stacked ? RENDER_TYPE_PAGE_LAYERSTACK : RENDER_TYPE_PAGE_LAYER;
}

auto SidebarPreviewLayerEntry::getLayer() const -> Layer::Index { return layerId; }

auto SidebarPreviewLayerEntry::getWidget() const -> GtkWidget* { return this->box.get(); }

/**
 * Set the value of the visible checkbox
 */
void SidebarPreviewLayerEntry::setVisibleCheckbox(bool enabled) {
    g_signal_handler_block(cbVisible, callbackId);
    gtk_check_button_set_active(GTK_CHECK_BUTTON(cbVisible), enabled);
    g_signal_handler_unblock(cbVisible, callbackId);
}
