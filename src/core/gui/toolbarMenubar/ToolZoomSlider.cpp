#include "ToolZoomSlider.h"

#include <cmath>    // for exp, log
#include <sstream>  // for stringstream, bas...
#include <utility>  // for move

#include <glib.h>  // for g_get_monotonic_time

#include "control/zoom/ZoomControl.h"               // for ZoomControl, DEFA...
#include "gui/toolbarMenubar/AbstractSliderItem.h"  // for AbstractSliderItem
#include "util/i18n.h"                              // for _

class ActionHandler;

constexpr double SCALE_LOG_OFFSET = 0.20753;

constexpr double FINE_STEP_COUNT = 100.0;
constexpr double COARSE_STEP_COUNT = 10.0;

// TODO(personalizedrefrigerator): Update the range to reflect the min and max zoom settings.
constexpr auto SLIDER_RANGE =
        AbstractSliderItem::SliderRange{DEFAULT_ZOOM_MIN, DEFAULT_ZOOM_MAX, FINE_STEP_COUNT, COARSE_STEP_COUNT};

class ToolZoomSlider::Impl {
public:
    Impl(ToolZoomSlider* publicApi, ZoomControl* zoom, IconNameHelper iconNameHelper);
    void onSliderChanged(double value);
    void onSliderHoverScroll();
    void onSliderButtonPress();
    void onSliderButtonRelease();

    void updateFromZoomChange(GtkRange* slider);
    void updateScaleMarks(GtkScale* slider, bool horizontal) const;

private:
    bool sliderChangingByZoomControlOrInit_ = false;
    bool sliderChangingBySliderDrag_ = false;
    bool sliderChangingBySliderHoverScroll_ = false;
    gint64 sliderHoverScrollLastTime_ = 0;

public:
    ZoomControl* zoom_ = nullptr;
    ToolZoomSlider* publicApi_ = nullptr;
    IconNameHelper iconNameHelper_;
};

ToolZoomSlider::ToolZoomSlider(std::string id, ActionHandler* handler, ActionType type, ZoomControl* zoom,
                               IconNameHelper iconNameHelper):
        AbstractSliderItem{std::move(id), handler, type, SLIDER_RANGE},
        pImpl{std::make_unique<Impl>(this, zoom, iconNameHelper)} {
    zoom->addZoomListener(this);
}

ToolZoomSlider::~ToolZoomSlider() = default;

ToolZoomSlider::Impl::Impl(ToolZoomSlider* publicApi, ZoomControl* zoom, IconNameHelper iconNameHelper):
        zoom_{zoom}, publicApi_{publicApi}, iconNameHelper_{iconNameHelper} {}

void ToolZoomSlider::onSliderChanged(double value) { pImpl->onSliderChanged(value); }

void ToolZoomSlider::Impl::onSliderChanged(double value) {
    // Only update the page's zoom if the **user** is modifying
    // the zoom.
    if (!sliderChangingByZoomControlOrInit_ && !zoom_->isZoomPresentationMode() &&
        (sliderChangingBySliderDrag_ || sliderChangingBySliderHoverScroll_)) {
        double back = zoom_->getZoom100Value() * value;
        zoom_->zoomSequenceChange(back, false);
    }
    sliderChangingBySliderHoverScroll_ = false;
}

void ToolZoomSlider::onSliderButtonPress() { pImpl->onSliderButtonPress(); }

void ToolZoomSlider::Impl::onSliderButtonPress() {
    if (!sliderChangingBySliderDrag_ && !zoom_->isZoomPresentationMode()) {
        zoom_->setZoomFitMode(false);
        zoom_->startZoomSequence();
        sliderChangingBySliderDrag_ = true;
    }
}

void ToolZoomSlider::onSliderButtonRelease() { pImpl->onSliderButtonRelease(); }

void ToolZoomSlider::Impl::onSliderButtonRelease() {
    zoom_->endZoomSequence();
    sliderChangingBySliderDrag_ = false;
}

void ToolZoomSlider::onSliderHoverScroll() { pImpl->onSliderHoverScroll(); }

void ToolZoomSlider::Impl::onSliderHoverScroll() {
    gint64 now = g_get_monotonic_time();
    if (now > sliderHoverScrollLastTime_ + 500) {
        zoom_->setZoomFitMode(false);
        zoom_->startZoomSequence();
    }

    sliderChangingBySliderHoverScroll_ = true;
    sliderHoverScrollLastTime_ = now;
}

auto ToolZoomSlider::formatSliderValue(double value) const -> std::string {
    std::stringstream out;
    out << static_cast<int>(100 * value);
    out << "%";
    return out.str();
}

auto ToolZoomSlider::getNewToolPixbuf() const -> GdkPixbuf* { return getPixbufFromImageIconName(); }

void ToolZoomSlider::zoomChanged() {
    GtkRange* slider = getSliderWidget();

    if (slider == nullptr) {
        return;
    }

    pImpl->updateFromZoomChange(slider);
}

void ToolZoomSlider::Impl::updateFromZoomChange(GtkRange* slider) {
    if (sliderChangingBySliderDrag_) {
        return;
    }

    sliderChangingByZoomControlOrInit_ = true;
    double newValue = publicApi_->scaleFunc(zoom_->getZoomReal());
    gtk_range_set_value(slider, newValue);
    sliderChangingByZoomControlOrInit_ = false;
}

void ToolZoomSlider::zoomRangeValuesChanged() {
    pImpl->updateScaleMarks(GTK_SCALE(getSliderWidget()), isCurrentHorizontal());
}

auto ToolZoomSlider::getToolDisplayName() const -> std::string { return _("Zoom Slider"); }

auto ToolZoomSlider::getNewToolIcon() const -> GtkWidget* {
    return gtk_image_new_from_icon_name(pImpl->iconNameHelper_.iconName("zoom-slider").c_str(),
                                        GTK_ICON_SIZE_SMALL_TOOLBAR);
}

void ToolZoomSlider::configure(GtkRange* slider, bool isHorizontal) const {
    AbstractSliderItem::configure(slider, isHorizontal);
    pImpl->updateScaleMarks(GTK_SCALE(slider), isHorizontal);

    double initialValue = scaleFunc(pImpl->zoom_->getZoomReal());
    gtk_range_set_value(slider, initialValue);
}

// Should be called when the window size changes
void ToolZoomSlider::Impl::updateScaleMarks(GtkScale* slider, bool isHorizontal) const {
    if (slider == nullptr) {
        return;
    }

    gtk_scale_clear_marks(slider);
    gtk_scale_add_mark(slider, publicApi_->scaleFunc(1.0), isHorizontal ? GTK_POS_BOTTOM : GTK_POS_RIGHT, nullptr);
    gtk_scale_add_mark(slider, publicApi_->scaleFunc(zoom_->getZoomFitValue() / zoom_->getZoom100Value()),
                       isHorizontal ? GTK_POS_BOTTOM : GTK_POS_RIGHT, nullptr);
}

auto ToolZoomSlider::scaleFunc(double x) const -> double { return std::log(x - SCALE_LOG_OFFSET); }

auto ToolZoomSlider::scaleFuncInv(double x) const -> double { return std::exp(x) + SCALE_LOG_OFFSET; }
