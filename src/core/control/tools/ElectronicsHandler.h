#pragma once

#include "InputHandler.h"
#include "model/PageRef.h"
#include "model/Point.h"
#include "util/Range.h"
#include "view/overlays/OverlayView.h"
#include "view/Repaintable.h"
#include <vector>
#include <memory>
#include "model/Stroke.h"

class Control;

class ElectronicsHandler : public InputHandler {
public:
    ElectronicsHandler(Control* control, const PageRef& page);
    ~ElectronicsHandler() override;

    bool onMotionNotifyEvent(const PositionInputData& pos, double zoom) override;
    void onButtonReleaseEvent(const PositionInputData& pos, double zoom) override;
    void onButtonPressEvent(const PositionInputData& pos, double zoom) override;
    void onButtonDoublePressEvent(const PositionInputData& pos, double zoom) override;
    void onSequenceCancelEvent() override;
    bool onKeyPressEvent(const KeyEvent& event) override { return false; }
    bool onKeyReleaseEvent(const KeyEvent& event) override { return false; }

    // We can use the view pool pattern if we want, but Xournal++ provides a standard way to draw overlays.
    // For simplicity, we can just draw to an overlay view.
    std::unique_ptr<xoj::view::OverlayView> createView(xoj::view::Repaintable* parent) const override;

    const std::vector<std::vector<Point>>& getShapes() const { return shapes; }
    Color getPreviewColor() const { return previewColor; }
    Control* getControl() const { return control; }

private:
    void generateShapes();
    void cancelStroke();

    Point startPoint;
    Point currPoint;
    std::vector<std::vector<Point>> shapes;
    Color previewColor = Colors::black;
    Range lastRepaintRange;
};
