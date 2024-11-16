#include "ZoomControl.h"

#include <algorithm>  // for find, max

#include <glib-object.h>  // for G_CALLBACK, g_signal_connect
#include <glib.h>         // for g_warning, guint

#include "control/Control.h"  // for Control
#include "control/actions/ActionDatabase.h"
#include "control/settings/Settings.h"  // for Settings
#include "control/zoom/ZoomListener.h"  // for ZoomListener
#include "gui/Layout.h"                 // for Layout
#include "gui/PageView.h"               // for XojPageView
#include "gui/XournalView.h"            // for XournalView
#include "gui/widgets/XournalWidget.h"  // for gtk_xournal_get_layout
#include "util/Assert.h"                // for xoj_assert
#include "util/Util.h"                  // for execInUiThread
#include "util/gdk4_helper.h"           // for gdk_event_get_modifier_state
#include "util/glib_casts.h"            // for wrap_for_g_callback

using xoj::util::Rectangle;

// auto onScrolledwindowMainScrollEvent(GtkWidget* widget, GdkEventScroll* event, ZoomControl* zoom) -> bool {
//     auto state =
//             gdk_event_get_modifier_state(reinterpret_cast<GdkEvent*>(event)) &
//             gtk_accelerator_get_default_mod_mask();
//
//     // do not handle e.g. ALT + Scroll (e.g. Compiz use this shortcut for setting transparency...)
//     if (state & ~(GDK_CONTROL_MASK | GDK_SHIFT_MASK)) {
//         return true;
//     }
//
//     if (state & GDK_CONTROL_MASK) {
//         auto direction =
//                 (event->direction == GDK_SCROLL_UP || (event->direction == GDK_SCROLL_SMOOTH && event->delta_y < 0))
//                 ?
//                         ZOOM_IN :
//                         ZOOM_OUT;
//         // translate absolute window coordinates to the widget-local coordinates and start zooming
//         zoom->zoomScroll(direction, Util::toWidgetCoords(widget, xoj::util::Point{event->x_root, event->y_root}));
//         return true;
//     }
//
//     // TODO(unknown): Disabling scroll here is maybe a bit hacky find a better way
//     return zoom->isZoomPresentationMode();
// }
//
// auto onTouchpadPinchEvent(GtkWidget* widget, GdkEventTouchpadPinch* event, ZoomControl* zoom) -> bool {
//     if (event->type == GDK_TOUCHPAD_PINCH && event->n_fingers == 2) {
//         switch (event->phase) {
//             case GDK_TOUCHPAD_GESTURE_PHASE_BEGIN:
//                 if (zoom->isZoomFitMode()) {
//                     zoom->setZoomFitMode(false);
//                 }
//                 // translate absolute window coordinates to the widget-local coordinates and start zooming
//                 zoom->startZoomSequence(Util::toWidgetCoords(widget, xoj::util::Point{event->x_root,
//                 event->y_root})); break;
//             case GDK_TOUCHPAD_GESTURE_PHASE_UPDATE:
//                 zoom->zoomSequenceChange(event->scale, true);
//                 break;
//             case GDK_TOUCHPAD_GESTURE_PHASE_END:
//                 zoom->endZoomSequence();
//                 break;
//             case GDK_TOUCHPAD_GESTURE_PHASE_CANCEL:
//                 zoom->cancelZoomSequence();
//                 break;
//         }
//         return true;
//     }
//     return false;
// }


// Todo: try to connect this function with the "expose_event", it would be way cleaner and we dont need to align/layout
//       the pages manually, but it only works with the top Widget (GtkWindow) for now this works "fine"
//       see https://stackoverflow.com/questions/1060039/gtk-detecting-window-resize-from-the-user
auto onWindowSizeChangedEvent(GtkWidget* widget, GdkEvent* event, ZoomControl* zoom) -> bool {
    xoj_assert(widget != zoom->view->getWidget());
    auto layout = gtk_xournal_get_layout(zoom->view->getWidget());
    // Todo (fabian): The following code is a hack.
    //    Problem size-allocate:
    //    when using the size-allocate signal, we cant use layout->recalculate() directly.
    //    But using the xournal-widgets allocation is wrong, since the calculation is skipped already.
    //    using size-allocate's allocation is wrong, since it is the alloc of the toplevel.
    //    Problem expose-event:
    //    when using the expose-event signal, the new Allocation is not known yet; calculation must be deferred.
    //    (minimize / maximize wont work)
    Util::execInUiThread([layout, zoom]() {
        zoom->updateZoomPresentationValue();
        zoom->updateZoomFitValue();
        layout->recalculate();
    });
    return false;
}

