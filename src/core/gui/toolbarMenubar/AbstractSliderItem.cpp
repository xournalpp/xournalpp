#include "AbstractSliderItem.h"

#include <iomanip>  // for operator<<, setw
#include <sstream>  // for stringstream, basic...
#include <tuple>    // for tuple
#include <utility>  // for move

#include <gdk/gdk.h>      // for GdkEvent, GdkEventS...
#include <glib-object.h>  // for G_CALLBACK, g_objec...
#include <glib.h>         // for g_strdup, gchar

#include "gui/toolbarMenubar/AbstractToolItem.h"  // for AbstractToolItem

class ActionHandler;

class AbstractSliderItem::Impl {
public:
    Impl(AbstractSliderItem* publicApi_, SliderRange range);

    static void onSliderChanged(GtkRange* range, AbstractSliderItem* parent);
    static bool onSliderButtonPress(GtkRange* range, GdkEvent* event, AbstractSliderItem* parent);
    static bool onSliderButtonRelease(GtkRange* range, GdkEvent* event, AbstractSliderItem* parent);
    static bool onSliderHoverScroll(GtkRange* range, GdkEventScroll* event, AbstractSliderItem* parent);
    static gchar* formatSliderValue(GtkRange* range, double value, AbstractSliderItem* parent);

    /**
     * Listen for input events on the new slider.
     *
     * If this already has a slider, this disconnects listeners
     * on events for the current slider.
     *
     * @param slider The new slider to listen for events on.
     */
    void setSlider(GtkRange* slider);

    void setEnabled(bool enabled);

    /**
     * Constructs a new toolbar item and slider.
     * Does not take ownership.
     */
    std::tuple<GtkToolItem*, GtkRange*> newItem(bool horizontal) const;

    // Conversions to/from range_ (internal values) to external scale values:
    double getScaleMax() const { return publicApi_->scaleFunc(range_.max); };
    double getScaleMin() const { return publicApi_->scaleFunc(range_.min); };
    double getFineStepSize() const { return (getScaleMax() - getScaleMin()) / range_.fineSteps; };
    double getCoarseStepSize() const { return (getScaleMax() - getScaleMin()) / range_.coarseSteps; };

public:
    bool horizontal_ = false;
    GtkRange* slider_ = nullptr;
    GtkToolItem* toolItem_ = nullptr;
    AbstractSliderItem* publicApi_;

    const SliderRange range_;
};

AbstractSliderItem::~AbstractSliderItem() = default;
AbstractSliderItem::AbstractSliderItem(std::string id, ActionHandler* handler, ActionType type, SliderRange range):
        AbstractToolItem{std::move(id), handler, type}, pImpl{std::make_unique<Impl>(this, range)} {}

void AbstractSliderItem::configure(GtkRange* slider, bool isHorizontal) const {
    double fineStepSize = pImpl->getFineStepSize();
    double coarseStepSize = pImpl->getCoarseStepSize();

    gtk_range_set_increments(slider, fineStepSize, coarseStepSize);

    if (isHorizontal) {
        gtk_widget_set_size_request(GTK_WIDGET(slider), 120, 16);
    } else {
        gtk_widget_set_size_request(GTK_WIDGET(slider), 16, 120);
    }

    gtk_scale_set_draw_value(GTK_SCALE(slider), true);
    gtk_widget_set_can_focus(GTK_WIDGET(slider), false);
}

auto AbstractSliderItem::createItem(bool horizontal) -> GtkToolItem* {
    pImpl->horizontal_ = horizontal;
    auto [toolItem, slider] = pImpl->newItem(horizontal);

    pImpl->toolItem_ = toolItem;
    g_object_ref(toolItem);
    pImpl->setSlider(slider);

    if (GTK_IS_TOOL_ITEM(toolItem)) {
        gtk_tool_item_set_homogeneous(GTK_TOOL_ITEM(toolItem), false);
    }
    if (GTK_IS_TOOL_BUTTON(toolItem) || GTK_IS_TOGGLE_TOOL_BUTTON(toolItem)) {
        g_signal_connect(toolItem, "clicked", G_CALLBACK(&toolButtonCallback), this);
    }

    return toolItem;
}

auto AbstractSliderItem::createTmpItem(bool horizontal) -> GtkToolItem* {
    auto [item, slider] = pImpl->newItem(horizontal);
    g_object_ref(item);

    gtk_widget_show_all(GTK_WIDGET(item));
    return item;
}

// TODO(personalizedrefrigerator): Remove this (currently required by AbstractToolItem
auto AbstractSliderItem::newItem() -> GtkToolItem* { return createTmpItem(true); }

