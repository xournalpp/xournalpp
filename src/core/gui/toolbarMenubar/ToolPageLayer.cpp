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
#include "util/glib_casts.h"                      // for closure_notify_cb
#include "util/gtk4_helper.h"                     // for gtk_box_append
#include "util/i18n.h"                            // for _
#include "util/raii/PangoSPtr.h"                  // for PangoAttrListSPtr


class ShowLayerEntry final {
public:
    ShowLayerEntry(LayerController* lc, Layer::Index id) noexcept;
    ShowLayerEntry(const ShowLayerEntry&) = delete;  // No copies, otherwise we'll get an issue with the signals
    ShowLayerEntry& operator=(const ShowLayerEntry&) = delete;
    ShowLayerEntry(ShowLayerEntry&& e) noexcept;
    ShowLayerEntry& operator=(ShowLayerEntry&&) noexcept;
    ~ShowLayerEntry() noexcept;
    GtkWidget* checkButton;
    gulong callbackHandler;
};

ShowLayerEntry::ShowLayerEntry(LayerController* lc, Layer::Index id) noexcept: checkButton(gtk_check_button_new()) {
    gtk_widget_set_can_focus(checkButton, false);  // todo(gtk4) not necessary anymore
    struct Data {
        LayerController* lc;
        Layer::Index id;
    };
    this->callbackHandler =
            g_signal_connect_data(checkButton, "toggled", G_CALLBACK(+[](GtkToggleButton* btn, gpointer data) {
                                      Data* d = reinterpret_cast<Data*>(data);
                                      d->lc->setLayerVisible(d->id, gtk_toggle_button_get_active(btn));
                                  }),
                                  new Data{lc, id}, xoj::util::closure_notify_cb<Data>, GConnectFlags(0));
    gtk_widget_set_margin_end(checkButton, 4);
}

ShowLayerEntry::ShowLayerEntry(ShowLayerEntry&& e) noexcept:
        checkButton(std::move(e.checkButton)), callbackHandler(std::exchange(e.callbackHandler, 0U)) {}

auto ShowLayerEntry::operator=(ShowLayerEntry&& e) noexcept -> ShowLayerEntry& {
    checkButton = std::move(e.checkButton);
    callbackHandler = std::exchange(e.callbackHandler, 0U);
    return *this;
}

ShowLayerEntry::~ShowLayerEntry() noexcept = default;

