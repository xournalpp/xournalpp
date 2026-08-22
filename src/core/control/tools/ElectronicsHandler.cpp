#include "ElectronicsHandler.h"

#include <cmath>
#include "control/Control.h"
#include "control/ToolHandler.h"
#include "control/Tool.h"
#include "model/Layer.h"
#include "model/Document.h"
#include "undo/InsertUndoAction.h"
#include "undo/GroupUndoAction.h"
#include "undo/UndoRedoHandler.h"
#include "gui/inputdevices/PositionInputData.h"
#include "model/XojPage.h"

ElectronicsHandler::ElectronicsHandler(Control* control, const PageRef& page)
    : BaseShapeHandler(control, page) {
}

ElectronicsHandler::~ElectronicsHandler() = default;

void ElectronicsHandler::onButtonReleaseEvent(const PositionInputData& pos, double zoom) {
    if (this->shape.size() <= 1) {
        BaseShapeHandler::onButtonReleaseEvent(pos, zoom);
        return;
    }

    Layer* layer = page->getSelectedLayer();
    if (!layer) {
        BaseShapeHandler::onButtonReleaseEvent(pos, zoom);
        return;
    }

    // Cancel the single-stroke preview from BaseShapeHandler
    std::vector<Point> finalShape = this->shape;

    // Call the base cancel explicitly to safely destroy the preview overlay and viewPool
    this->onSequenceCancelEvent();

    // Now, we regenerate the disjoint multi-strokes
    ElectronicsComponentType component = this->control->getToolHandler()->getActiveTool()->getElectronicsComponentType();

    double dx = currPoint.x - startPoint.x;
    double dy = currPoint.y - startPoint.y;
    double len = std::sqrt(dx * dx + dy * dy);
    if (len < 1.0) len = 1.0;
    double sx = startPoint.x;
    double sy = startPoint.y;
    double scale = len / 100.0;
    double theta = std::atan2(dy, dx);
    double cT = std::cos(theta);
    double sT = std::sin(theta);

    std::vector<std::vector<Point>> disjointShapes;
    std::vector<Point> currentShape;

    auto newShape = [&]() {
        if (!currentShape.empty()) {
            disjointShapes.push_back(currentShape);
            currentShape.clear();
        }
    };

    auto addPt = [&](double nx, double ny) {
        currentShape.push_back(Point(sx + nx, sy + ny, Point::NO_PRESSURE));
    };

    auto addRotPt = [&](double lx, double ly) {
        double nx = (lx * cT - ly * sT) * scale;
        double ny = (lx * sT + ly * cT) * scale;
        addPt(nx, ny);
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
                    currentShape.push_back(Point(sx + px, sy + py + dy/2.0, Point::NO_PRESSURE));
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
                if (p == 0) addPt(offset, 0);
                addPt(offset, amplitude);
                addPt(offset + periodWidth/2.0, amplitude);
                addPt(offset + periodWidth/2.0, 0);
                addPt(offset + periodWidth, 0);
                if (p == periods - 1) addPt(offset + periodWidth, amplitude);
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
                if (p == 0) addPt(offset, amplitude);
                else { newShape(); addPt(offset, amplitude); }
                addPt(offset + periodWidth/2.0, 0);
                addPt(offset + periodWidth/2.0, amplitude);
                addPt(offset + periodWidth, 0);
                if (p == periods - 1) addPt(offset + periodWidth, amplitude);
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
        case ELEC_SOURCE_DC: {
            addRotPt(0, 0); addRotPt(20, 0);
            newShape();
            for(int i=0; i<=360; i+=10) addRotPt(50 + 30*std::cos(i*M_PI/180), 30*std::sin(i*M_PI/180));
            newShape();
            addRotPt(80, 0); addRotPt(100, 0);
            newShape();
            addRotPt(35, -15); addRotPt(45, -15); // + horizontal
            newShape();
            addRotPt(40, -20); addRotPt(40, -10); // + vertical
            newShape();
            addRotPt(55, -15); addRotPt(65, -15); // - horizontal
            break;
        }
        case ELEC_SOURCE_AC: {
            addRotPt(0, 0); addRotPt(20, 0);
            newShape();
            for(int i=0; i<=360; i+=10) addRotPt(50 + 30*std::cos(i*M_PI/180), 30*std::sin(i*M_PI/180));
            newShape();
            addRotPt(80, 0); addRotPt(100, 0);
            newShape();
            // Sine wave inside
            for(int i=-180; i<=180; i+=20) addRotPt(50 + i*15.0/180.0, 10*std::sin(i*M_PI/180));
            break;
        }
        case ELEC_SOURCE_CURRENT: {
            addRotPt(0, 0); addRotPt(20, 0);
            newShape();
            for(int i=0; i<=360; i+=10) addRotPt(50 + 30*std::cos(i*M_PI/180), 30*std::sin(i*M_PI/180));
            newShape();
            addRotPt(80, 0); addRotPt(100, 0);
            newShape();
            addRotPt(35, 0); addRotPt(65, 0); // arrow line
            newShape();
            addRotPt(55, -5); addRotPt(65, 0); addRotPt(55, 5); // arrow tip
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
            addRotPt(0, 30); addRotPt(20, 30); addRotPt(20, 10); addRotPt(50, 10);
            for(int i=-90; i<=90; i+=10) addRotPt(50 + 40*std::cos(i*M_PI/180), 50 + 40*std::sin(i*M_PI/180));
            addRotPt(20, 90); addRotPt(20, 10);
            newShape(); addRotPt(20, 70); addRotPt(0, 70);
            newShape();
            if (component == ELEC_GATE_NAND) {
                addRotPt(90, 50); for(int i=0; i<=360; i+=30) addRotPt(90 + 5*std::cos(i*M_PI/180), 50 + 5*std::sin(i*M_PI/180));
                newShape(); addRotPt(95, 50); addRotPt(110, 50);
            } else { addRotPt(90, 50); addRotPt(110, 50); }
            break;
        }
        case ELEC_GATE_OR:
        case ELEC_GATE_NOR: {
            addRotPt(0, 30); addRotPt(25, 30);
            newShape(); addRotPt(0, 70); addRotPt(25, 70);
            newShape();
            for(int i=-90; i<=90; i+=15) addRotPt(10 + 20*std::cos(i*M_PI/180), 50 + 40*std::sin(i*M_PI/180));
            for(int i=90; i>=0; i-=10) addRotPt(10 + 90*std::cos(i*M_PI/180), 10 + 80*std::sin(i*M_PI/180));
            for(int i=0; i<=90; i+=10) addRotPt(10 + 90*std::cos(i*M_PI/180), 90 - 80*std::sin(i*M_PI/180));
            newShape();
            if (component == ELEC_GATE_NOR) {
                for(int i=0; i<=360; i+=30) addRotPt(100 + 5*std::cos(i*M_PI/180), 50 + 5*std::sin(i*M_PI/180));
                newShape(); addRotPt(105, 50); addRotPt(120, 50);
            } else { addRotPt(100, 50); addRotPt(120, 50); }
            break;
        }
        case ELEC_GATE_NOT: {
            addRotPt(0, 50); addRotPt(20, 50); addRotPt(20, 20); addRotPt(70, 50); addRotPt(20, 80); addRotPt(20, 50);
            newShape();
            for(int i=0; i<=360; i+=20) addRotPt(75 + 5*std::cos(i*M_PI/180), 50 + 5*std::sin(i*M_PI/180));
            newShape(); addRotPt(80, 50); addRotPt(100, 50);
            break;
        }
        case ELEC_GATE_XOR:
        case ELEC_GATE_XNOR: {
            addRotPt(0, 30); addRotPt(15, 30); addRotPt(15, 10);
            for(int i=-90; i<=90; i+=10) addRotPt(15 + 80*std::cos(i*M_PI/180), 50 + 80*std::sin(i*M_PI/180));
            addRotPt(15, 90); addRotPt(15, 70); addRotPt(0, 70);
            newShape();
            for(int i=-90; i<=90; i+=10) addRotPt(10 + 20*std::cos(i*M_PI/180), 50 + 40*std::sin(i*M_PI/180));
            newShape(); addRotPt(95, 50); addRotPt(105, 50);
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
        case ELEC_GND_EARTH: {
            addRotPt(50, 0); addRotPt(50, 40);
            newShape(); addRotPt(20, 40); addRotPt(80, 40);
            newShape(); addRotPt(30, 50); addRotPt(70, 50);
            newShape(); addRotPt(40, 60); addRotPt(60, 60);
            break;
        }
        case ELEC_GND_CHASSIS: {
            addRotPt(50, 0); addRotPt(50, 40);
            newShape(); addRotPt(20, 40); addRotPt(80, 40);
            newShape(); addRotPt(20, 40); addRotPt(10, 60);
            newShape(); addRotPt(50, 40); addRotPt(40, 60);
            newShape(); addRotPt(80, 40); addRotPt(70, 60);
            break;
        }
        case ELEC_GND_SIGNAL: {
            addRotPt(50, 0); addRotPt(50, 40);
            newShape(); addRotPt(20, 40); addRotPt(80, 40); addRotPt(50, 70); addRotPt(20, 40);
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
        case ELEC_SWITCH_SPST:
        case ELEC_SWITCH_SPDT:
        default: {
            addPt(0,0); addPt(dx, 0); addPt(dx, dy); addPt(0, dy); addPt(0, 0);
            break;
        }
    }
    newShape();

    Tool* activeTool = control->getToolHandler()->getActiveTool();
    if (!activeTool) return;

    std::vector<Element*> addedElements;
    for (const auto& shape_pts : disjointShapes) {
        if (shape_pts.empty()) continue;

        Stroke* stroke = new Stroke();
        stroke->setToolType(StrokeTool::PEN);
        stroke->setColor(activeTool->getColor());
        stroke->setWidth(control->getToolHandler()->getThickness());
        stroke->setLineStyle(activeTool->getLineStyle());
        stroke->setPointVector(shape_pts, nullptr);
        addedElements.push_back(stroke);
    }

    if (!addedElements.empty()) {
        auto groupAction = std::make_unique<GroupUndoAction>();
        Document* doc = control->getDocument();
        doc->lock();
        for (auto elem : addedElements) {
            auto elemPtr = std::unique_ptr<Element>(elem);
            auto undoAction = std::make_unique<InsertUndoAction>(this->page, layer, elem);
            groupAction->addAction(std::move(undoAction));
            layer->addElement(std::move(elemPtr));
        }
        doc->unlock();

        for (auto elem : addedElements) {
            page->fireElementChanged(elem);
        }

        control->getUndoRedoHandler()->addUndoAction(std::move(groupAction));
    }
}

std::pair<std::vector<Point>, Range> ElectronicsHandler::createShape(bool isAltDown, bool isShiftDown, bool isControlDown) {
    std::vector<Point> previewShape;
    Range range;

    ElectronicsComponentType component = this->control->getToolHandler()->getActiveTool()->getElectronicsComponentType();

    double dx = currPoint.x - startPoint.x;
    double dy = currPoint.y - startPoint.y;
    double len = std::sqrt(dx * dx + dy * dy);
    if (len < 1.0) len = 1.0;
    double sx = startPoint.x;
    double sy = startPoint.y;
    double scale = len / 100.0;
    double theta = std::atan2(dy, dx);
    double cT = std::cos(theta);
    double sT = std::sin(theta);

    auto addPt = [&](double nx, double ny) {
        Point p(sx + nx, sy + ny, Point::NO_PRESSURE);
        previewShape.push_back(p);
        if (previewShape.size() == 1) range = Range(p.x, p.y, p.x, p.y);
        else range = range.unite(Range(p.x, p.y, p.x, p.y));
    };

    auto addRotPt = [&](double lx, double ly) {
        double nx = (lx * cT - ly * sT) * scale;
        double ny = (lx * sT + ly * cT) * scale;
        addPt(nx, ny);
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
                    addPt(px, py + dy/2.0);
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
                if (p == 0) addPt(offset, 0);
                addPt(offset, amplitude);
                addPt(offset + periodWidth/2.0, amplitude);
                addPt(offset + periodWidth/2.0, 0);
                addPt(offset + periodWidth, 0);
                if (p == periods - 1) addPt(offset + periodWidth, amplitude);
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
                if (p == 0) addPt(offset, amplitude);
                else { addPt(offset, amplitude); }
                addPt(offset + periodWidth/2.0, 0);
                addPt(offset + periodWidth/2.0, amplitude);
                addPt(offset + periodWidth, 0);
                if (p == periods - 1) addPt(offset + periodWidth, amplitude);
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
            addRotPt(50, 40); addRotPt(50, -30); addRotPt(45, -20); addRotPt(55, -20); addRotPt(50, -30);
            break;
        }
        case ELEC_CAPACITOR_NP: {
            addRotPt(0, 0); addRotPt(45, 0); addRotPt(45, -30); addRotPt(45, 30);
            addRotPt(55, 0); addRotPt(55, -30); addRotPt(55, 30); addRotPt(55, 0);
            addRotPt(100, 0);
            break;
        }
        case ELEC_CAPACITOR_POL: {
            addRotPt(0, 0); addRotPt(45, 0); addRotPt(45, -30); addRotPt(45, 30);
            for(int i=-30; i<=30; i+=10) addRotPt(55 + 10*(1-std::cos(i*M_PI/60.0)), i);
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
            addRotPt(60, 0); addRotPt(60, -15); addRotPt(60, 15); addRotPt(60, 0);
            addRotPt(100, 0);
            break;
        }
        case ELEC_SOURCE_DC: {
            addRotPt(0, 0); addRotPt(20, 0);
            for(int i=0; i<=360; i+=10) addRotPt(50 + 30*std::cos(i*M_PI/180), 30*std::sin(i*M_PI/180));
            addRotPt(80, 0); addRotPt(100, 0);
            addRotPt(35, -15); addRotPt(45, -15);
            addRotPt(40, -20); addRotPt(40, -10);
            addRotPt(55, -15); addRotPt(65, -15);
            break;
        }
        case ELEC_SOURCE_AC: {
            addRotPt(0, 0); addRotPt(20, 0);
            for(int i=0; i<=360; i+=10) addRotPt(50 + 30*std::cos(i*M_PI/180), 30*std::sin(i*M_PI/180));
            addRotPt(80, 0); addRotPt(100, 0);
            for(int i=-180; i<=180; i+=20) addRotPt(50 + i*15.0/180.0, 10*std::sin(i*M_PI/180));
            break;
        }
        case ELEC_SOURCE_CURRENT: {
            addRotPt(0, 0); addRotPt(20, 0);
            for(int i=0; i<=360; i+=10) addRotPt(50 + 30*std::cos(i*M_PI/180), 30*std::sin(i*M_PI/180));
            addRotPt(80, 0); addRotPt(100, 0);
            addRotPt(35, 0); addRotPt(65, 0);
            addRotPt(55, -5); addRotPt(65, 0); addRotPt(55, 5);
            break;
        }
        case ELEC_DIODE:
        case ELEC_DIODE_LED:
        case ELEC_DIODE_ZENER:
        case ELEC_DIODE_SCHOTTKY: {
            addRotPt(0, 0); addRotPt(40, 0); addRotPt(40, -20); addRotPt(60, 0); addRotPt(40, 20); addRotPt(40, 0);
            addRotPt(60, -20); addRotPt(60, 20); addRotPt(60, 0); addRotPt(100, 0);
            break;
        }
        case ELEC_BJT_NPN:
        case ELEC_BJT_PNP: {
            addRotPt(0, 50); addRotPt(40, 50); addRotPt(40, 20); addRotPt(40, 80);
            addRotPt(40, 30); addRotPt(80, 0); addRotPt(100, 0);
            addRotPt(40, 70); addRotPt(80, 100); addRotPt(100, 100);
            break;
        }
        case ELEC_MOSFET_N:
        case ELEC_MOSFET_P: {
            addRotPt(0, 50); addRotPt(30, 50); addRotPt(30, 20); addRotPt(30, 80);
            addRotPt(40, 25); addRotPt(40, 45);
            addRotPt(40, 48); addRotPt(40, 68);
            addRotPt(40, 70); addRotPt(40, 90);
            addRotPt(40, 35); addRotPt(100, 35);
            addRotPt(40, 80); addRotPt(100, 80);
            break;
        }
        case ELEC_OPAMP: {
            addRotPt(0, 35); addRotPt(20, 35); addRotPt(20, 20); addRotPt(80, 50); addRotPt(20, 80); addRotPt(20, 65); addRotPt(0, 65);
            addRotPt(80, 50); addRotPt(100, 50);
            break;
        }
        case ELEC_GATE_AND:
        case ELEC_GATE_NAND: {
            addRotPt(0, 30); addRotPt(20, 30); addRotPt(20, 10); addRotPt(50, 10);
            for(int i=-90; i<=90; i+=10) addRotPt(50 + 40*std::cos(i*M_PI/180), 50 + 40*std::sin(i*M_PI/180));
            addRotPt(20, 90); addRotPt(20, 10);
            addRotPt(20, 70); addRotPt(0, 70);
            if (component == ELEC_GATE_NAND) {
                addRotPt(90, 50); for(int i=0; i<=360; i+=30) addRotPt(90 + 5*std::cos(i*M_PI/180), 50 + 5*std::sin(i*M_PI/180));
                addRotPt(95, 50); addRotPt(110, 50);
            } else { addRotPt(90, 50); addRotPt(110, 50); }
            break;
        }
        case ELEC_GATE_OR:
        case ELEC_GATE_NOR: {
            addRotPt(0, 30); addRotPt(25, 30);
            addRotPt(0, 70); addRotPt(25, 70);
            for(int i=-90; i<=90; i+=15) addRotPt(10 + 20*std::cos(i*M_PI/180), 50 + 40*std::sin(i*M_PI/180));
            for(int i=90; i>=0; i-=10) addRotPt(10 + 90*std::cos(i*M_PI/180), 10 + 80*std::sin(i*M_PI/180));
            for(int i=0; i<=90; i+=10) addRotPt(10 + 90*std::cos(i*M_PI/180), 90 - 80*std::sin(i*M_PI/180));
            if (component == ELEC_GATE_NOR) {
                for(int i=0; i<=360; i+=30) addRotPt(100 + 5*std::cos(i*M_PI/180), 50 + 5*std::sin(i*M_PI/180));
                addRotPt(105, 50); addRotPt(120, 50);
            } else { addRotPt(100, 50); addRotPt(120, 50); }
            break;
        }
        case ELEC_GATE_NOT: {
            addRotPt(0, 50); addRotPt(20, 50); addRotPt(20, 20); addRotPt(70, 50); addRotPt(20, 80); addRotPt(20, 50);
            for(int i=0; i<=360; i+=20) addRotPt(75 + 5*std::cos(i*M_PI/180), 50 + 5*std::sin(i*M_PI/180));
            addRotPt(80, 50); addRotPt(100, 50);
            break;
        }
        case ELEC_GATE_XOR:
        case ELEC_GATE_XNOR: {
            addRotPt(0, 30); addRotPt(15, 30); addRotPt(15, 10);
            for(int i=-90; i<=90; i+=10) addRotPt(15 + 80*std::cos(i*M_PI/180), 50 + 80*std::sin(i*M_PI/180));
            addRotPt(15, 90); addRotPt(15, 70); addRotPt(0, 70);
            for(int i=-90; i<=90; i+=10) addRotPt(10 + 20*std::cos(i*M_PI/180), 50 + 40*std::sin(i*M_PI/180));
            addRotPt(95, 50); addRotPt(105, 50);
            break;
        }
        case ELEC_FF_D:
        case ELEC_FF_JK: {
            addRotPt(20, 10); addRotPt(80, 10); addRotPt(80, 90); addRotPt(20, 90); addRotPt(20, 10);
            addRotPt(0, 30); addRotPt(20, 30);
            addRotPt(0, 70); addRotPt(20, 70); addRotPt(30, 75); addRotPt(20, 80);
            addRotPt(80, 30); addRotPt(100, 30);
            addRotPt(80, 70); addRotPt(100, 70);
            break;
        }
        case ELEC_KMAP_2X2: {
            addRotPt(0, 0); addRotPt(100, 0); addRotPt(100, 100); addRotPt(0, 100); addRotPt(0, 0);
            addRotPt(50, 0); addRotPt(50, 100);
            addRotPt(0, 50); addRotPt(100, 50);
            addRotPt(0, 0); addRotPt(-20, -20);
            break;
        }
        case ELEC_KMAP_2X4: {
            addRotPt(0, 0); addRotPt(200, 0); addRotPt(200, 100); addRotPt(0, 100); addRotPt(0, 0);
            addRotPt(50, 0); addRotPt(50, 100);
            addRotPt(100, 0); addRotPt(100, 100);
            addRotPt(150, 0); addRotPt(150, 100);
            addRotPt(0, 50); addRotPt(200, 50);
            addRotPt(0, 0); addRotPt(-20, -20);
            break;
        }
        case ELEC_KMAP_4X4: {
            addRotPt(0, 0); addRotPt(200, 0); addRotPt(200, 200); addRotPt(0, 200); addRotPt(0, 0);
            addRotPt(50, 0); addRotPt(50, 200);
            addRotPt(100, 0); addRotPt(100, 200);
            addRotPt(150, 0); addRotPt(150, 200);
            addRotPt(0, 50); addRotPt(200, 50);
            addRotPt(0, 100); addRotPt(200, 100);
            addRotPt(0, 150); addRotPt(200, 150);
            addRotPt(0, 0); addRotPt(-20, -20);
            break;
        }
        case ELEC_TIMING_GRID: {
            for(int i=0; i<=4; ++i) { addRotPt(0, i*25); addRotPt(100, i*25); }
            for(int i=0; i<=4; ++i) { addRotPt(i*25, 0); addRotPt(i*25, 100); }
            break;
        }
        case ELEC_TRUTH_TABLE: {
            addRotPt(0, 0); addRotPt(100, 0); addRotPt(100, 100); addRotPt(0, 100); addRotPt(0, 0);
            addRotPt(0, 20); addRotPt(100, 20);
            addRotPt(50, 0); addRotPt(50, 100);
            break;
        }
        case ELEC_REGISTER_8BIT: {
            addRotPt(0, 0); addRotPt(160, 0); addRotPt(160, 40); addRotPt(0, 40); addRotPt(0, 0);
            for(int i=1; i<8; ++i) { addRotPt(i*20, 0); addRotPt(i*20, 40); }
            break;
        }
        case ELEC_REGISTER_16BIT: {
            addRotPt(0, 0); addRotPt(320, 0); addRotPt(320, 40); addRotPt(0, 40); addRotPt(0, 0);
            for(int i=1; i<16; ++i) { addRotPt(i*20, 0); addRotPt(i*20, 40); }
            break;
        }
        case ELEC_GND_EARTH: {
            addRotPt(50, 0); addRotPt(50, 40);
            addRotPt(20, 40); addRotPt(80, 40);
            addRotPt(30, 50); addRotPt(70, 50);
            addRotPt(40, 60); addRotPt(60, 60);
            break;
        }
        case ELEC_GND_CHASSIS: {
            addRotPt(50, 0); addRotPt(50, 40);
            addRotPt(20, 40); addRotPt(80, 40);
            addRotPt(20, 40); addRotPt(10, 60);
            addRotPt(50, 40); addRotPt(40, 60);
            addRotPt(80, 40); addRotPt(70, 60);
            break;
        }
        case ELEC_GND_SIGNAL: {
            addRotPt(50, 0); addRotPt(50, 40);
            addRotPt(20, 40); addRotPt(80, 40); addRotPt(50, 70); addRotPt(20, 40);
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
        case ELEC_SWITCH_SPST:
        case ELEC_SWITCH_SPDT:
        default: {
            addPt(0,0); addPt(dx, 0); addPt(dx, dy); addPt(0, dy); addPt(0, 0);
            break;
        }
    }

    return {previewShape, range};
}
