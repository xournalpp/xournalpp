#include "ToolZoomSlider.h"

#include <cmath>    // for exp, log
#include <sstream>  // for stringstream, bas...
#include <utility>  // for move

#include <glib.h>  // for g_get_monotonic_time

#include "control/Control.h"
#include "control/actions/ActionDatabase.h"
#include "control/zoom/ZoomControl.h"  // for ZoomControl, DEFA...
#include "gui/XournalView.h"
#include "util/GtkUtil.h"
#include "util/gtk4_helper.h"
#include "util/i18n.h"  // for _
#include "util/raii/GObjectSPtr.h"

constexpr double SCALE_LOG_OFFSET = 0.20753;

constexpr int FINE_STEP_COUNT = 100;
constexpr int COARSE_STEP_COUNT = 10;

// TODO(personalizedrefrigerator): Update the range to reflect the min and max zoom settings.
constexpr auto SLIDER_RANGE =
        AbstractSliderItem::SliderRange{DEFAULT_ZOOM_MIN, DEFAULT_ZOOM_MAX, FINE_STEP_COUNT, COARSE_STEP_COUNT};

ToolZoomSlider::ToolZoomSlider(std::string id, ZoomControl* zoom, IconNameHelper iconNameHelper, ActionDatabase& db):
        AbstractSliderItem{std::move(id), Category::NAVIGATION, SLIDER_RANGE, db.getAction(Action::ZOOM)},
        iconName(iconNameHelper.iconName("zoom-slider")),
        zoomCtrl(zoom) {}

auto ToolZoomSlider::formatSliderValue(double value) -> std::string {
    std::stringstream out;
    out << static_cast<int>(std::round(100 * scaleInverseFunction(value)));
    out << "%";
    return out.str();
}

namespace {
void onZoomSliderValueChanged(GtkRange* range, gpointer data);

class ZoomSliderBinding: public ZoomListener {
public:
    ZoomSliderBinding(GtkScale* slider, ZoomControl* zoomCtrl):
            slider(slider), zoomCtrl(zoomCtrl), alive(zoomCtrl->aliveFlag()) {
        zoomCtrl->addZoomListener(this);
    }
    ~ZoomSliderBinding() override {
        // The GtkWindow finalizes this slider after ZoomControl has already been destroyed.
        if (this->alive && *this->alive && this->zoomCtrl != nullptr) {
            this->zoomCtrl->removeZoomListener(this);
        }
    }
    void syncThumb() {
        if (!this->alive || !*this->alive) {
            return;
        }
        g_signal_handlers_block_by_func(slider, reinterpret_cast<gpointer>(onZoomSliderValueChanged), this);
        gtk_range_set_value(GTK_RANGE(slider), ToolZoomSlider::scaleFunction(zoomCtrl->getZoomReal()));
        g_signal_handlers_unblock_by_func(slider, reinterpret_cast<gpointer>(onZoomSliderValueChanged), this);
    }
    void zoomChanged() override { syncThumb(); }
    void zoomRangeValuesChanged() override {
        if (!this->alive || !*this->alive) {
            return;
        }
        gtk_scale_clear_marks(slider);
        auto position = gtk_orientable_get_orientation(GTK_ORIENTABLE(slider)) == GTK_ORIENTATION_HORIZONTAL ?
                                GTK_POS_BOTTOM :
                                GTK_POS_RIGHT;
        gtk_scale_add_mark(slider, ToolZoomSlider::scaleFunction(1.0), position, nullptr);
        gtk_scale_add_mark(slider,
                           ToolZoomSlider::scaleFunction(zoomCtrl->getZoomFitValue() / zoomCtrl->getZoom100Value()),
                           position, nullptr);
        syncThumb();
    }

    GtkScale* slider;
    ZoomControl* zoomCtrl;
    std::shared_ptr<bool> alive;
};

void onZoomSliderValueChanged(GtkRange* range, gpointer data) {
    auto* binding = static_cast<ZoomSliderBinding*>(data);
    if (binding == nullptr || !binding->alive || !*binding->alive) {
        return;
    }
    ZoomControl* zoom = binding->zoomCtrl;
    const double scale = ToolZoomSlider::scaleInverseFunction(gtk_range_get_value(range));
    if (XournalView* view = zoom->getView(); view != nullptr) {
        view->getControl()->focusWindowFrom(view->getWidget());
    }
    zoom->setZoomFitMode(false);
    zoom->startZoomSequence();
    zoom->zoomSequenceChange(zoom->getZoom100Value() * scale, false);
    zoom->endZoomSequence();
}
}  // namespace

auto ToolZoomSlider::createItem(bool horizontal) -> xoj::util::WidgetSPtr {
    GtkOrientation orientation = horizontal ? GTK_ORIENTATION_HORIZONTAL : GTK_ORIENTATION_VERTICAL;
    const double min = scaleFunction(this->range.min);
    const double max = scaleFunction(this->range.max);
    const double fineStepSize = (max - min) / this->range.nbFineSteps;
    const double coarseStepSize = (max - min) / this->range.nbCoarseSteps;

    GtkRange* slider = GTK_RANGE(gtk_scale_new_with_range(orientation, min, max, fineStepSize));
    gtk_range_set_increments(slider, fineStepSize, coarseStepSize);
    if (horizontal) {
        gtk_widget_set_size_request(GTK_WIDGET(slider), 120, 16);
    } else {
        gtk_widget_set_size_request(GTK_WIDGET(slider), 16, 120);
    }
    gtk_widget_set_can_focus(GTK_WIDGET(slider), false);

    // The thumb follows this window's ZoomControl. The shared zoom action would move every window's slider.
    auto data = std::make_unique<ZoomSliderBinding>(GTK_SCALE(slider), zoomCtrl);
    gtk_range_set_value(slider, scaleFunction(this->zoomCtrl->getZoomReal()));
    g_signal_connect(slider, "value-changed", G_CALLBACK(onZoomSliderValueChanged), data.get());

    // Presentation mode disables the shared zoom action, which should disable every slider.
    xoj::util::gtk::setWidgetFollowActionEnabled(GTK_WIDGET(slider), G_ACTION(this->gAction.get()));

    gtk_scale_set_draw_value(GTK_SCALE(slider), true);
    gtk_scale_set_format_value_func(
            GTK_SCALE(slider),
            +[](GtkScale*, double value, gpointer) -> char* { return g_strdup(formatSliderValue(value).c_str()); },
            nullptr, nullptr);

    data->zoomRangeValuesChanged();

    xoj::util::WidgetSPtr item(GTK_WIDGET(slider), xoj::util::adopt);
    g_object_weak_ref(
            G_OBJECT(item.get()), +[](gpointer d, GObject*) { delete static_cast<ZoomSliderBinding*>(d); },
            data.release());
    return item;
}

auto ToolZoomSlider::getToolDisplayName() const -> std::string { return _("Zoom Slider"); }

auto ToolZoomSlider::getNewToolIcon() const -> GtkWidget* {
    return gtk_image_new_from_icon_name(iconName.c_str(), GTK_ICON_SIZE_SMALL_TOOLBAR);
}

auto ToolZoomSlider::scaleFunction(double x) -> double { return std::log(x - SCALE_LOG_OFFSET); }

auto ToolZoomSlider::scaleInverseFunction(double x) -> double { return std::exp(x) + SCALE_LOG_OFFSET; }
