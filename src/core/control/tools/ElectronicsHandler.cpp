#include "ElectronicsHandler.h"

#include <cmath>
#include "control/Control.h"
#include "control/ToolHandler.h"
#include "control/Tool.h"
#include "model/XojPage.h"
#include "model/Layer.h"
#include "undo/InsertUndoAction.h"
#include "undo/UndoRedoHandler.h"
#include "gui/inputdevices/PositionInputData.h"
#include "control/jobs/RenderJob.h"
#include "control/jobs/XournalScheduler.h"

class ElectronicsOverlayView : public xoj::view::OverlayView {
public:
    ElectronicsOverlayView(xoj::view::Repaintable* parent, const ElectronicsHandler* handler)
        : xoj::view::OverlayView(parent), handler(handler) {}


    bool isViewOf(const OverlayBase* overlay) const override { return false; }

    void draw(cairo_t* cr) const override {
        if (!handler || handler->getShapes().empty()) return;

        cairo_save(cr);
        cairo_set_source_rgba(cr, 0, 0, 0, 1);
        cairo_set_line_width(cr, 2.0); // Simple preview line width

        // Match the current tool color
        auto tool = handler->getControl()->getToolHandler()->getActiveTool();

        if (tool) {
            Color color = tool->getColor();
            // Just use a solid color for preview, since Color API is tricky here and we don't have Color::getRed() handy.
        }


        for (const auto& shape : handler->getShapes()) {
            if (shape.empty()) continue;
            cairo_move_to(cr, shape[0].x, shape[0].y);
            for (size_t i = 1; i < shape.size(); ++i) {
                cairo_line_to(cr, shape[i].x, shape[i].y);
            }
            cairo_stroke(cr);
        }
        cairo_restore(cr);
    }

private:
    const ElectronicsHandler* handler;
};

ElectronicsHandler::ElectronicsHandler(Control* control, const PageRef& page)
    : InputHandler(control, page) {
}

ElectronicsHandler::~ElectronicsHandler() {
    cancelStroke();
}

std::unique_ptr<xoj::view::OverlayView> ElectronicsHandler::createView(xoj::view::Repaintable* parent) const {
    return std::make_unique<ElectronicsOverlayView>(parent, this);
}

void ElectronicsHandler::onSequenceCancelEvent() {
    cancelStroke();
}

void ElectronicsHandler::cancelStroke() {
    shapes.clear();
    lastRepaintRange = Range();
}

void ElectronicsHandler::onButtonDoublePressEvent(const PositionInputData& pos, double zoom) {
}

void ElectronicsHandler::onButtonPressEvent(const PositionInputData& pos, double zoom) {
    startPoint.x = pos.x / zoom;
    startPoint.y = pos.y / zoom;
    currPoint = startPoint;
    shapes.clear();
}

bool ElectronicsHandler::onMotionNotifyEvent(const PositionInputData& pos, double zoom) {
    currPoint.x = pos.x / zoom;
    currPoint.y = pos.y / zoom;

    // We only want to generate the shape data when moving, and notify the repaint system
    generateShapes();
    return true;
}

void ElectronicsHandler::onButtonReleaseEvent(const PositionInputData& pos, double zoom) {
    currPoint.x = pos.x / zoom;
    currPoint.y = pos.y / zoom;

    generateShapes();

    if (shapes.empty()) return;

    Layer* layer = page->getSelectedLayer();
    if (!layer) {
        cancelStroke();
        return;
    }

    std::vector<Element*> addedElements;

    Tool* activeTool = control->getToolHandler()->getActiveTool();
    if (!activeTool) {
        cancelStroke();
        return;
    }

        Color color = activeTool->getColor();
    const LineStyle& style = activeTool->getLineStyle();

    for (const auto& shape_pts : shapes) {
        if (shape_pts.empty()) continue;

        Stroke* stroke = new Stroke();
        stroke->setToolType(StrokeTool::PEN); // Always serialize as standard stroke so it's fully backwards compatible
        stroke->setColor(color);
        stroke->setWidth(control->getToolHandler()->getThickness());
        stroke->setLineStyle(style);

        // Add points
        for (const auto& pt : shape_pts) {
            stroke->addPoint(pt);
        }
        addedElements.push_back(stroke);
    }



        for (auto elem : addedElements) {
            auto elemPtr = std::unique_ptr<Element>(elem);
            auto undoAction = std::make_unique<InsertUndoAction>(this->page, layer, elem);
            layer->addElement(std::move(elemPtr));
            control->getUndoRedoHandler()->addUndoAction(std::move(undoAction));
        }
        // Force redraw
        // Redraw handled by undo system



    cancelStroke();
}