auto ZoomControl::withZoomStep(ZoomDirection direction, double zoomStep) const -> double {
    double multiplier = 1.0 + zoomStep;
    double newZoom;

    if (direction == ZOOM_IN) {
        newZoom = this->zoom * multiplier;
    } else {
        newZoom = this->zoom / multiplier;
    }

    return newZoom;
}

void ZoomControl::zoomOneStep(ZoomDirection direction, xoj::util::Point<double> zoomCenter) {
    if (this->zoomPresentationMode) {
        return;
    }
    this->setZoomFitMode(false);

    double newZoom = this->withZoomStep(direction, this->zoomStep);

    startZoomSequence(zoomCenter);
    this->zoomSequenceChange(newZoom, false);
    endZoomSequence();
}

void ZoomControl::zoomOneStep(ZoomDirection direction) {
    Rectangle rect = getVisibleRect();
    zoomOneStep(direction, {rect.width / 2.0, rect.height / 2.0});
}


void ZoomControl::zoomScroll(ZoomDirection direction, xoj::util::Point<double> zoomCenter) {
    if (this->zoomPresentationMode) {
        return;
    }

    if (this->isZoomFitMode()) {
        this->setZoomFitMode(false);
    }

    double newZoom = this->withZoomStep(direction, this->zoomStepScroll);
    startZoomSequence(zoomCenter);
    this->zoomSequenceChange(newZoom, false);
    endZoomSequence();
}

void ZoomControl::startZoomSequence() {
    Rectangle rect = getVisibleRect();
    startZoomSequence({rect.width / 2.0, rect.height / 2.0});
}

void ZoomControl::startZoomSequence(xoj::util::Point<double> zoomCenter) {
    // * set zoom center and zoom startlevel
    this->zoomWidgetPos = zoomCenter;  // widget space coordinates of the zoomCenter!
    this->zoomSequenceStart = this->zoom;

    // * set unscaledPixels padding value
    size_t currentPageIdx = this->view->getCurrentPage();

    // To get the layout, we need to call view->getWidget(), which isn't const.
    // As such, we get the view and determine `unscaledPixels` here, rather than
    // in `getScrollPositionAfterZoom`.
    GtkWidget* widget = view->getWidget();
    Layout* layout = gtk_xournal_get_layout(widget);

    // Not everything changes size as we zoom in/out. The padding, for example,
    // remains constant! (changed when page changes, but the error stays small enough)
    this->unscaledPixels = {static_cast<double>(layout->getPaddingLeftOfPage(currentPageIdx)),
                            static_cast<double>(layout->getPaddingAbovePage(currentPageIdx))};

    // * set initial scrollPosition value
    auto const& rect = getVisibleRect();
    auto const& view_pos = xoj::util::Point{rect.x, rect.y};

    // Use this->zoomWidgetPos to zoom into a location other than the top-left (e.g. where
    // the user pinched).
    this->scrollPosition = (view_pos + this->zoomWidgetPos - this->unscaledPixels) / this->zoom;
}

void ZoomControl::zoomSequenceChange(double zoom, bool relative) {
    if (relative) {
        if (isZoomSequenceActive()) {
            zoom *= zoomSequenceStart;
        } else {
            zoom *= this->zoom;
        }
    }

    setZoom(zoom);
}

void ZoomControl::zoomSequenceChange(double zoom, bool relative, xoj::util::Point<double> scrollVector) {
    if (relative) {
        if (isZoomSequenceActive()) {
            zoom *= zoomSequenceStart;
        } else {
            zoom *= this->zoom;
        }
    }

    // scroll update
    this->zoomWidgetPos += scrollVector;

    setZoom(zoom);
}

void ZoomControl::endZoomSequence() {
    scrollPosition = {-1, -1};
    zoomSequenceStart = -1;
}

void ZoomControl::cancelZoomSequence() {
    if (isZoomSequenceActive()) {
        setZoom(zoomSequenceStart);
        endZoomSequence();
    }
}

