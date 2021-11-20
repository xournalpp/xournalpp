#include "ToolPageLayer.h"

#include <utility>

#include <config.h>

#include "control/layer/LayerController.h"
#include "gui/GladeGui.h"
#include "gui/widgets/PopupMenuButton.h"
#include "util/i18n.h"

ToolPageLayer::ToolPageLayer(LayerController* lc, GladeGui* gui, ActionHandler* handler, std::string id,
                             ActionType type, IconNameHelper iconNameHelper):
        AbstractToolItem(std::move(id), handler, type, nullptr),
        lc(lc),
        gui(gui),
        menu(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6)),
        iconNameHelper(iconNameHelper) {
    this->layerLabel = gtk_label_new(_("Loading..."));
    this->layerButton = gtk_button_new_with_label("⌄");

    PangoAttrList* attrs = pango_attr_list_new();
    pango_attr_list_insert(attrs, pango_attr_weight_new(PANGO_WEIGHT_BOLD));
    gtk_label_set_attributes(GTK_LABEL(this->layerLabel), attrs);

    popupMenuButton = new PopupMenuButton(this->layerButton, menu);

    LayerCtrlListener::registerListener(lc);
}

ToolPageLayer::~ToolPageLayer() {
    delete popupMenuButton;
    popupMenuButton = nullptr;
}

void ToolPageLayer::rebuildLayerMenu() { updateMenu(); }

void ToolPageLayer::layerVisibilityChanged() { updateLayerData(); }

const int MENU_WIDTH = 3;

void ToolPageLayer::createSeparator() {
    gtk_box_append(GTK_BOX(menu), gtk_separator_new(GTK_ORIENTATION_VERTICAL));
    menuY++;
}

auto ToolPageLayer::createSpecialMenuEntry(const std::string& name) -> GtkWidget* {
    GtkWidget* lb = gtk_label_new(name.c_str());
    gtk_widget_set_halign(lb, GTK_ALIGN_START);

    PangoAttrList* attrs = pango_attr_list_new();
    pango_attr_list_insert(attrs, pango_attr_weight_new(PANGO_WEIGHT_BOLD));
    gtk_label_set_attributes(GTK_LABEL(lb), attrs);

    gtk_box_append(GTK_BOX(menu), lb);
    menuY++;

    return lb;
}

/**
 * Add special button to the top of the menu
 */
void ToolPageLayer::addSpecialButtonTop() {
    GtkWidget* itShowAll = createSpecialMenuEntry(_("Show all"));
    g_signal_connect(itShowAll, "activate",
                     G_CALLBACK(+[](GtkWidget* menu, ToolPageLayer* self) { self->lc->showAllLayer(); }), this);


    GtkWidget* itHideAll = createSpecialMenuEntry(_("Hide all"));
    g_signal_connect(itHideAll, "activate",
                     G_CALLBACK(+[](GtkWidget* menu, ToolPageLayer* self) { self->lc->hideAllLayer(); }), this);

    createSeparator();

    GtkWidget* itNewLayer = createSpecialMenuEntry(_("Create new layer"));
    g_signal_connect(itNewLayer, "activate",
                     G_CALLBACK(+[](GtkWidget* menu, ToolPageLayer* self) { self->lc->addNewLayer(); }), this);

    createSeparator();
}

void ToolPageLayer::selectLayer(int layerId) { lc->switchToLay(layerId); }

void ToolPageLayer::layerMenuClicked(GtkWidget* menu) {
    if (inMenuUpdate) {
        return;
    }

    int layerId = -1;

    for (auto& kv: layerItems) {
        if (kv.second == menu) {
            layerId = kv.first;
            break;
        }
    }

    if (layerId < 0) {
        g_warning("Invalid Layer Menu selected - not handled");
        return;
    }


    if (!gtk_check_button_get_active(GTK_CHECK_BUTTON(menu))) {
        if (layerId == static_cast<int>(lc->getCurrentLayerId())) {
            // This is the current layer, don't allow to deselect it
            inMenuUpdate = true;
            gtk_check_button_set_active(GTK_CHECK_BUTTON(menu), true);
            inMenuUpdate = false;
        }
        return;
    }

    selectLayer(layerId);
}

