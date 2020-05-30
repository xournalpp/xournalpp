#include "ZoomControl.h"

#include <cmath>

#include "control/Control.h"
#include "gui/Layout.h"
#include "gui/PageView.h"
#include "gui/XournalView.h"
#include "gui/widgets/XournalWidget.h"

void ZoomControl::zoomOneStep(ZoomDirection direction, utl::Point<double> zoomCenter) {
    if (this->zoomPresentationMode) {
        return;
    }
    this->setZoomFitMode(false);

    startZoomSequence(zoomCenter);
    double newZoom = (direction == ZOOM_IN) ? this->zoom + this->zoomStep : this->zoom - this->zoomStep;
    this->zoomSequenceChange(newZoom, false);
    endZoomSequence();
}

void ZoomControl::zoomOneStep(ZoomDirection direction) {
    Rectangle rect = getVisibleRect();
    zoomOneStep(direction, {rect.x + rect.width / 2.0, rect.y + rect.height / 2.0});
}


void ZoomControl::zoomScroll(ZoomDirection zoomIn, utl::Point<double> zoomCenter) {
    if (this->zoomPresentationMode) {
        return;
    }

    if (this->zoomFitMode) {
        this->setZoomFitMode(false);
    }

    if (this->zoomSequenceStart == -1 || scrollCursorPosition != zoomCenter) {
        scrollCursorPosition = zoomCenter;
        startZoomSequence(zoomCenter);
    }

    double newZoom = this->zoom + (zoomIn ? this->zoomStepScroll : -this->zoomStepScroll);
    this->zoomSequenceChange(newZoom, false);
}

void ZoomControl::startZoomSequence() {
    Rectangle rect = getVisibleRect();
    startZoomSequence({rect.x + rect.width / 2.0, rect.y + rect.height / 2.0});
}

void ZoomControl::startZoomSequence(utl::Point<double> zoomCenter) {
    auto const& rect = getVisibleRect();
    auto const& view_pos = utl::Point{rect.x, rect.y};

    this->zoomWidgetPos = zoomCenter - view_pos;
    this->scrollPosition = (view_pos + this->zoomWidgetPos) / this->zoom;
    this->zoomSequenceStart = this->zoom;
}

void ZoomControl::zoomSequenceChange(double zoom, bool relative) {
    if (relative && this->zoomSequenceStart != -1) {
        zoom *= zoomSequenceStart;
    }

    setZoom(zoom);
}

void ZoomControl::endZoomSequence() {
    scrollPosition = {-1, -1};
    zoomSequenceStart = -1;
}

auto ZoomControl::getVisibleRect() -> Rectangle<double> {
    GtkWidget* widget = view->getWidget();
    Layout* layout = gtk_xournal_get_layout(widget);
    return layout->getVisibleRect();
}

void ZoomControl::setScrollPositionAfterZoom(utl::Point<double> scrollPos) {
    this->scrollPosition = (scrollPos + this->zoomWidgetPos) / this->zoom;
}

auto ZoomControl::getScrollPositionAfterZoom() const -> utl::Point<double> {
    if (this->zoomSequenceStart == -1) {
        return {-1, -1};
    }

    return (this->scrollPosition * this->zoom) - this->zoomWidgetPos;
}


void ZoomControl::addZoomListener(ZoomListener* l) { this->listener.emplace_back(l); }

void ZoomControl::initZoomHandler(GtkWidget* widget, XournalView* v, Control* c) {
    this->control = c;
    this->view = v;
    g_signal_connect(widget, "scroll_event", G_CALLBACK(onScrolledwindowMainScrollEvent), this);
    g_signal_connect(widget, "size-allocate", G_CALLBACK(onWidgetSizeChangedEvent), this);

    registerListener(this->control);
}

void ZoomControl::fireZoomChanged() {
    if (this->zoom < this->zoomMin) {
        this->zoom = this->zoomMin;
    }

    if (this->zoom > this->zoomMax) {
        this->zoom = this->zoomMax;
    }

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
    this->zoom = zoomI;
    fireZoomChanged();
}

void ZoomControl::setZoom100Value(double zoom100Val) {
    auto setWithRelZoom = [zoomOld = this->zoom100Value, zoom100Val](double& val) { val = val / zoomOld * zoom100Val; };
    setWithRelZoom(this->zoomStep);
    setWithRelZoom(this->zoomStepScroll);
    setWithRelZoom(this->zoomMax);
    setWithRelZoom(this->zoomMin);
    this->zoom100Value = zoom100Val;
    fireZoomRangeValueChanged();
}

auto ZoomControl::updateZoomFitValue(size_t pageNo) -> bool { return updateZoomFitValue(getVisibleRect(), pageNo); }

