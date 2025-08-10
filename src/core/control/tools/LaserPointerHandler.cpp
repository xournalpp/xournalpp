#include "LaserPointerHandler.h"

#include <control/Control.h>
#include <control/settings/Settings.h>
#include <gui/MainWindow.h>
#include <gui/PageView.h>
#include <gui/XournalView.h>

#include "gui/inputdevices/PositionInputData.h"  // for PositionInputData
#include "model/Stroke.h"
#include "util/DispatchPool.h"
#include "util/glib_casts.h"
#include "view/overlays/LaserPointerView.h"

#include "StrokeHandler.h"

static constexpr int FADEOUT_STEP_DURATION = 50;  ///< in ms
static constexpr uint8_t FADEOUT_ALPHA_STEP = 25;
// The total fadeout duration will be  FADEOUT_STEP_DURATION * 255 / FADEOUT_ALPHA_STEP

class TemporaryStrokeHandler: public StrokeHandler {
public:
    using StrokeHandler::finalizeStroke;
    using StrokeHandler::StrokeHandler;
};

LaserPointerHandler::LaserPointerHandler(XojPageView* pageView, Control* control, const PageRef& page):
        viewPool(std::make_shared<xoj::util::DispatchPool<xoj::view::LaserPointerView>>()),
        ctrl(control),
        page(page),
        pageView(pageView),
        fadeoutStartDelay(control->getSettings()->getLaserPointerFadeOutTime()) {}

LaserPointerHandler::~LaserPointerHandler() = default;

std::unique_ptr<xoj::view::OverlayView> LaserPointerHandler::createView(xoj::view::Repaintable* parent) const {
    auto view = std::make_unique<xoj::view::LaserPointerView>(this, parent);
    if (this->strokehandler) {
        view->on(xoj::view::LaserPointerView::START_NEW_STROKE_REQUEST, this->strokehandler.get());
    }
    return view;
}

void LaserPointerHandler::onButtonPressEvent(const PositionInputData& pos, double zoom) {
    this->strokehandler = std::make_unique<TemporaryStrokeHandler>(ctrl, page);
    this->strokehandler->onButtonPressEvent(pos, zoom);
    this->viewPool->dispatch(xoj::view::LaserPointerView::START_NEW_STROKE_REQUEST, strokehandler.get());

    this->fadeoutTimer.cancel();
    this->fadeoutAlpha = 255;
}

void LaserPointerHandler::onButtonReleaseEvent(const PositionInputData& pos, double zoom) {
    if (!strokehandler) {
        return;  // This could happen if the tool changed between button press and release
    }
    this->strokehandler->finalizeStroke(pos.pressure);
    this->viewPool->dispatch(xoj::view::LaserPointerView::FINISH_STROKE_REQUEST,
                             Range(this->strokehandler->getStroke()->boundingRect()));
    this->strokehandler.reset();
    this->fadeoutTimer =
            g_timeout_add(this->fadeoutStartDelay, xoj::util::wrap_for_once_v<triggerFadeoutCallback>, this);
}

bool LaserPointerHandler::onMotionNotifyEvent(const PositionInputData& pos, double zoom) {
    return this->strokehandler && this->strokehandler->onMotionNotifyEvent(pos, zoom);
}

void LaserPointerHandler::onSequenceCancelEvent() {
    auto s = std::move(this->strokehandler);
    if (s && s->getStroke()) {
        Range rg(s->getStroke()->boundingRect());
        this->viewPool->dispatch(xoj::view::LaserPointerView::INPUT_CANCELLATION_REQUEST, rg);
    }
    this->fadeoutTimer =
            g_timeout_add(this->fadeoutStartDelay, xoj::util::wrap_for_once_v<triggerFadeoutCallback>, this);
}

void LaserPointerHandler::triggerFadeoutCallback(LaserPointerHandler* self) {
    self->fadeoutTimer.consume();
    self->fadeoutTimer = g_timeout_add(FADEOUT_STEP_DURATION, xoj::util::wrap_v<fadeoutCallback>, self);
}

gboolean LaserPointerHandler::fadeoutCallback(LaserPointerHandler* self) {
    if (self->fadeoutAlpha <= FADEOUT_ALPHA_STEP) {
        self->fadeoutAlpha = 0;
        self->viewPool->dispatchAndClear(xoj::view::LaserPointerView::FINALIZATION_REQUEST);
        self->fadeoutTimer.consume();
        self->pageView->deleteLaserPointerHandler();  // WARNING: deletes *self
        return G_SOURCE_REMOVE;
    }
    self->fadeoutAlpha -= FADEOUT_ALPHA_STEP;
    self->viewPool->dispatch(xoj::view::LaserPointerView::SET_ALPHA_REQUEST, self->fadeoutAlpha);
    return G_SOURCE_CONTINUE;  // The timer will run again and call this callback once more (or be cancelled)
}
