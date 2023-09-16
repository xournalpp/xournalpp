#include "ToolPageLayer.h"

#include <utility>  // for move

#include <glib-object.h>  // for G_CALLBACK, g_signa...
#include <glib.h>         // for g_warning
#include <pango/pango.h>  // for pango_attr_list_insert

#include "control/layer/LayerController.h"        // for LayerController
#include "enums/Action.enum.h"                    // for Action
#include "gui/IconNameHelper.h"                   // for IconNameHelper
#include "gui/toolbarMenubar/AbstractToolItem.h"  // for AbstractToolItem
#include "model/PageRef.h"                        // for PageRef
#include "model/XojPage.h"                        // for XojPage
#include "util/GtkUtil.h"                         // for setRadioButtonActionName
#include "util/gtk4_helper.h"                     // for gtk_box_append
#include "util/i18n.h"                            // for _
#include "util/raii/PangoSPtr.h"                  // for PangoAttrListSPtr


class ToolPageLayer::ShowLayerEntry final {
public:
    ShowLayerEntry(LayerController* lc, Layer::Index id) noexcept;
    ShowLayerEntry(const ShowLayerEntry&) = delete;  // No copies, otherwise we'll get an issue with the signals
    ShowLayerEntry& operator=(const ShowLayerEntry&) = delete;
    ShowLayerEntry(ShowLayerEntry&& e) noexcept;
    ShowLayerEntry& operator=(ShowLayerEntry&&) noexcept;
    ~ShowLayerEntry() noexcept;
    xoj::util::WidgetSPtr checkButton;
    gulong callbackHandler;
};

ToolPageLayer::ShowLayerEntry::ShowLayerEntry(LayerController* lc, Layer::Index id) noexcept:
        checkButton(gtk_check_button_new(), xoj::util::adopt) {
    gtk_widget_set_can_focus(checkButton.get(), false);  // todo(gtk4) not necessary anymore
    struct Data {
        LayerController* lc;
        Layer::Index id;
    };
    this->callbackHandler = g_signal_connect_data(
            checkButton.get(), "toggled", G_CALLBACK(+[](GtkToggleButton* btn, gpointer data) {
                Data* d = reinterpret_cast<Data*>(data);
                d->lc->setLayerVisible(d->id, gtk_toggle_button_get_active(btn));
            }),
            new Data{lc, id}, +[](gpointer d, GClosure*) { delete reinterpret_cast<Data*>(d); }, GConnectFlags(0));
    gtk_widget_set_margin_end(checkButton.get(), 4);
}

ToolPageLayer::ShowLayerEntry::ShowLayerEntry(ShowLayerEntry&& e) noexcept:
        checkButton(std::move(e.checkButton)), callbackHandler(std::exchange(e.callbackHandler, 0U)) {}

auto ToolPageLayer::ShowLayerEntry::operator=(ShowLayerEntry&& e) noexcept -> ShowLayerEntry& {
    checkButton = std::move(e.checkButton);
    callbackHandler = std::exchange(e.callbackHandler, 0U);
    return *this;
}

ToolPageLayer::ShowLayerEntry::~ShowLayerEntry() noexcept {
    if (checkButton && callbackHandler) {
        // The button may be referenced elsewhere:
        // we disconnect the callback in case checkButton is not destroyed right away
        g_signal_handler_disconnect(checkButton.get(), callbackHandler);
    }
}


ToolPageLayer::ToolPageLayer(std::string id, LayerController* lc, IconNameHelper iconNameHelper):
        AbstractToolItem(std::move(id)), lc(lc), iconName(iconNameHelper.iconName("combo-layer")) {
    LayerCtrlListener::registerListener(lc);
}

ToolPageLayer::~ToolPageLayer() = default;

auto ToolPageLayer::getToolDisplayName() const -> std::string { return _("Layer Combo"); }

auto ToolPageLayer::getNewToolIcon() const -> GtkWidget* {
    return gtk_image_new_from_icon_name(this->iconName.c_str(), GTK_ICON_SIZE_SMALL_TOOLBAR);
}

void ToolPageLayer::setUsed(bool used) {
    AbstractToolItem::setUsed(used);
    if (!used) {
        this->showLayerItems.clear();
    }
}