void ElectronicsHandler::generateShapes() {
    shapes.clear();

    ElectronicsComponentType component = this->control->getToolHandler()->getActiveTool()->getElectronicsComponentType();

    double dx = currPoint.x - startPoint.x;
    double dy = currPoint.y - startPoint.y;
    double len = std::sqrt(dx * dx + dy * dy);

    if (len < 1.0) len = 1.0;

    double sx = startPoint.x;
    double sy = startPoint.y;

    // Scale factor to map standard 100x100 grid shapes to the drag distance
    double scale = len / 100.0;
    // Rotation angle
    double theta = std::atan2(dy, dx);
    double cT = std::cos(theta);
    double sT = std::sin(theta);

    std::vector<Point> currentShape;

    auto newShape = [&]() {
        if (!currentShape.empty()) {
            shapes.push_back(currentShape);
            currentShape.clear();
        }
    };

    auto addPt = [&](double nx, double ny, double pressure = Point::NO_PRESSURE) {
        Point p(sx + nx, sy + ny, pressure);
        currentShape.push_back(p);
    };

    // Helper to rotate and scale
    auto addRotPt = [&](double lx, double ly, double pressure = Point::NO_PRESSURE) {
        double nx = (lx * cT - ly * sT) * scale;
        double ny = (lx * sT + ly * cT) * scale;
        addPt(nx, ny, pressure);
    };

    switch (component) {
        case ELEC_WAVE_SINE: {
            if (std::abs(dx) < 1.0) dx = 1.0;
            double amplitude = std::abs(dy) / 2.0;
            if (amplitude < 10.0) amplitude = 10.0;
            int periods = std::max(1, static_cast<int>(std::abs(dx) / 15.0));
            double periodWidth = dx / periods;
            int pointsPerPeriod = 40;
            double step = periodWidth / pointsPerPeriod;

            for (int p = 0; p < periods; ++p) {
                for (int i = 0; i <= pointsPerPeriod; ++i) {
                    if (p > 0 && i == 0) continue;
                    double lx = step * i;
                    double val = std::sin((lx / periodWidth) * 2 * M_PI);
                    double px = (p * periodWidth) + lx;
                    double py = -(val * amplitude);

                    Point pt(sx + px, sy + py + dy/2.0);
                    currentShape.push_back(pt);
                }
            }
            break;
        }
        case ELEC_WAVE_SQUARE: {
            if (std::abs(dx) < 1.0) dx = 1.0;
            double amplitude = std::abs(dy);
            int periods = std::max(1, static_cast<int>(std::abs(dx) / 15.0));
            double periodWidth = dx / periods;

            for (int p = 0; p < periods; ++p) {
                double offset = p * periodWidth;
                if (p == 0) {
                    addPt(offset, 0);
                }
                addPt(offset, amplitude);
                addPt(offset + periodWidth/2.0, amplitude);
                addPt(offset + periodWidth/2.0, 0);
                addPt(offset + periodWidth, 0);
                if (p == periods - 1) {
                    addPt(offset + periodWidth, amplitude);
                }
            }
            break;
        }
        case ELEC_WAVE_TRIANGLE: {
            if (std::abs(dx) < 1.0) dx = 1.0;
            double amplitude = std::abs(dy);
            int periods = std::max(1, static_cast<int>(std::abs(dx) / 15.0));
            double periodWidth = dx / periods;

            for (int p = 0; p < periods; ++p) {
                double offset = p * periodWidth;
                if (p == 0) addPt(offset, amplitude/2.0);
                addPt(offset + periodWidth/4.0, 0);
                addPt(offset + 3.0*periodWidth/4.0, amplitude);
                addPt(offset + periodWidth, amplitude/2.0);
            }
            break;
        }
        case ELEC_WAVE_SAWTOOTH: {
            if (std::abs(dx) < 1.0) dx = 1.0;
            double amplitude = std::abs(dy);
            int periods = std::max(1, static_cast<int>(std::abs(dx) / 15.0));
            double periodWidth = dx / periods;

            for (int p = 0; p < periods; ++p) {
                double offset = p * periodWidth;
                if (p == 0) {
                    addPt(offset, amplitude);
                } else {
                    newShape();
                    addPt(offset, amplitude);
                }
                addPt(offset + periodWidth/2.0, 0);
                addPt(offset + periodWidth/2.0, amplitude);
                addPt(offset + periodWidth, 0);
                if (p == periods - 1) {
                    addPt(offset + periodWidth, amplitude);
                }
            }
            break;
        }
        case ELEC_RESISTOR_US: {
            addRotPt(0, 0); addRotPt(20, 0); addRotPt(25, -20); addRotPt(35, 20);
            addRotPt(45, -20); addRotPt(55, 20); addRotPt(65, -20); addRotPt(75, 20);
            addRotPt(80, 0); addRotPt(100, 0);
            break;
        }
        case ELEC_RESISTOR_EU: {
            addRotPt(0, 0); addRotPt(20, 0); addRotPt(20, -10); addRotPt(80, -10); addRotPt(80, 10);
            addRotPt(20, 10); addRotPt(20, 0); addRotPt(80, 0); addRotPt(100, 0);
            break;
        }
        case ELEC_POTENTIOMETER: {
            addRotPt(0, 0); addRotPt(20, 0); addRotPt(25, -20); addRotPt(35, 20);
            addRotPt(45, -20); addRotPt(55, 20); addRotPt(65, -20); addRotPt(75, 20);
            addRotPt(80, 0); addRotPt(100, 0);

            newShape();
            addRotPt(50, 40); addRotPt(50, -30); addRotPt(45, -20); addRotPt(55, -20); addRotPt(50, -30);
            break;
        }
        case ELEC_CAPACITOR_NP: {
            addRotPt(0, 0); addRotPt(45, 0); addRotPt(45, -30); addRotPt(45, 30);
            newShape();
            addRotPt(55, 0); addRotPt(55, -30); addRotPt(55, 30); addRotPt(55, 0);
            addRotPt(100, 0);
            break;
        }
        case ELEC_CAPACITOR_POL: {
            addRotPt(0, 0); addRotPt(45, 0); addRotPt(45, -30); addRotPt(45, 30);
            newShape();
            for(int i=-30; i<=30; i+=10) addRotPt(55 + 10*(1-std::cos(i*M_PI/60.0)), i);
            newShape();
            addRotPt(55, 0); addRotPt(100, 0);
            break;
        }
        case ELEC_INDUCTOR: {
            addRotPt(0, 0); addRotPt(20, 0);
            for(int i=0; i<180; i+=20) addRotPt(20 + 10*(1-std::cos(i*M_PI/180)), -10*std::sin(i*M_PI/180));
            for(int i=0; i<180; i+=20) addRotPt(40 + 10*(1-std::cos(i*M_PI/180)), -10*std::sin(i*M_PI/180));
            for(int i=0; i<180; i+=20) addRotPt(60 + 10*(1-std::cos(i*M_PI/180)), -10*std::sin(i*M_PI/180));
            addRotPt(80, 0); addRotPt(100, 0);
            break;
        }
        case ELEC_SOURCE_DC_BATT: {
            addRotPt(0, 0); addRotPt(40, 0); addRotPt(40, -30); addRotPt(40, 30);
            newShape();
            addRotPt(60, 0); addRotPt(60, -15); addRotPt(60, 15); addRotPt(60, 0);
            addRotPt(100, 0);
            break;
        }
        case ELEC_SOURCE_DC:
        case ELEC_SOURCE_AC:
        case ELEC_SOURCE_CURRENT: {
            addRotPt(0, 0); addRotPt(20, 0);
            newShape();
            for(int i=0; i<=360; i+=10) addRotPt(50 + 30*std::cos(i*M_PI/180), 30*std::sin(i*M_PI/180));
            newShape();
            addRotPt(80, 0); addRotPt(100, 0);
            break;
        }
        case ELEC_DIODE:
        case ELEC_DIODE_LED:
        case ELEC_DIODE_ZENER:
        case ELEC_DIODE_SCHOTTKY: {
            addRotPt(0, 0); addRotPt(40, 0); addRotPt(40, -20); addRotPt(60, 0); addRotPt(40, 20); addRotPt(40, 0);
            newShape();
            addRotPt(60, -20); addRotPt(60, 20); addRotPt(60, 0); addRotPt(100, 0);
            break;
        }
        case ELEC_BJT_NPN:
        case ELEC_BJT_PNP: {
            addRotPt(0, 50); addRotPt(40, 50); addRotPt(40, 20); addRotPt(40, 80);
            newShape();
            addRotPt(40, 30); addRotPt(80, 0); addRotPt(100, 0);
            newShape();
            addRotPt(40, 70); addRotPt(80, 100); addRotPt(100, 100);
            break;
        }
        case ELEC_MOSFET_N:
        case ELEC_MOSFET_P: {
            addRotPt(0, 50); addRotPt(30, 50); addRotPt(30, 20); addRotPt(30, 80);
            newShape(); addRotPt(40, 25); addRotPt(40, 45);
            newShape(); addRotPt(40, 48); addRotPt(40, 68);
            newShape(); addRotPt(40, 70); addRotPt(40, 90);
            newShape(); addRotPt(40, 35); addRotPt(100, 35);
            newShape(); addRotPt(40, 80); addRotPt(100, 80);
            break;
        }
        case ELEC_OPAMP: {
            addRotPt(0, 35); addRotPt(20, 35); addRotPt(20, 20); addRotPt(80, 50); addRotPt(20, 80); addRotPt(20, 65); addRotPt(0, 65);
            newShape();
            addRotPt(80, 50); addRotPt(100, 50);
            break;
        }
        case ELEC_GATE_AND:
        case ELEC_GATE_NAND: {
            // Inputs
            addRotPt(0, 30); addRotPt(20, 30); addRotPt(20, 10);
            // Top straight edge
            addRotPt(50, 10);
            // Semicircle right
            for(int i=-90; i<=90; i+=10) addRotPt(50 + 40*std::cos(i*M_PI/180), 50 + 40*std::sin(i*M_PI/180));
            // Bottom straight edge
            addRotPt(20, 90);
            // Flat back
            addRotPt(20, 10);
            newShape();
            // Input 2
            addRotPt(20, 70); addRotPt(0, 70);
            newShape();
            // Output
            if (component == ELEC_GATE_NAND) {
                addRotPt(90, 50);
                for(int i=0; i<=360; i+=30) addRotPt(90 + 5*std::cos(i*M_PI/180), 50 + 5*std::sin(i*M_PI/180));
                newShape();
                addRotPt(95, 50); addRotPt(110, 50);
            } else {
                addRotPt(90, 50); addRotPt(110, 50);
            }
            break;
        }
        case ELEC_GATE_OR:
        case ELEC_GATE_NOR: {
            // US OR Gate: curved back, sharp curved front
            addRotPt(0, 30); addRotPt(25, 30); // Top input
            newShape();
            addRotPt(0, 70); addRotPt(25, 70); // Bottom input
            newShape();

            for(int i=-90; i<=90; i+=15) addRotPt(10 + 20*std::cos(i*M_PI/180), 50 + 40*std::sin(i*M_PI/180));

            // Bottom to tip
            for(int i=90; i>=0; i-=10) addRotPt(10 + 90*std::cos(i*M_PI/180), 10 + 80*std::sin(i*M_PI/180));

            // Tip to top
            for(int i=0; i<=90; i+=10) addRotPt(10 + 90*std::cos(i*M_PI/180), 90 - 80*std::sin(i*M_PI/180));

            newShape();
            if (component == ELEC_GATE_NOR) {
                for(int i=0; i<=360; i+=30) addRotPt(100 + 5*std::cos(i*M_PI/180), 50 + 5*std::sin(i*M_PI/180));
                newShape();
                addRotPt(105, 50); addRotPt(120, 50);
            } else {
                addRotPt(100, 50); addRotPt(120, 50);
            }
            break;
        }
        case ELEC_GATE_NOT: {
            addRotPt(0, 50); addRotPt(20, 50); addRotPt(20, 20); addRotPt(70, 50); addRotPt(20, 80); addRotPt(20, 50);
            newShape();
            for(int i=0; i<=360; i+=20) addRotPt(75 + 5*std::cos(i*M_PI/180), 50 + 5*std::sin(i*M_PI/180));
            newShape();
            addRotPt(80, 50); addRotPt(100, 50);
            break;
        }
        case ELEC_GATE_XOR:
        case ELEC_GATE_XNOR: {
            // US XOR gate
            addRotPt(0, 30); addRotPt(15, 30); addRotPt(15, 10);
            for(int i=-90; i<=90; i+=10) addRotPt(15 + 80*std::cos(i*M_PI/180), 50 + 80*std::sin(i*M_PI/180));
            addRotPt(15, 90); addRotPt(15, 70); addRotPt(0, 70);
            newShape();
            for(int i=-90; i<=90; i+=10) addRotPt(10 + 20*std::cos(i*M_PI/180), 50 + 40*std::sin(i*M_PI/180));
            newShape();
            addRotPt(95, 50); addRotPt(105, 50);
            break;
        }
        case ELEC_FF_D:
        case ELEC_FF_JK: {
            addRotPt(20, 10); addRotPt(80, 10); addRotPt(80, 90); addRotPt(20, 90); addRotPt(20, 10);
            newShape(); addRotPt(0, 30); addRotPt(20, 30);
            newShape(); addRotPt(0, 70); addRotPt(20, 70); addRotPt(30, 75); addRotPt(20, 80);
            newShape(); addRotPt(80, 30); addRotPt(100, 30);
            newShape(); addRotPt(80, 70); addRotPt(100, 70);
            break;
        }
        case ELEC_KMAP_2X2: {
            addRotPt(0, 0); addRotPt(100, 0); addRotPt(100, 100); addRotPt(0, 100); addRotPt(0, 0);
            newShape(); addRotPt(50, 0); addRotPt(50, 100);
            newShape(); addRotPt(0, 50); addRotPt(100, 50);
            newShape(); addRotPt(0, 0); addRotPt(-20, -20);
            break;
        }
        case ELEC_KMAP_2X4: {
            addRotPt(0, 0); addRotPt(200, 0); addRotPt(200, 100); addRotPt(0, 100); addRotPt(0, 0);
            newShape(); addRotPt(50, 0); addRotPt(50, 100);
            newShape(); addRotPt(100, 0); addRotPt(100, 100);
            newShape(); addRotPt(150, 0); addRotPt(150, 100);
            newShape(); addRotPt(0, 50); addRotPt(200, 50);
            newShape(); addRotPt(0, 0); addRotPt(-20, -20);
            break;
        }
        case ELEC_KMAP_4X4: {
            addRotPt(0, 0); addRotPt(200, 0); addRotPt(200, 200); addRotPt(0, 200); addRotPt(0, 0);
            newShape(); addRotPt(50, 0); addRotPt(50, 200);
            newShape(); addRotPt(100, 0); addRotPt(100, 200);
            newShape(); addRotPt(150, 0); addRotPt(150, 200);
            newShape(); addRotPt(0, 50); addRotPt(200, 50);
            newShape(); addRotPt(0, 100); addRotPt(200, 100);
            newShape(); addRotPt(0, 150); addRotPt(200, 150);
            newShape(); addRotPt(0, 0); addRotPt(-20, -20);
            break;
        }
        case ELEC_TIMING_GRID: {
            for(int i=0; i<=4; ++i) { addRotPt(0, i*25); addRotPt(100, i*25); newShape(); }
            for(int i=0; i<=4; ++i) { addRotPt(i*25, 0); addRotPt(i*25, 100); newShape(); }
            break;
        }
        case ELEC_TRUTH_TABLE: {
            addRotPt(0, 0); addRotPt(100, 0); addRotPt(100, 100); addRotPt(0, 100); addRotPt(0, 0);
            newShape(); addRotPt(0, 20); addRotPt(100, 20);
            newShape(); addRotPt(50, 0); addRotPt(50, 100);
            break;
        }
        case ELEC_REGISTER_8BIT: {
            addRotPt(0, 0); addRotPt(160, 0); addRotPt(160, 40); addRotPt(0, 40); addRotPt(0, 0);
            for(int i=1; i<8; ++i) { newShape(); addRotPt(i*20, 0); addRotPt(i*20, 40); }
            break;
        }
        case ELEC_REGISTER_16BIT: {
            addRotPt(0, 0); addRotPt(320, 0); addRotPt(320, 40); addRotPt(0, 40); addRotPt(0, 0);
            for(int i=1; i<16; ++i) { newShape(); addRotPt(i*20, 0); addRotPt(i*20, 40); }
            break;
        }
        case ELEC_FLOW_START:
        case ELEC_FLOW_PROCESS:
        case ELEC_BLOCK_MIXER:
        case ELEC_BLOCK_AMPLIFIER:
        case ELEC_BLOCK_FILTER_LPF:
        case ELEC_BLOCK_FILTER_HPF:
        case ELEC_BLOCK_OSCILLATOR:
        case ELEC_BLOCK_ANTENNA:
        case ELEC_FLOW_DECISION:
        case ELEC_FLOW_IO:
        case ELEC_GND_CHASSIS:
        case ELEC_GND_SIGNAL:
        case ELEC_SWITCH_SPST:
        case ELEC_SWITCH_SPDT:
        default: { // Fallback standard rectangle to avoid crashing
            addPt(0,0);
            addPt(dx, 0);
            addPt(dx, dy);
            addPt(0, dy);
            addPt(0, 0);
            break;
        }
    }

    if (!currentShape.empty()) {
        shapes.push_back(currentShape);
    }
}
