#include "ToolPageLayer.h"

#include <algorithm>  // for find, max
#include <cstddef>    // for size_t
#include <iterator>   // for distance
#include <memory>     // for operator==, __share...
#include <utility>    // for move

#include <glib-object.h>  // for G_CALLBACK, g_signa...
#include <glib.h>         // for g_warning
#include <pango/pango.h>  // for pango_attr_list_insert

#include "control/layer/LayerController.h"        // for LayerController
#include "gui/toolbarMenubar/AbstractToolItem.h"  // for AbstractToolItem
#include "gui/widgets/PopupMenuButton.h"          // for PopupMenuButton
#include "model/PageRef.h"                        // for PageRef
#include "model/XojPage.h"                        // for XojPage
#include "util/i18n.h"                            // for _

class ActionHandler;

ToolPageLayer::ToolPageLayer(LayerController* lc, ActionHandler* handler, std::string id, ActionType type,
                             IconNameHelper iconNameHelper):
        AbstractToolItem(std::move(id), handler, type, nullptr),
        lc(lc),
        menu(gtk_menu_new()),
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
    gtk_menu_attach(GTK_MENU(menu), gtk_separator_menu_item_new(), 0, MENU_WIDTH, menuY, menuY + 1);
    menuY++;
}

auto ToolPageLayer::createSpecialMenuEntry(const std::string& name) -> GtkWidget* {
    GtkWidget* it = gtk_menu_item_new();
    GtkWidget* lb = gtk_label_new(name.c_str());
    gtk_widget_set_halign(lb, GTK_ALIGN_START);

    PangoAttrList* attrs = pango_attr_list_new();
    pango_attr_list_insert(attrs, pango_attr_weight_new(PANGO_WEIGHT_BOLD));
    gtk_label_set_attributes(GTK_LABEL(lb), attrs);

    gtk_container_add(GTK_CONTAINER(it), lb);
    gtk_menu_attach(GTK_MENU(menu), it, 0, MENU_WIDTH, menuY, menuY + 1);
    menuY++;

    return it;
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

void ToolPageLayer::selectLayer(Layer::Index layerId) { lc->switchToLay(layerId); }

void ToolPageLayer::layerMenuClicked(GtkWidget* menu) {
    if (inMenuUpdate) {
        return;
    }

    auto it = std::find(layerItems.begin(), layerItems.end(), menu);

    if (it == layerItems.end()) {
        g_warning("Invalid Layer Menu selected - not handled");
        return;
    }
    auto layerId = static_cast<Layer::Index>(std::distance(layerItems.begin(), it));

    if (!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menu))) {
        if (layerId == lc->getCurrentLayerId()) {
            // This is the current layer, don't allow to deselect it
            inMenuUpdate = true;
            gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menu), true);
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

    auto it = std::find(showLayerItems.begin(), showLayerItems.end(), menu);

    if (it == showLayerItems.end()) {
        g_warning("Invalid Layer Show Menu selected - not handled");
        return;
    }
    size_t layerId = (size_t)std::distance(showLayerItems.begin(), it);

    bool checked = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menu));

    lc->setLayerVisible(layerId, checked);
}

void ToolPageLayer::createLayerMenuItem(const std::string& text, Layer::Index layerId) {
    GtkWidget* itLayer = gtk_check_menu_item_new_with_label(text.c_str());
    gtk_check_menu_item_set_draw_as_radio(GTK_CHECK_MENU_ITEM(itLayer), true);
    gtk_menu_attach(GTK_MENU(menu), itLayer, 0, 2, menuY, menuY + 1);

    g_signal_connect(itLayer, "activate",
                     G_CALLBACK(+[](GtkWidget* menu, ToolPageLayer* self) { self->layerMenuClicked(menu); }), this);

    layerItems[layerId] = itLayer;
}

void ToolPageLayer::createLayerMenuItemShow(Layer::Index layerId) {
    GtkWidget* itShow = gtk_check_menu_item_new_with_label(_("show"));
    gtk_menu_attach(GTK_MENU(menu), itShow, 2, 3, menuY, menuY + 1);
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
    menu = gtk_menu_new();
    popupMenuButton->setMenu(menu);
    layerItems.clear();
    showLayerItems.clear();

    menuY = 0;

    addSpecialButtonTop();

    auto layerCount = lc->getLayerCount();
    layerItems.resize(layerCount + 1, nullptr);
    showLayerItems.resize(layerCount + 1, nullptr);
    for (auto layer = layerCount; layer > 0; layer--) {
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

    gtk_widget_show_all(menu);

    updateLayerData();
}

/**
 * Update selected layer, update visible layer
 */
void ToolPageLayer::updateLayerData() {
    auto layerId = lc->getCurrentLayerId();

    inMenuUpdate = true;

    for (GtkWidget* widgetPtr: layerItems) {
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(widgetPtr), false);
    }
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(layerItems[layerId]), true);

    PageRef page = lc->getCurrentPage();

    if (page) {
        Layer::Index n = 0;
        for (GtkWidget* widgetPtr: showLayerItems) {
            gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(widgetPtr), page->isLayerVisible(n++));
        }
    }

    inMenuUpdate = false;

    gtk_label_set_text(GTK_LABEL(layerLabel), lc->getCurrentLayerName().c_str());
}

auto ToolPageLayer::getToolDisplayName() const -> std::string { return _("Layer Combo"); }

auto ToolPageLayer::getNewToolIcon() const -> GtkWidget* {
    return gtk_image_new_from_icon_name(this->iconNameHelper.iconName("combo-layer").c_str(),
                                        GTK_ICON_SIZE_SMALL_TOOLBAR);
}

auto ToolPageLayer::getNewToolPixbuf() const -> GdkPixbuf* { return getPixbufFromImageIconName(); }


auto ToolPageLayer::newItem() -> GtkToolItem* {
    GtkToolItem* it = gtk_tool_item_new();

    GtkWidget* hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
    gtk_box_pack_start(GTK_BOX(hbox), gtk_label_new(_("Layer")), false, false, 7);

    gtk_box_pack_start(GTK_BOX(hbox), this->layerLabel, false, false, 0);
    gtk_box_pack_start(GTK_BOX(hbox), this->layerButton, false, false, 0);

    gtk_container_add(GTK_CONTAINER(it), hbox);

    return it;
}