/// @return The GtkWidget* is a floating ref
static std::tuple<GtkWidget*, std::vector<ShowLayerEntry>, std::vector<std::pair<GtkWidget*, gulong>>> createGrid(
        LayerController* lc, GtkPopover* popover) {

#if GTK_MAJOR_VERSION == 3
    GtkRadioButton* group = nullptr;
    auto createLayerRadioButton = [&group, popover](const std::string& layerName,
                                                    Layer::Index id) -> std::pair<GtkWidget*, gulong> {
        GtkWidget* btn = gtk_radio_button_new_with_label_from_widget(group, layerName.c_str());
        gtk_widget_set_can_focus(btn, false);
        group = GTK_RADIO_BUTTON(btn);

        gtk_actionable_set_action_target_value(GTK_ACTIONABLE(btn), xoj::util::makeGVariantSPtr(id).get());
        /**
         * RadioButton's and GAction don't work as expected in GTK3
         * To circumvent this, we have our own GAction/RadioButton interactions
         */
        xoj::util::gtk::setRadioButtonActionName(GTK_RADIO_BUTTON(btn), "win", Action_toString(Action::LAYER_ACTIVE));

        // Callback to hide the popover when a new layer is selected
        auto sig = g_signal_connect_object(btn, "toggled", G_CALLBACK(+[](GtkToggleButton*, gpointer popover) {
                                               gtk_popover_popdown(GTK_POPOVER(popover));
                                           }),
                                           popover, GConnectFlags(0));
        // Block this callback for now, otherwise it is called when the grid is added to the popover
        g_signal_handler_block(btn, sig);

        return {btn, sig};
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
                                                                 Layer::Index id) -> std::pair<GtkWidget*, gulong> {
        GtkWidget* btn = gtk_check_button_new_with_label(layerName.c_str());
        // Is grouping necessary here? The GTK4 doc is unclear
        gtk_check_button_set_group(GTK_CHECK_BUTTON(btn), std::exchange(group, GTK_CHECK_BUTTON(btn));
        gtk_actionable_set_action_name(GTK_ACTIONABLE(btn), actionName.c_str());
        gtk_actionable_set_action_target_value(GTK_ACTIONABLE(btn), xoj::util::makeGVariantSPtr(id).get());
        // Callback to hide the popover when a new layer is selected
        auto sig = g_signal_connect_object(btn, "toggled", G_CALLBACK(+[](GtkToggleButton*, gpointer popover) {
            gtk_popover_popdown(GTK_POPOVER(popover)); }), popover, GConnectFlags(0));
        // Block this callback for now, otherwise it is called when the grid is added to the popover
        g_signal_handler_block(btn, sig);

        return {btn, sig};
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
    std::vector<ShowLayerEntry> entries;
    entries.reserve(layerCount + 1);
    std::vector<std::pair<GtkWidget*, gulong>> radioButtons;
    radioButtons.reserve(layerCount);
    // Handle the background separately, as it cannot be selected
    gtk_grid_attach(grid, createBackgroundRadioButton(lc), 0, static_cast<int>(layerCount), 1, 1);
    gtk_grid_attach(grid, entries.emplace_back(lc, 0).checkButton, 1, static_cast<int>(layerCount), 1, 1);
    // Then the other layers
    for (Layer::Index id = 1; id <= layerCount; id++) {
        int y = static_cast<int>(layerCount - id);
        auto [btn, _] = radioButtons.emplace_back(createLayerRadioButton(lc->getLayerNameById(id), id));
        gtk_grid_attach(grid, btn, 0, y, 1, 1);
        gtk_grid_attach(grid, entries.emplace_back(lc, id).checkButton, 1, y, 1, 1);
    }

    gtk_widget_show_all(GTK_WIDGET(grid));
    return {GTK_WIDGET(grid), std::move(entries), std::move(radioButtons)};
}

/// @return floating ref
static GtkLabel* makeLabel() {
    auto* label = GTK_LABEL(gtk_label_new(_("Loading...")));
    xoj::util::PangoAttrListSPtr attrs(pango_attr_list_new(), xoj::util::adopt);
    pango_attr_list_insert(attrs.get(), pango_attr_weight_new(PANGO_WEIGHT_BOLD));
    gtk_label_set_attributes(label, attrs.get());
    return label;
}

/// @brief Appends to `box` a button with bold label `name` activating the Action `action`
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

/// @return floating ref
static GtkBox* makePopoverContent() {
    GtkBox* box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 0));
    addSpecialButton(box, _("Show all"), Action::LAYER_SHOW_ALL);
    addSpecialButton(box, _("Hide all"), Action::LAYER_HIDE_ALL);
    gtk_box_append(box, gtk_separator_new(GTK_ORIENTATION_HORIZONTAL));
    addSpecialButton(box, _("Create new layer"), Action::LAYER_NEW);
    gtk_box_append(box, gtk_separator_new(GTK_ORIENTATION_HORIZONTAL));
    gtk_widget_show_all(GTK_WIDGET(box));
    return box;
}

/// @return floating ref
static GtkPopover* makePopover(GtkBox* content) {
    auto popover = GTK_POPOVER(gtk_popover_new());
    gtk_widget_add_css_class(GTK_WIDGET(popover), "toolbar");
    gtk_popover_set_child(popover, GTK_WIDGET(content));
    return popover;
}

class InstanceData: public LayerCtrlListener {
public:
    InstanceData(LayerController* lc):
            lc(lc),
            label(makeLabel()),
            popoverBox(makePopoverContent()),
            popover(makePopover(popoverBox)),
            grid(nullptr) {}
    ~InstanceData() {}