/// @return The GtkWidget* is a floating ref
static std::pair<GtkWidget*, std::vector<ToolPageLayer::ShowLayerEntry>> createGrid(LayerController* lc,
                                                                                    GtkWidget* popover) {

#if GTK_MAJOR_VERSION == 3
    GtkRadioButton* group = nullptr;
    auto createLayerRadioButton = [&group, popover](const std::string& layerName, Layer::Index id) -> GtkWidget* {
        GtkWidget* btn = gtk_radio_button_new_with_label_from_widget(group, layerName.c_str());
        gtk_widget_set_can_focus(btn, false);
        group = GTK_RADIO_BUTTON(btn);

        gtk_actionable_set_action_target_value(GTK_ACTIONABLE(btn), xoj::util::makeGVariantSPtr(id).get());
        /**
         * RadioButton's and GAction don't work as expected in GTK3
         * To circumvent this, we have our own GAction/RadioButton interactions
         */
        xoj::util::gtk::setRadioButtonActionName(GTK_RADIO_BUTTON(btn), "win", Action_toString(Action::LAYER_ACTIVE));

        g_signal_connect_object(btn, "toggled", G_CALLBACK(+[](GtkToggleButton*, gpointer popover) {
                                    gtk_popover_popdown(GTK_POPOVER(popover));
                                }),
                                popover, GConnectFlags(0));
        return btn;
    };

    auto createBackgroundRadioButton = [](LayerController* lc) {
        GtkWidget* btn = gtk_radio_button_new_with_label(nullptr, lc->getLayerNameById(0U).c_str());
        gtk_widget_set_can_focus(btn, false);
        gtk_widget_add_css_class(btn, "invisible");
        gtk_widget_set_hexpand(btn, true);
        return btn;
    };
#else  // GTK4
    std::string actionName = std::string("win.") + Action_toString(Action::LAYER_ACTIVE);  // Todo(cpp20) constexpr
    GtkCheckButton* group = nullptr;
    auto createLayerRadioButton = [&actionName, &group, popover](const std::string& layerName,
                                                                 Layer::Index id) -> GtkWidget* {
        GtkWidget* btn = gtk_check_button_new_with_label(layerName.c_str());
        // Is grouping necessary here? The GTK4 doc is unclear
        gtk_check_button_set_group(GTK_CHECK_BUTTON(btn), std::exchange(group, GTK_CHECK_BUTTON(btn));
        gtk_actionable_set_action_name(GTK_ACTIONABLE(btn), actionName.c_str());
        gtk_actionable_set_action_target_value(GTK_ACTIONABLE(btn), xoj::util::makeGVariantSPtr(id).get());
        g_signal_connect_object(btn, "toggled", G_CALLBACK(+[](GtkToggleButton*, gpointer popover) {
            gtk_popover_popdown(GTK_POPOVER(popover)); }), popover, GConnectFlags(0));
        return btn;
    };
    auto createBackgroundRadioButton = [](LayerController* lc) {
        GtkWidget* btn = gtk_check_button_new_with_label(lc->getLayerNameById(0U).c_str());
        // Todo(gtk4): adapt the css class
        gtk_widget_add_css_class(btn, "invisible");
        gtk_widget_set_hexpand(btn, true);
        return btn;
    };
#endif

    GtkGrid* grid = GTK_GRID(gtk_grid_new());

    auto layerCount = lc->getLayerCount();
    std::vector<ToolPageLayer::ShowLayerEntry> entries;
    entries.reserve(layerCount + 1);
    // Handle the background separately, as it cannot be selected
    gtk_grid_attach(grid, createBackgroundRadioButton(lc), 0, static_cast<int>(layerCount), 1, 1);
    gtk_grid_attach(grid, entries.emplace_back(lc, 0).checkButton.get(), 1, static_cast<int>(layerCount), 1, 1);
    // Then the other layers
    for (Layer::Index id = 1; id <= layerCount; id++) {
        int y = static_cast<int>(layerCount - id);
        gtk_grid_attach(grid, createLayerRadioButton(lc->getLayerNameById(id), id), 0, y, 1, 1);
        gtk_grid_attach(grid, entries.emplace_back(lc, id).checkButton.get(), 1, y, 1, 1);
    }

    gtk_widget_show_all(GTK_WIDGET(grid));
    return {GTK_WIDGET(grid), std::move(entries)};
}

void ToolPageLayer::setupLayersGrid() {
    auto [grid, entries] = createGrid(lc, popover.get());
    this->grid.reset(grid, xoj::util::adopt);
    gtk_box_append(GTK_BOX(box.get()), grid);
    this->showLayerItems = std::move(entries);

    layerVisibilityChanged();
    updateSelectedLayer();
}

void ToolPageLayer::rebuildLayerMenu() {
    if (!item) {
        return;
    }
    showLayerItems.clear();
    gtk_box_remove(GTK_BOX(box.get()), grid.get());
    grid.reset();

    setupLayersGrid();
}

void ToolPageLayer::layerVisibilityChanged() {
    if (!item) {
        return;
    }
    PageRef page = lc->getCurrentPage();
    if (page) {
        Layer::Index n = 0;
        for (auto& e: showLayerItems) {
            g_signal_handler_block(e.checkButton.get(), e.callbackHandler);
            gtk_check_button_set_active(GTK_CHECK_BUTTON(e.checkButton.get()), page->isLayerVisible(n++));
            g_signal_handler_unblock(e.checkButton.get(), e.callbackHandler);
        }
    }
}

void ToolPageLayer::updateSelectedLayer() {
    if (layerLabel) {
        gtk_label_set_text(GTK_LABEL(layerLabel.get()), lc->getCurrentLayerName().c_str());
    }
}


/**
 * @brief Appends to `box` a button with bold label `name` activating the Action `action`
 */
static void addSpecialButton(GtkBox* box, const std::string& name, Action action) {
    GtkWidget* btn = gtk_button_new();
    gtk_widget_set_can_focus(btn, false);  // todo(gtk4) not necessary anymore
    GtkWidget* lb = gtk_label_new(name.c_str());
    gtk_widget_set_halign(lb, GTK_ALIGN_START);

    xoj::util::PangoAttrListSPtr attrs(pango_attr_list_new(), xoj::util::adopt);
    pango_attr_list_insert(attrs.get(), pango_attr_weight_new(PANGO_WEIGHT_BOLD));
    gtk_label_set_attributes(GTK_LABEL(lb), attrs.get());

    gtk_button_set_child(GTK_BUTTON(btn), lb);
    std::string fullActionName = std::string("win.") + Action_toString(action);
    gtk_actionable_set_action_name(GTK_ACTIONABLE(btn), fullActionName.c_str());
    gtk_box_append(box, btn);
}

auto ToolPageLayer::createItem(bool horizontal) -> GtkWidget* {
    {  // Setup selected layer name label
        layerLabel.reset(gtk_label_new(_("Loading...")), xoj::util::adopt);
        xoj::util::PangoAttrListSPtr attrs(pango_attr_list_new(), xoj::util::adopt);
        pango_attr_list_insert(attrs.get(), pango_attr_weight_new(PANGO_WEIGHT_BOLD));
        gtk_label_set_attributes(GTK_LABEL(this->layerLabel.get()), attrs.get());
    }

    {  // Setup main widget
        this->item.reset(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1), xoj::util::adopt);
        GtkBox* hbox = GTK_BOX(this->item.get());
        GtkWidget* label = gtk_label_new(_("Layer"));
        gtk_widget_set_margin_start(label, 7);
        gtk_widget_set_margin_end(label, 7);
        gtk_box_append(hbox, label);
        GtkWidget* menuButton = gtk_menu_button_new();
#if GTK_MAJOR_VERSION == 3
        gtk_widget_set_can_focus(menuButton, false);
        gtk_box_append(hbox, this->layerLabel.get());
#else
        gtk_menu_button_set_child(GTK_MENU_BUTTON(menuButton), this->layerLabel.get());
        gtk_menu_button_set_always_show_arrow(GTK_MENU_BUTTON(menuButton), true);
#endif
        gtk_box_append(hbox, menuButton);
        // TODO: fix orientation
        gtk_menu_button_set_direction(GTK_MENU_BUTTON(menuButton), horizontal ? GTK_ARROW_DOWN : GTK_ARROW_RIGHT);

        GtkWidget* popover = gtk_popover_new();
        this->popover.reset(popover, xoj::util::adopt);
        gtk_widget_add_css_class(popover, "toolbar");
        this->box.reset(gtk_box_new(GTK_ORIENTATION_VERTICAL, 0), xoj::util::adopt);
        gtk_popover_set_child(GTK_POPOVER(popover), this->box.get());
        gtk_menu_button_set_popover(GTK_MENU_BUTTON(menuButton), popover);
    }

    {  // Fill the popover box
        GtkBox* box = GTK_BOX(this->box.get());
        addSpecialButton(box, _("Show all"), Action::LAYER_SHOW_ALL);
        addSpecialButton(box, _("Hide all"), Action::LAYER_HIDE_ALL);
        gtk_box_append(box, gtk_separator_new(GTK_ORIENTATION_HORIZONTAL));
        addSpecialButton(box, _("Create new layer"), Action::LAYER_NEW);
        gtk_box_append(box, gtk_separator_new(GTK_ORIENTATION_HORIZONTAL));
        gtk_widget_show_all(this->box.get());
    }

    setupLayersGrid();

    return this->item.get();
}
