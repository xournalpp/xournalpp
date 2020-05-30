#include "ToolZoomSlider.h"

#include <cmath>
#include <utility>

#include <config.h>

#include "control/zoom/ZoomControl.h"

#include "StringUtils.h"
#include "i18n.h"

ToolZoomSlider::ToolZoomSlider(ActionHandler* handler, string id, ActionType type, ZoomControl* zoom):
        AbstractToolItem(std::move(id), handler, type, nullptr), zoom(zoom) {
    zoom->addZoomListener(this);
}

ToolZoomSlider::~ToolZoomSlider() = default;

void ToolZoomSlider::sliderChanged(GtkRange* range, ToolZoomSlider* self) {
    if (!self->sliderChangingByZoomControlOrInit && !self->zoom->isZoomPresentationMode() &&
        (self->sliderChangingBySliderDrag || self->sliderChangingBySliderHoverScroll)) {
        double back = self->zoom->getZoom100Value() * scaleFuncInv(gtk_range_get_value(range));
        self->zoom->zoomSequenceChange(back, false);
    }
    self->sliderChangingBySliderHoverScroll = false;
}

auto ToolZoomSlider::sliderButtonPress(GtkRange* range, GdkEvent* event, ToolZoomSlider* self) -> bool {
    if (!self->sliderChangingBySliderDrag && !self->zoom->isZoomPresentationMode()) {
        self->zoom->setZoomFitMode(false);
        self->zoom->startZoomSequence();
        self->sliderChangingBySliderDrag = true;
    }
    return false;
}

auto ToolZoomSlider::sliderButtonRelease(GtkRange* range, GdkEvent* event, ToolZoomSlider* self) -> bool {
    self->zoom->endZoomSequence();
    self->sliderChangingBySliderDrag = false;
    return false;
}

auto ToolZoomSlider::sliderHoverScroll(GtkWidget* range, GdkEventScroll* event, ToolZoomSlider* self) -> bool {
    gint64 now = g_get_monotonic_time();
    if (now > self->sliderHoverScrollLastTime + 500) {
        self->zoom->setZoomFitMode(false);
        self->zoom->startZoomSequence();
    }
    self->sliderChangingBySliderHoverScroll = true;
    self->sliderHoverScrollLastTime = now;
    return false;
}

auto ToolZoomSlider::sliderFormatValue(GtkRange* range, gdouble value, ToolZoomSlider* self) -> gchar* {
    return g_strdup_printf("%d%%", static_cast<int>(100 * scaleFuncInv(value)));
}

void ToolZoomSlider::zoomChanged() {
    if (this->slider == nullptr || this->sliderChangingBySliderDrag) {
        return;
    }

    this->sliderChangingByZoomControlOrInit = true;
    double slider_range = scaleFunc(this->zoom->getZoomReal());
    gtk_range_set_value(GTK_RANGE(this->slider), slider_range);
    this->sliderChangingByZoomControlOrInit = false;
}

void ToolZoomSlider::zoomRangeValuesChanged() { updateScaleMarks(); }

auto ToolZoomSlider::getToolDisplayName() -> string { return _("Zoom slider"); }

auto ToolZoomSlider::getNewToolIcon() -> GtkWidget* {
    return gtk_image_new_from_icon_name("zoom-in", GTK_ICON_SIZE_SMALL_TOOLBAR);
}

// Should be called when the window size changes
void ToolZoomSlider::updateScaleMarks() {
    if (this->slider == nullptr) {
        return;
    }

    gtk_scale_clear_marks(GTK_SCALE(this->slider));
    gtk_scale_add_mark(GTK_SCALE(this->slider), scaleFunc(zoom->getZoom100Value()),
                       horizontal ? GTK_POS_BOTTOM : GTK_POS_RIGHT, nullptr);
    gtk_scale_add_mark(GTK_SCALE(this->slider), scaleFunc(zoom->getZoomFitValue()),
                       horizontal ? GTK_POS_BOTTOM : GTK_POS_RIGHT, nullptr);
}