void ToolPageLayer::layerMenuShowClicked(GtkWidget* menu) {

    if (inMenuUpdate) {
        return;
    }

    int layerId = -1;

    for (auto& kv: showLayerItems) {
        if (kv.second == menu) {
            layerId = kv.first;
            break;
        }
    }

    if (layerId < 0) {
        g_warning("Invalid Layer Show Menu selected - not handled");
        return;
    }

    bool checked = gtk_check_button_get_active(GTK_CHECK_BUTTON(menu));

    lc->setLayerVisible(layerId, checked);
}

void ToolPageLayer::createLayerMenuItem(const std::string& text, int layerId) {
    GtkWidget* itLayer = gtk_check_button_new_with_label(text.c_str());
    gtk_box_append(GTK_BOX(menu), itLayer);

    g_signal_connect(itLayer, "activate",
                     G_CALLBACK(+[](GtkWidget* menu, ToolPageLayer* self) { self->layerMenuClicked(menu); }), this);

    layerItems[layerId] = itLayer;
}

void ToolPageLayer::createLayerMenuItemShow(int layerId) {
    GtkWidget* itShow = gtk_check_button_new_with_label(_("show"));
    gtk_box_append(GTK_BOX(menu), itShow);
    gtk_widget_set_hexpand(itShow, false);

    g_signal_connect(itShow, "activate",
                     G_CALLBACK(+[](GtkWidget* menu, ToolPageLayer* self) { self->layerMenuShowClicked(menu); }), this);

    showLayerItems[layerId] = itShow;
}

/**
 * Rebuild the Menu
 */
void ToolPageLayer::updateMenu() {
    /*
     * Remove all items from Menu (does sometimes not work)
     * gtk_container_foreach(GTK_CONTAINER(menu), (GtkCallback) gtk_widget_destroy, nullptr);
     */

    // Create a new menu on refresh
    menu = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    popupMenuButton->setMenu(menu);
    layerItems.clear();
    showLayerItems.clear();

    menuY = 0;

    addSpecialButtonTop();

    int layerCount = lc->getLayerCount();
    for (int layer = layerCount; layer > 0; layer--) {
        createLayerMenuItem(lc->getLayerNameById(layer), layer);
        createLayerMenuItemShow(layer);
        menuY++;
    }

    if (layerCount > 0) {
        createSeparator();
    }

    if (lc->getCurrentPage() == nullptr) {
        createLayerMenuItem(_("Background"), 0);
    } else {
        createLayerMenuItem(lc->getCurrentPage()->getBackgroundName(), 0);
    }
    createLayerMenuItemShow(0);

    menuY++;


    updateLayerData();
}

/**
 * Update selected layer, update visible layer
 */
void ToolPageLayer::updateLayerData() {
    auto layerId = lc->getCurrentLayerId();

    inMenuUpdate = true;

    for (auto& kv: layerItems) {
        if (kv.first == layerId) {
            gtk_check_button_set_active(GTK_CHECK_BUTTON(kv.second), true);
        } else {
            gtk_check_button_set_active(GTK_CHECK_BUTTON(kv.second), false);
        }
    }

    PageRef page = lc->getCurrentPage();

    if (page) {
        for (auto& kv: showLayerItems) {
            gtk_check_button_set_active(GTK_CHECK_BUTTON(kv.second), page->isLayerVisible(kv.first));
        }
    }

    inMenuUpdate = false;

    gtk_label_set_text(GTK_LABEL(layerLabel), lc->getCurrentLayerName().c_str());
}

auto ToolPageLayer::getToolDisplayName() const -> std::string { return _("Layer Combo"); }

auto ToolPageLayer::getNewToolIcon() const -> GtkWidget* {
    return gtk_image_new_from_icon_name(this->iconNameHelper.iconName("combo-layer").c_str());
}

auto ToolPageLayer::getNewToolPixbuf() const -> GdkPixbuf* { return getPixbufFromImageIconName(); }


auto ToolPageLayer::newItem() -> GtkWidget* {
    GtkWidget* hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_append(GTK_BOX(hbox), gtk_label_new(_("Layer")));
    gtk_box_append(GTK_BOX(hbox), this->layerLabel);
    gtk_box_append(GTK_BOX(hbox), this->layerButton);
    return hbox;
}
