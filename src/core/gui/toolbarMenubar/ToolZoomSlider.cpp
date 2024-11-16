#include "ToolZoomSlider.h"

#include <cmath>    // for exp, log
#include <sstream>  // for stringstream, bas...
#include <utility>  // for move

#include <glib.h>  // for g_get_monotonic_time

#include "control/actions/ActionDatabase.h"
#include "control/zoom/ZoomControl.h"  // for ZoomControl, DEFA...
#include "util/i18n.h"                 // for _

#include "SliderItemCreationHelper.h"

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

auto ToolZoomSlider::createItem(bool horizontal) -> xoj::util::WidgetSPtr {
    auto item = SliderItemCreationHelper<ToolZoomSlider>::createItem(this, horizontal);

    class Listener: public ZoomListener {
    public:
        Listener(GtkScale* slider, ZoomControl* zoomCtrl): slider(slider), zoomCtrl(zoomCtrl) {
            zoomCtrl->addZoomListener(this);
        }
        ~Listener() override { zoomCtrl->removeZoomListener(this); }
        void zoomChanged() override {}  // No need to do anything here. Handled by the GAction
        void zoomRangeValuesChanged() override {
            gtk_scale_clear_marks(slider);
            auto position = gtk_orientable_get_orientation(GTK_ORIENTABLE(slider)) == GTK_ORIENTATION_HORIZONTAL ?
                                    GTK_POS_BOTTOM :
                                    GTK_POS_RIGHT;
            gtk_scale_add_mark(slider, scaleFunction(1.0), position, nullptr);
            gtk_scale_add_mark(slider, scaleFunction(zoomCtrl->getZoomFitValue() / zoomCtrl->getZoom100Value()),
                               position, nullptr);
        }

        GtkScale* slider;  ///< Parent to this data
        ZoomControl* zoomCtrl;
    };

    auto data = std::make_unique<Listener>(GTK_SCALE(item.get()), zoomCtrl);
    data->zoomRangeValuesChanged();  // Set up the marks

    // Destroy *data if the widget is destroyed
    g_object_weak_ref(
            G_OBJECT(item.get()), +[](gpointer d, GObject*) { delete static_cast<Listener*>(d); }, data.release());

    return item;
}

auto ToolZoomSlider::getToolDisplayName() const -> std::string { return _("Zoom Slider"); }

auto ToolZoomSlider::getNewToolIcon() const -> GtkWidget* { return gtk_image_new_from_icon_name(iconName.c_str()); }

auto ToolZoomSlider::scaleFunction(double x) -> double { return std::log(x - SCALE_LOG_OFFSET); }

auto ToolZoomSlider::scaleInverseFunction(double x) -> double { return std::exp(x) + SCALE_LOG_OFFSET; }
