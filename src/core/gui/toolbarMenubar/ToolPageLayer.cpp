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
#include "util/glib_casts.h"                      // for closure_notify_cb
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
    struct Data {
        LayerController* lc;
        Layer::Index id;
    };
    this->callbackHandler =
            g_signal_connect_data(checkButton, "toggled", G_CALLBACK(+[](GtkCheckButton* btn, gpointer data) {
                                      Data* d = reinterpret_cast<Data*>(data);
                                      d->lc->setLayerVisible(d->id, gtk_check_button_get_active(btn));
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
static std::tuple<GtkWidget*, std::vector<ShowLayerEntry>> createGrid(LayerController* lc, GtkPopover* popover) {

    std::string actionName = std::string("win.") + Action_toString(Action::LAYER_ACTIVE);  // Todo(cpp20) constexpr
    auto createLayerRadioButton = [&actionName, popover](const std::string& layerName, Layer::Index id) {
        GtkWidget* btn = gtk_check_button_new_with_label(layerName.c_str());
        gtk_actionable_set_action_name(GTK_ACTIONABLE(btn), actionName.c_str());
        gtk_actionable_set_action_target_value(GTK_ACTIONABLE(btn), xoj::util::makeGVariantSPtr(id).get());

        // Callback to hide the popover when a new layer is selected
        GtkGesture* click = gtk_gesture_click_new();
        gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(click), 1);
        gtk_widget_add_controller(btn, GTK_EVENT_CONTROLLER(click));
        // Anything later than CAPTURE and the default handler has grabbed the sequence
        gtk_event_controller_set_propagation_phase(GTK_EVENT_CONTROLLER(click), GTK_PHASE_CAPTURE);
        g_signal_connect_object(click, "released",
                                G_CALLBACK(+[](GtkGestureClick*, int n_press, double, double, gpointer popover) {
                                    if (n_press == 1) {
                                        // Wait for the default callback to have run before hiding the popover
                                        Util::execWhenIdle([popover]() { gtk_popover_popdown(GTK_POPOVER(popover)); });
                                    }
                                }),
                                popover, GConnectFlags(0));

        return btn;
    };
    auto createBackgroundRadioButton = [](LayerController* lc) {
        GtkWidget* btn = gtk_check_button_new_with_label(lc->getLayerNameById(0U).c_str());
        gtk_widget_add_css_class(btn, "invisible");
        gtk_widget_set_hexpand(btn, true);
        return btn;
    };

    GtkGrid* grid = GTK_GRID(gtk_grid_new());

    auto layerCount = lc->getLayerCount();
    std::vector<ShowLayerEntry> entries;
    entries.reserve(layerCount + 1);
    // Handle the background separately, as it cannot be selected
    gtk_grid_attach(grid, createBackgroundRadioButton(lc), 0, static_cast<int>(layerCount), 1, 1);
    gtk_grid_attach(grid, entries.emplace_back(lc, 0).checkButton, 1, static_cast<int>(layerCount), 1, 1);
    // Then the other layers
    for (Layer::Index id = 1; id <= layerCount; id++) {
        int y = static_cast<int>(layerCount - id);
        gtk_grid_attach(grid, createLayerRadioButton(lc->getLayerNameById(id), id), 0, y, 1, 1);
        gtk_grid_attach(grid, entries.emplace_back(lc, id).checkButton, 1, y, 1, 1);
    }

    return {GTK_WIDGET(grid), std::move(entries)};
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
    return box;
}

/// @return floating ref
static GtkPopover* makePopover(GtkBox* content) {
    auto popover = GTK_POPOVER(gtk_popover_new());
    gtk_widget_add_css_class(GTK_WIDGET(popover), "toolbar");
    gtk_popover_set_has_arrow(popover, false);
    gtk_widget_set_halign(GTK_WIDGET(popover), GTK_ALIGN_START);
    gtk_widget_set_valign(GTK_WIDGET(popover), GTK_ALIGN_START);

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
    ~InstanceData() = default;

    xoj::util::WidgetSPtr makeWidget(ToolbarSide side) {
        GtkBox* hbox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1));
        GtkWidget* lbl = gtk_label_new(_("Layer"));
        gtk_widget_set_margin_start(lbl, 7);
        gtk_widget_set_margin_end(lbl, 7);
        gtk_box_append(hbox, lbl);

        GtkWidget* menuButton = gtk_menu_button_new();
        gtk_menu_button_set_child(GTK_MENU_BUTTON(menuButton), GTK_WIDGET(this->label));
        gtk_menu_button_set_always_show_arrow(GTK_MENU_BUTTON(menuButton), true);
        gtk_box_append(hbox, menuButton);

        setMenuButtonDirection(GTK_MENU_BUTTON(menuButton), side);
        gtk_menu_button_set_popover(GTK_MENU_BUTTON(menuButton), GTK_WIDGET(this->popover));

        LayerCtrlListener::registerListener(lc);

        // *this is destroyed once the widget (hence *popover) has been destroyed. No need to disconnect those signals
        g_signal_connect(popover, "show", xoj::util::wrap_for_g_callback_v<popoverShown>, this);
        g_signal_connect(popover, "hide", xoj::util::wrap_for_g_callback_v<popoverHidden>, this);

        updateSelectedLayer();

        return xoj::util::WidgetSPtr(GTK_WIDGET(hbox), xoj::util::adopt);
    }

    static void popoverShown(GtkWidget*, InstanceData* data) {
        xoj_assert(!data->grid && data->showLayerItems.empty());
        data->setupLayersGrid();
    }
    static void popoverHidden(GtkWidget*, InstanceData* data) {
        if (data->grid) {
            data->showLayerItems.clear();
            gtk_box_remove(data->popoverBox, std::exchange(data->grid, nullptr));
        }
    }

private:
    void setupLayersGrid() {
        auto [grid, entries] = createGrid(lc, popover);
        this->grid = grid;
        gtk_box_append(this->popoverBox, grid);
        xoj_assert(GTK_WIDGET(this->popoverBox) == gtk_widget_get_parent(grid));
        this->showLayerItems = std::move(entries);

        layerVisibilityChanged();
    }

    void rebuildLayerMenu() override {
        if (grid) {
            // Only rebuild if the popover is shown
            showLayerItems.clear();
            gtk_box_remove(popoverBox, std::exchange(grid, nullptr));
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
        ItemWithNamedIcon(std::move(id), Category::NAVIGATION),
        lc(lc),
        iconName(iconNameHelper.iconName("combo-layer")) {}

ToolPageLayer::~ToolPageLayer() = default;

auto ToolPageLayer::getToolDisplayName() const -> std::string { return _("Layer Combo"); }

auto ToolPageLayer::getIconName() const -> const char* { return this->iconName.c_str(); }

auto ToolPageLayer::createItem(ToolbarSide side) -> Widgetry {
    auto data = std::make_unique<InstanceData>(lc);
    auto item = data->makeWidget(side);

    // Destroy *data if the widget is destroyed
    g_object_weak_ref(
            G_OBJECT(item.get()), +[](gpointer d, GObject*) { delete static_cast<InstanceData*>(d); }, data.release());

    return {std::move(item), nullptr};
}