auto ZoomControl::updateZoomFitValue(const Rectangle<double>& widget_rect, size_t pageNo) -> bool {
    if (pageNo == 0) {
        pageNo = view->getCurrentPage();
    }
    XojPageView* page = view->getViewFor(pageNo);
    if (!page) {  // no page
        return false;
    }

    double zoom_fit_width = widget_rect.width / (page->getWidth() + 20.0);
    if (zoom_fit_width < this->zoomMin || zoom_fit_width > this->zoomMax) {
        return false;
    }

    this->zoomFitValue = zoom_fit_width;
    fireZoomRangeValueChanged();
    if (this->zoomFitMode && !this->zoomPresentationMode) {
        this->setZoomFitMode(true);
    }
    return true;
}

auto ZoomControl::getZoomFitValue() const -> double { return this->zoomFitValue; }

auto ZoomControl::updateZoomPresentationValue(size_t pageNo) -> bool {
    XojPageView* page = view->getViewFor(view->getCurrentPage());
    if (!page) {
        // no page
        return false;
    }

    Rectangle widget_rect = getVisibleRect();
    double zoom_fit_width = widget_rect.width / (page->getWidth() + 14.0);
    double zoom_fit_height = widget_rect.height / (page->getHeight() + 14.0);
    double zoom_presentation = zoom_fit_width < zoom_fit_height ? zoom_fit_width : zoom_fit_height;
    if (zoom_presentation < this->zoomMin) {
        return false;
    }

    this->zoomPresentationValue = zoom_presentation;
    if (this->zoomPresentationMode) {
        this->setZoomPresentationMode(true);
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
    if (this->zoomFitMode && !this->zoomPresentationMode && this->zoom != this->zoomFitValue) {
        startZoomSequence();
        this->zoomSequenceChange(this->zoomFitValue, false);
        endZoomSequence();
    }
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
        this->control->fireActionSelected(GROUP_ZOOM_FIT, isZoomFitMode ? ACTION_ZOOM_FIT : ACTION_NOT_SELECTED);
    }

    if (isZoomFitMode) {
        zoomFit();
    }
}

auto ZoomControl::isZoomFitMode() const -> bool { return this->zoomFitMode; }

void ZoomControl::setZoomPresentationMode(bool isZoomPresentationMode) {
    this->zoomPresentationMode = isZoomPresentationMode;

    if (isZoomPresentationMode) {
        zoomPresentation();
    }
}

auto ZoomControl::isZoomPresentationMode() const -> bool { return this->zoomPresentationMode; }

void ZoomControl::setZoomStep(double zoomStep) {
    this->zoomStep = zoomStep * this->zoom100Value;
}

void ZoomControl::setZoomStepScroll(double zoomStep) {
    this->zoomStepScroll = zoomStep * this->zoom100Value;
}

void ZoomControl::pageSizeChanged(size_t page) {
    updateZoomPresentationValue(page);
    updateZoomFitValue(page);
}

void ZoomControl::pageSelected(size_t page) {
    updateZoomPresentationValue(page);
    updateZoomFitValue(page);
}

auto ZoomControl::onScrolledwindowMainScrollEvent(GtkWidget* widget, GdkEventScroll* event, ZoomControl* zoom) -> bool {
    guint state = event->state & gtk_accelerator_get_default_mod_mask();

    // do not handle e.g. ALT + Scroll (e.g. Compiz use this shortcut for setting transparency...)
    if (state != 0 && (state & ~(GDK_CONTROL_MASK | GDK_SHIFT_MASK))) {
        return true;
    }

    if (state & GDK_CONTROL_MASK) {
        auto direction =
                (event->direction == GDK_SCROLL_UP || (event->direction == GDK_SCROLL_SMOOTH && event->delta_y < 0)) ?
                        ZOOM_IN :
                        ZOOM_OUT;
        zoom->zoomScroll(direction, {event->x, event->y});
        return true;
    }

    // TODO(unknown): Disabling scroll here is maybe a bit hacky find a better way
    return zoom->isZoomPresentationMode();
}


// Todo: try to connect this function with the "expose_event", it would be way cleaner and we dont need to allign/layout
//       the pages manually, but it only works with the top Widget (GtkWindow) for now this works fine
//       see https://stackoverflow.com/questions/1060039/gtk-detecting-window-resize-from-the-user
auto ZoomControl::onWidgetSizeChangedEvent(GtkWidget* widget, GdkRectangle* allocation, ZoomControl* zoom) -> bool {
    g_assert_true(widget != zoom->view->getWidget());

    Rectangle<double> r(allocation->x, allocation->y, allocation->width, allocation->height);

    zoom->updateZoomPresentationValue();
    zoom->updateZoomFitValue(r);

    auto layout = gtk_xournal_get_layout(zoom->view->getWidget());
    layout->layoutPages(allocation->width, allocation->height);
    gtk_widget_queue_resize(zoom->view->getWidget());

    return true;
}