auto ZoomControl::isZoomSequenceActive() const -> bool { return zoomSequenceStart != -1; }

auto ZoomControl::getVisibleRect() -> Rectangle<double> {
    GtkWidget* widget = view->getWidget();
    Layout* layout = gtk_xournal_get_layout(widget);
    return layout->getVisibleRect();
}

auto ZoomControl::getScrollPositionAfterZoom() const -> xoj::util::Point<double> {
    //  If we aren't in a zoomSequence, `unscaledPixels`, `scrollPosition`, and `zoomWidgetPos
    // can't be used to determine the scroll position! Return now.
    // NOTE: this case should never happen currently.
    //       getScrollPositionAfterZoom is called from XournalView after setZoom() fired the ZoomListeners
    if (!this->isZoomSequenceActive()) {
        xoj_assert_message(false, "ZoomControl::getScrollPositionAfterZoom() was called outside of a zoom sequence.");
        return {0, 0};
    }

    return this->scrollPosition * this->zoom - this->zoomWidgetPos + this->unscaledPixels;
}

void ZoomControl::addZoomListener(ZoomListener* l) { this->listener.emplace_back(l); }

void ZoomControl::removeZoomListener(ZoomListener* l) {
    if (auto it = std::find(this->listener.begin(), this->listener.end(), l); it != this->listener.end()) {
        this->listener.erase(it);
    }
}

void ZoomControl::initZoomHandler(GtkWidget* window, GtkWidget* widget, XournalView* v, Control* c) {
    this->control = c;
    this->view = v;
    g_warning("Implement ZoomHandler");
    // gtk_widget_add_events(widget, GDK_TOUCHPAD_GESTURE_MASK);
    // g_signal_connect(widget, "scroll-event", xoj::util::wrap_for_g_callback_v<onScrolledwindowMainScrollEvent>,
    // this); g_signal_connect(widget, "event", xoj::util::wrap_for_g_callback_v<onTouchpadPinchEvent>, this);
    g_signal_connect(window, "configure-event", xoj::util::wrap_for_g_callback_v<onWindowSizeChangedEvent>, this);
    registerListener(this->control);
}

void ZoomControl::fireZoomChanged() {
    for (ZoomListener* z: this->listener) {
        z->zoomChanged();
    }
}

void ZoomControl::fireZoomRangeValueChanged() {
    for (ZoomListener* z: this->listener) {
        z->zoomRangeValuesChanged();
    }
}

auto ZoomControl::getZoom() const -> double { return this->zoom; }

auto ZoomControl::getZoomReal() const -> double { return this->zoom / this->zoom100Value; }

void ZoomControl::setZoom(double zoomI) {
    zoomI = std::clamp(zoomI, this->zoomMin, this->zoomMax);
    if (this->zoom == zoomI) {
        return;
    }
    this->zoom = zoomI;
    this->control->getActionDatabase()->setActionState(Action::ZOOM, getZoomReal());
    fireZoomChanged();
}

void ZoomControl::setZoom100Value(double zoom100Val) {
    auto setWithRelZoom = [zoomOld = this->zoom100Value, zoom100Val](double& val) { val = val / zoomOld * zoom100Val; };
    setWithRelZoom(this->zoomMax);
    setWithRelZoom(this->zoomMin);
    this->zoom100Value = zoom100Val;
    fireZoomRangeValueChanged();
}

auto ZoomControl::updateZoomFitValue(size_t pageNo) -> bool {
    if (pageNo == 0) {
        pageNo = view->getCurrentPage();
    }
    XojPageView* page = view->getViewFor(pageNo);
    if (!page) {  // no page
        return false;
    }

    Rectangle widget_rect = getVisibleRect();
    double zoom_fit_width = widget_rect.width / (page->getWidth() + 20.0);

    this->zoomFitValue = std::clamp(zoom_fit_width, this->zoomMin, this->zoomMax);
    fireZoomRangeValueChanged();
    if (this->isZoomFitMode() && !this->zoomPresentationMode) {
        this->zoomFit();
    }
    return true;
}

auto ZoomControl::getZoomFitValue() const -> double { return this->zoomFitValue; }

