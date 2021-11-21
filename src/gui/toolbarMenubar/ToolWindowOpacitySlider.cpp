#include "ToolWindowOpacitySlider.h"

#include "control/Control.h"
#include "gui/MainWindow.h"
#include "util/IconNameHelper.h"

#include "i18n.h"

const double MIN_OPACITY = 0.2;
const double MAX_OPACITY = 1.0;
const double OPACITY_SMALL_STEP_COUNT = 10.0;
const double OPACITY_LARGE_STEP_COUNT = 5.0;

static const auto OPACITY_SLIDER_RANGE =
        AbstractSliderItem::SliderRange{MIN_OPACITY, MAX_OPACITY, OPACITY_SMALL_STEP_COUNT, OPACITY_LARGE_STEP_COUNT};

// TODO: Might be simpler to not use pImpl pattern here?

class ToolWindowOpacitySlider::Impl {
public:
    Impl(ToolWindowOpacitySlider* parent, IconNameHelper iconNameHelper, Control* ctrl);
    GtkWidget* getNewToolIcon() const;

    void onSliderChanged(double value);

    double getCurrentOpacity() const;

private:
    /**
     * Notify the user that their system/configruation
     * does not support changing the root window's opacity
     * if this is the case.
     *
     * Must be run on the UI thread.
     */
    void warnIfOpacityNotSupported();

    /**
     * Resets or initializes the tooltip displayed for the
     * slider.
     */
    void resetTooltip();

private:
    IconNameHelper iconNameHelper_;
    Control* control_ = nullptr;
    ToolWindowOpacitySlider* parent_;
};

ToolWindowOpacitySlider::~ToolWindowOpacitySlider() = default;

ToolWindowOpacitySlider::ToolWindowOpacitySlider(std::string id, ActionHandler* handler, ActionType type,
                                                 Control* control, IconNameHelper iconNameHelper):
        AbstractSliderItem{std::move(id), handler, type, OPACITY_SLIDER_RANGE},
        pImpl{std::make_unique<Impl>(this, std::move(iconNameHelper), control)} {}

auto ToolWindowOpacitySlider::getToolDisplayName() -> std::string { return _("Window Opacity Slider"); }

void ToolWindowOpacitySlider::configure(GtkRange* slider, bool horizontal) const {
    AbstractSliderItem::configure(slider, horizontal);
    gtk_range_set_value(GTK_RANGE(slider), scaleFuncInv(pImpl->getCurrentOpacity()));
}

void ToolWindowOpacitySlider::onSliderChanged(double value) { pImpl->onSliderChanged(value); }

auto ToolWindowOpacitySlider::getNewToolIcon() -> GtkWidget* { return pImpl->getNewToolIcon(); }

auto ToolWindowOpacitySlider::scaleFunc(double x) const -> double { return x; }

auto ToolWindowOpacitySlider::scaleFuncInv(double x) const -> double { return x; }

ToolWindowOpacitySlider::Impl::Impl(ToolWindowOpacitySlider* parent, IconNameHelper iconNameHelper, Control* control):
        parent_{parent}, iconNameHelper_{iconNameHelper}, control_{control} {}

auto ToolWindowOpacitySlider::Impl::getNewToolIcon() const -> GtkWidget* {
    return gtk_image_new_from_icon_name(iconNameHelper_.iconName("window-opacity-slider").c_str(),
                                        GTK_ICON_SIZE_SMALL_TOOLBAR);
}

auto ToolWindowOpacitySlider::Impl::getCurrentOpacity() const -> double {
    MainWindow* window = control_->getWindow();

    if (window != nullptr) {
        return window->getOpacity();
    }

    return 1.0;
}

void ToolWindowOpacitySlider::Impl::onSliderChanged(double value) {
    MainWindow* window = control_->getWindow();
    warnIfOpacityNotSupported();

    if (window != nullptr && window->supportsOpacity()) {
        window->setOpacity(value);
    }
}

void ToolWindowOpacitySlider::Impl::warnIfOpacityNotSupported() {
    MainWindow* window = control_->getWindow();

    if (window != nullptr) {
        if (!window->supportsOpacity()) {
            GtkRange* slider = parent_->getSliderWidget();

            // If the display doesn't support changing the opacity of the window,
            // change the tooltip's text and show it.
            gtk_widget_set_tooltip_text(GTK_WIDGET(slider), _("Window opacity: Not supported by your system"));
            gtk_widget_trigger_tooltip_query(GTK_WIDGET(slider));

            gtk_range_set_value(GTK_RANGE(slider), 1.0);
        } else {
            resetTooltip();
        }
    }
}

void ToolWindowOpacitySlider::Impl::resetTooltip() {
    GtkRange* slider = parent_->getSliderWidget();
    gtk_widget_set_tooltip_text(GTK_WIDGET(slider), _("Window opacity"));
}
