#include "AbstractSliderItem.h"

#include <iomanip>  // for operator<<, setw
#include <sstream>  // for stringstream, basic...
#include <tuple>    // for tuple
#include <utility>  // for move

#include <gdk/gdk.h>      // for GdkEvent, GdkEventS...
#include <glib-object.h>  // for G_CALLBACK, g_objec...
#include <glib.h>         // for g_strdup, gchar

#include "gui/toolbarMenubar/AbstractToolItem.h"  // for AbstractToolItem
#include "util/GtkUtil.h"                         // for setWidgetFollowActionEnabled

class AbstractSliderItem::Impl {
public:
    Impl(AbstractSliderItem* publicApi, SliderRange range, ActionRef enablingGAction);

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

    /**
     * Constructs a new toolbar item and slider.
     * Does not take ownership.
     */
    GtkRange* newItem(bool horizontal) const;

    // Conversions to/from range_ (internal values) to external scale values:
    double getScaleMax() const { return publicApi_->scaleFunc(range_.max); };
    double getScaleMin() const { return publicApi_->scaleFunc(range_.min); };
    double getFineStepSize() const { return (getScaleMax() - getScaleMin()) / range_.fineSteps; };
    double getCoarseStepSize() const { return (getScaleMax() - getScaleMin()) / range_.coarseSteps; };

public:
    bool horizontal_ = false;
    GtkRange* slider_ = nullptr;
    ActionRef enablingGAction_;  ///< The widget is sensitive exactly when this GAction's `enabled` property is true
    AbstractSliderItem* publicApi_;

    const SliderRange range_;
};

AbstractSliderItem::~AbstractSliderItem() = default;
AbstractSliderItem::AbstractSliderItem(std::string id, SliderRange range, ActionRef enablingGAction):
        AbstractToolItem{std::move(id)}, pImpl{std::make_unique<Impl>(this, range, std::move(enablingGAction))} {}

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
    gtk_widget_set_can_focus(GTK_WIDGET(slider), false);  // todo(gtk4) not necessary anymore
}

auto AbstractSliderItem::createItem(bool horizontal) -> xoj::util::WidgetSPtr {
    pImpl->horizontal_ = horizontal;
    auto* slider = pImpl->newItem(horizontal);

    pImpl->setSlider(slider);

    return xoj::util::WidgetSPtr(GTK_WIDGET(slider), xoj::util::adopt);
}

void AbstractSliderItem::onSliderButtonPress() {}
void AbstractSliderItem::onSliderButtonRelease() {}
void AbstractSliderItem::onSliderHoverScroll() {}
auto AbstractSliderItem::formatSliderValue(double value) const -> std::string {
    std::stringstream result;
    result << std::setw(3) << std::fixed << value;
    return result.str();
}

auto AbstractSliderItem::getSliderWidget() -> GtkRange* { return pImpl->slider_; }
auto AbstractSliderItem::isCurrentHorizontal() const -> bool {
    // Default to false if no associated slider exists.
    if (pImpl->slider_ == nullptr) {
        return false;
    }

    return pImpl->horizontal_;
}

AbstractSliderItem::Impl::Impl(AbstractSliderItem* parent, SliderRange range, ActionRef enablingGAction):
        enablingGAction_(std::move(enablingGAction)), publicApi_{parent}, range_{range} {}

auto AbstractSliderItem::Impl::newItem(bool horizontal) const -> GtkRange* {
    GtkOrientation orientation = horizontal ? GTK_ORIENTATION_HORIZONTAL : GTK_ORIENTATION_VERTICAL;
    GtkRange* slider =
            GTK_RANGE(gtk_scale_new_with_range(orientation, getScaleMin(), getScaleMax(), getFineStepSize()));
    publicApi_->configure(slider, horizontal);

    return slider;
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
    if (enablingGAction_) {
        xoj::util::gtk::setWidgetFollowActionEnabled(newSliderWidget, G_ACTION(enablingGAction_.get()));
    }

    slider_ = slider;
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