    xoj::util::WidgetSPtr makeWidget(bool horizontal) {
        xoj::util::WidgetSPtr item(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1), xoj::util::adopt);
        GtkBox* hbox = GTK_BOX(item.get());
        GtkWidget* lbl = gtk_label_new(_("Layer"));
        gtk_widget_set_margin_start(lbl, 7);
        gtk_widget_set_margin_end(lbl, 7);
        gtk_box_append(hbox, lbl);
        GtkWidget* menuButton = gtk_menu_button_new();
#if GTK_MAJOR_VERSION == 3
        gtk_widget_set_can_focus(menuButton, false);
        gtk_box_append(hbox, GTK_WIDGET(this->label));
#else
        gtk_menu_button_set_child(GTK_MENU_BUTTON(menuButton), GTK_WIDGET(this->label));
        gtk_menu_button_set_always_show_arrow(GTK_MENU_BUTTON(menuButton), true);
#endif
        gtk_box_append(hbox, menuButton);
        // TODO: fix orientation
        gtk_menu_button_set_direction(GTK_MENU_BUTTON(menuButton), horizontal ? GTK_ARROW_DOWN : GTK_ARROW_RIGHT);
        gtk_menu_button_set_popover(GTK_MENU_BUTTON(menuButton), GTK_WIDGET(this->popover));

        LayerCtrlListener::registerListener(lc);

        // *this is destroyed once the widget (hence *popover) has been destroyed. No need to disconnect those signals
        g_signal_connect(popover, "show", xoj::util::wrap_for_g_callback_v<popoverShown>, this);
        g_signal_connect(popover, "hide", xoj::util::wrap_for_g_callback_v<popoverHidden>, this);

        updateSelectedLayer();

        return item;
    }

    static void popoverShown(GtkWidget*, InstanceData* data) {
        xoj_assert(!data->grid && data->showLayerItems.empty());
        data->setupLayersGrid();
    }
    static void popoverHidden(GtkWidget*, InstanceData* data) {
        if (data->grid) {
            data->showLayerItems.clear();
            gtk_box_remove(data->popoverBox, data->grid);
            data->grid = nullptr;
        }
    }

private:
    void setupLayersGrid() {
        auto [grid, entries, radioButtons] = createGrid(lc, popover);
        this->grid = grid;
        gtk_box_append(this->popoverBox, grid);
        this->showLayerItems = std::move(entries);
        for (auto [btn, sig]: radioButtons) {
            g_signal_handler_unblock(btn, sig);
        }

        layerVisibilityChanged();
    }

    void rebuildLayerMenu() override {
        if (grid) {
            // Only rebuild if the popover is shown
            showLayerItems.clear();
            gtk_box_remove(popoverBox, grid);
            grid = nullptr;

            setupLayersGrid();
        }
        updateSelectedLayer();
    }

    void layerVisibilityChanged() override {
        PageRef page = lc->getCurrentPage();
        if (page) {
            Layer::Index n = 0;
            for (auto& e: showLayerItems) {
                g_signal_handler_block(e.checkButton, e.callbackHandler);
                gtk_check_button_set_active(GTK_CHECK_BUTTON(e.checkButton), page->isLayerVisible(n++));
                g_signal_handler_unblock(e.checkButton, e.callbackHandler);
            }
        }
    }

    void updateSelectedLayer() override { gtk_label_set_text(label, lc->getCurrentLayerName().c_str()); }

private:
    LayerController* lc;
    GtkLabel* label;
    GtkBox* popoverBox;
    GtkPopover* popover;
    GtkWidget* grid;

    std::vector<ShowLayerEntry> showLayerItems;  ///< Widgets for the layer menu. Index = 0 is for background.
};


ToolPageLayer::ToolPageLayer(std::string id, LayerController* lc, IconNameHelper iconNameHelper):
        AbstractToolItem(std::move(id)), lc(lc), iconName(iconNameHelper.iconName("combo-layer")) {}

ToolPageLayer::~ToolPageLayer() = default;

auto ToolPageLayer::getToolDisplayName() const -> std::string { return _("Layer Combo"); }

auto ToolPageLayer::getNewToolIcon() const -> GtkWidget* {
    return gtk_image_new_from_icon_name(this->iconName.c_str(), GTK_ICON_SIZE_SMALL_TOOLBAR);
}

auto ToolPageLayer::createItem(bool horizontal) -> xoj::util::WidgetSPtr {
    auto data = std::make_unique<InstanceData>(lc);
    auto item = data->makeWidget(horizontal);

    // Destroy *data if the widget is destroyed
    g_object_weak_ref(
            G_OBJECT(item.get()), +[](gpointer d, GObject*) { delete static_cast<InstanceData*>(d); }, data.release());

    return item;
}