auto ZoomControl::updateZoomPresentationValue(size_t pageNo) -> bool {
    XojPageView* page = view->getViewFor(view->getCurrentPage());
    if (!page) {
        g_warning("Cannot update zoomPresentationValue yet. This should only happen on startup! ");
        return true;
    }

    Rectangle widget_rect = getVisibleRect();
    double zoom_fit_width = widget_rect.width / (page->getWidth() + 14.0);
    double zoom_fit_height = widget_rect.height / (page->getHeight() + 14.0);
    double zoom_presentation = zoom_fit_width < zoom_fit_height ? zoom_fit_width : zoom_fit_height;

    this->zoomPresentationValue = std::clamp(zoom_presentation, this->zoomMin, this->zoomMax);
    if (this->zoomPresentationMode) {
        this->zoomPresentation();
    }
    return true;
}

auto ZoomControl::getZoom100Value() const -> double { return this->zoom100Value; }

void ZoomControl::zoom100() {
    if (this->zoomPresentationMode) {
        return;
    }

    if (this->zoomFitMode) {
        this->setZoomFitMode(false);
    }

    startZoomSequence();
    this->zoomSequenceChange(this->zoom100Value, false);
    endZoomSequence();
}

void ZoomControl::zoomFit() {
    // TODO: properly fix the zoom fit infinite recursion
    if (this->isZoomFittingNow) {
        return;
    }

    this->isZoomFittingNow = true;
    if (this->isZoomFitMode() && !this->zoomPresentationMode && this->zoom != this->zoomFitValue) {
        startZoomSequence();
        this->zoomSequenceChange(this->zoomFitValue, false);
        endZoomSequence();
    }
    this->isZoomFittingNow = false;
}

void ZoomControl::zoomPresentation() {
    if (this->zoomPresentationMode && this->zoom != this->zoomPresentationValue) {
        startZoomSequence();
        this->zoomSequenceChange(this->zoomPresentationValue, false);
        endZoomSequence();
    }
}

void ZoomControl::setZoomFitMode(bool isZoomFitMode) {
    if (this->zoomFitMode != isZoomFitMode) {
        this->zoomFitMode = isZoomFitMode;
        this->control->getActionDatabase()->setActionState(Action::ZOOM_FIT, this->zoomFitMode);
    }

    if (this->isZoomFitMode()) {
        zoomFit();
    }
}

auto ZoomControl::isZoomFitMode() const -> bool {
    // Todo(fabian): Remove this fix and make both modes possible in parallel, after fixing the infinite loop.
    //  Explanation: First of all, zoomFit never worked with paired pages. Also, using both resulted in a stackoverflow
    //               when different page sizes are used. This is caused by a logical loop.
    //               We decided to deactivate it in PR#2821 & I#2770. instead of fixing it, to get release 1.1.0 ready.
    //               Zoom presentation mode is also excluded, because it was never intended to work together.
    //               It is also excluded everywhere else (duplicate code).
    auto infiniteLoopFixup = !this->zoomPresentationMode && !this->control->getSettings()->isShowPairedPages();
    return this->zoomFitMode && infiniteLoopFixup;
}

void ZoomControl::setZoomPresentationMode(bool isZoomPresentationMode) {
    this->zoomPresentationMode = isZoomPresentationMode;

    if (isZoomPresentationMode) {
        zoomPresentation();
    }
}

auto ZoomControl::isZoomPresentationMode() const -> bool { return this->zoomPresentationMode; }

void ZoomControl::setZoomStep(double zoomStep) { this->zoomStep = zoomStep; }

void ZoomControl::setZoomStepScroll(double zoomStep) { this->zoomStepScroll = zoomStep; }

void ZoomControl::pageSizeChanged(size_t page) {
    updateZoomPresentationValue(page);
    updateZoomFitValue(page);
}

void ZoomControl::pageSelected(size_t page) {
    // Todo (fabian): page selected should do nothing here, since Zoom Controls, which page is selected.
    //                This results in a logical loop. See PR#2821 & I#2770
    if (current_page != page) {
        this->last_page = this->current_page;
        this->current_page = page;
    }

    updateZoomPresentationValue(this->current_page);
    if (view->isPageVisible(this->last_page, nullptr)) {
        return;
    }
    updateZoomFitValue(this->current_page);
}