auto ToolZoomSlider::createItem(bool horizontal) -> GtkToolItem* {
    this->horizontal = horizontal;
    this->item = newItem();
    g_object_ref(this->item);

    if (GTK_IS_TOOL_ITEM(this->item)) {
        gtk_tool_item_set_homogeneous(GTK_TOOL_ITEM(this->item), false);
    }

    if (GTK_IS_TOOL_BUTTON(this->item) || GTK_IS_TOGGLE_TOOL_BUTTON(this->item)) {
        g_signal_connect(this->item, "clicked", G_CALLBACK(&toolButtonCallback), this);
    }
    return this->item;
}

auto ToolZoomSlider::createTmpItem(bool horizontal) -> GtkToolItem* {
    GtkToolItem* item = newItem();
    g_object_ref(item);

    if (GTK_IS_TOOL_ITEM(item)) {
        gtk_tool_item_set_homogeneous(GTK_TOOL_ITEM(item), false);
    }

    // no slider marks, but don't matter, because it's only a preview

    gtk_widget_show_all(GTK_WIDGET(item));
    return item;
}

void ToolZoomSlider::enable(bool enabled) {
    if (this->item) {
        gtk_widget_set_sensitive(GTK_WIDGET(this->item), enabled);
    }
    if (this->slider) {
        gtk_widget_set_sensitive(GTK_WIDGET(this->slider), enabled);
    }
}

auto ToolZoomSlider::newItem() -> GtkToolItem* {
    GtkToolItem* it = gtk_tool_item_new();

    if (this->slider) {
        g_signal_handlers_disconnect_by_func(this->slider, (void*)(sliderChanged), this);
        g_signal_handlers_disconnect_by_func(this->slider, (void*)(sliderButtonPress), this);
        g_signal_handlers_disconnect_by_func(this->slider, (void*)(sliderButtonRelease), this);
        g_signal_handlers_disconnect_by_func(this->slider, (void*)(sliderHoverScroll), this);
        g_signal_handlers_disconnect_by_func(this->slider, (void*)(sliderFormatValue), this);
    }

    double sliderMin = scaleFunc(DEFAULT_ZOOM_MIN);
    double sliderMax = scaleFunc(DEFAULT_ZOOM_MAX);
    // slider has 100 steps
    double sliderStep = (sliderMax - sliderMin) / 100;

    if (this->horizontal) {
        this->slider = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, sliderMin, sliderMax, sliderStep);
    } else {
        this->slider = gtk_scale_new_with_range(GTK_ORIENTATION_VERTICAL, sliderMin, sliderMax, sliderStep);
        gtk_range_set_inverted(GTK_RANGE(this->slider), true);
    }

    g_signal_connect(this->slider, "value-changed", G_CALLBACK(sliderChanged), this);
    g_signal_connect(this->slider, "button-press-event", G_CALLBACK(sliderButtonPress), this);
    g_signal_connect(this->slider, "button-release-event", G_CALLBACK(sliderButtonRelease), this);
    g_signal_connect(this->slider, "scroll-event", G_CALLBACK(sliderHoverScroll), this);
    g_signal_connect(this->slider, "format-value", G_CALLBACK(sliderFormatValue), this);
    gtk_scale_set_draw_value(GTK_SCALE(this->slider), true);

    gtk_widget_set_can_focus(this->slider, false);

    if (this->horizontal) {
        gtk_widget_set_size_request(GTK_WIDGET(this->slider), 120, 16);
    } else {
        gtk_widget_set_size_request(GTK_WIDGET(this->slider), 16, 120);
    }

    gtk_container_add(GTK_CONTAINER(it), this->slider);

    sliderChangingByZoomControlOrInit = true;
    double slider_range = scaleFunc(this->zoom->getZoomReal());
    gtk_range_set_value(GTK_RANGE(this->slider), slider_range);
    sliderChangingByZoomControlOrInit = false;

    updateScaleMarks();

    return it;
}

auto ToolZoomSlider::scaleFunc(double x) -> double { return log(x - SCALE_LOG_OFFSET); }

auto ToolZoomSlider::scaleFuncInv(double x) -> double { return exp(x) + SCALE_LOG_OFFSET; }