void AbstractSliderItem::onSliderButtonPress() {}
void AbstractSliderItem::onSliderButtonRelease() {}
void AbstractSliderItem::onSliderHoverScroll() {}
auto AbstractSliderItem::formatSliderValue(double value) const -> std::string {
    std::stringstream result;
    result << std::setw(3) << std::fixed << value;
    return result.str();
}

void AbstractSliderItem::enable(bool enabled) { pImpl->setEnabled(enabled); }

auto AbstractSliderItem::getSliderWidget() -> GtkRange* { return pImpl->slider_; }
auto AbstractSliderItem::isCurrentHorizontal() const -> bool {
    // Default to false if no associated slider exists.
    if (pImpl->slider_ == nullptr) {
        return false;
    }

    return pImpl->horizontal_;
}

AbstractSliderItem::Impl::Impl(AbstractSliderItem* parent, SliderRange range): publicApi_{parent}, range_{range} {}

auto AbstractSliderItem::Impl::newItem(bool horizontal) const -> std::tuple<GtkToolItem*, GtkRange*> {
    GtkOrientation orientation = horizontal ? GTK_ORIENTATION_HORIZONTAL : GTK_ORIENTATION_VERTICAL;
    GtkToolItem* it = gtk_tool_item_new();
    GtkRange* slider =
            GTK_RANGE(gtk_scale_new_with_range(orientation, getScaleMin(), getScaleMax(), getFineStepSize()));
    publicApi_->configure(slider, horizontal);

    gtk_container_add(GTK_CONTAINER(it), GTK_WIDGET(slider));
    return {it, slider};
}

void AbstractSliderItem::Impl::setSlider(GtkRange* slider) {
    GtkWidget* newSliderWidget = GTK_WIDGET(slider);

    // If we already had a slider, disconnect from its events.
    if (slider_) {
        GtkWidget* oldSliderWidget = GTK_WIDGET(slider_);
        g_signal_handlers_disconnect_by_func(oldSliderWidget, (void*)(onSliderChanged), publicApi_);
        g_signal_handlers_disconnect_by_func(oldSliderWidget, (void*)(onSliderButtonPress), publicApi_);
        g_signal_handlers_disconnect_by_func(oldSliderWidget, (void*)(onSliderButtonRelease), publicApi_);
        g_signal_handlers_disconnect_by_func(oldSliderWidget, (void*)(onSliderHoverScroll), publicApi_);
        g_signal_handlers_disconnect_by_func(oldSliderWidget, (void*)(formatSliderValue), publicApi_);
    }

    // Connect new events.
    g_signal_connect(newSliderWidget, "value-changed", G_CALLBACK(onSliderChanged), publicApi_);
    g_signal_connect(newSliderWidget, "button-press-event", G_CALLBACK(onSliderButtonPress), publicApi_);
    g_signal_connect(newSliderWidget, "button-release-event", G_CALLBACK(onSliderButtonRelease), publicApi_);
    g_signal_connect(newSliderWidget, "scroll-event", G_CALLBACK(onSliderHoverScroll), publicApi_);
    g_signal_connect(newSliderWidget, "format-value", G_CALLBACK(formatSliderValue), publicApi_);

    slider_ = slider;
}

void AbstractSliderItem::Impl::setEnabled(bool enabled) {
    if (slider_) {
        gtk_widget_set_sensitive(GTK_WIDGET(slider_), enabled);
    }
    if (toolItem_) {
        gtk_widget_set_sensitive(GTK_WIDGET(toolItem_), enabled);
    }
}

void AbstractSliderItem::Impl::onSliderChanged(GtkRange* range, AbstractSliderItem* parent) {
    parent->onSliderChanged(parent->scaleFuncInv(gtk_range_get_value(range)));
}

bool AbstractSliderItem::Impl::onSliderButtonPress(GtkRange* range, GdkEvent* event, AbstractSliderItem* parent) {
    parent->onSliderButtonPress();

    return false;  // Let the slider handle the event.
}

bool AbstractSliderItem::Impl::onSliderButtonRelease(GtkRange* range, GdkEvent* event, AbstractSliderItem* parent) {
    parent->onSliderButtonRelease();
    return false;
}

bool AbstractSliderItem::Impl::onSliderHoverScroll(GtkRange* range, GdkEventScroll* event, AbstractSliderItem* parent) {
    parent->onSliderHoverScroll();
    return false;
}

auto AbstractSliderItem::Impl::formatSliderValue(GtkRange* range, double value, AbstractSliderItem* parent) -> gchar* {
    std::string text = parent->formatSliderValue(parent->scaleFuncInv(value));
    return g_strdup(static_cast<const gchar*>(text.c_str()));
}
