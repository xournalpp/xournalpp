#include "ElectronicsHandler.h"

#include <cmath>
#include "control/Control.h"
#include "control/ToolHandler.h"
#include "control/Tool.h"

ElectronicsHandler::ElectronicsHandler(Control* control, const PageRef& page)
    : BaseShapeHandler(control, page) {
}

ElectronicsHandler::~ElectronicsHandler() = default;

std::pair<std::vector<Point>, Range> ElectronicsHandler::createShape(bool isAltDown, bool isShiftDown, bool isControlDown) {
    std::vector<Point> shape;
    Range range;

    ElectronicsComponentType component = this->control->getToolHandler()->getActiveTool()->getElectronicsComponentType();

    double dx = currPoint.x - startPoint.x;
    double dy = currPoint.y - startPoint.y;
    double len = std::sqrt(dx * dx + dy * dy);

    if (len < 1.0) len = 1.0;

    double sx = startPoint.x;
    double sy = startPoint.y;

    auto addPt = [&](double nx, double ny, double pressure = Point::NO_PRESSURE) {
        Point p(sx + nx, sy + ny, pressure);
        shape.push_back(p);
        if (shape.size() == 1) range = Range(p.x, p.y, p.x, p.y);
        else range = range.unite(Range(p.x, p.y, p.x, p.y));
    };

    // Scale factor to map standard 100x100 grid shapes to the drag distance
    double scale = len / 100.0;
    // Rotation angle
    double theta = std::atan2(dy, dx);
    double cT = std::cos(theta);
    double sT = std::sin(theta);

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
            int periods = std::max(1, static_cast<int>(std::abs(dx) / 100.0));
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

                    Point pt(sx + px, sy + py);
                    shape.push_back(pt);
                    if (p == 0 && i == 0) range = Range(pt.x, pt.y, pt.x, pt.y);
                    else range = range.unite(Range(pt.x, pt.y, pt.x, pt.y));
                }
            }
            break;
        }
        case ELEC_WAVE_SQUARE: {
            double w = dx; double h = dy;
            if(w==0) w=1;
            addPt(0, 0); addPt(0, h); addPt(w/2, h); addPt(w/2, 0); addPt(w, 0); addPt(w, h);
            break;
        }
        case ELEC_WAVE_TRIANGLE: {
            double w = dx; double h = dy;
            if(w==0) w=1;
            addPt(0, h/2); addPt(w/4, 0); addPt(3*w/4, h); addPt(w, h/2);
            break;
        }
        case ELEC_WAVE_SAWTOOTH: {
            double w = dx; double h = dy;
            if(w==0) w=1;
            addPt(0, h); addPt(w/2, 0); addPt(w/2, h); addPt(w, 0); addPt(w, h);
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
            // Arrow (Backtracking stroke)
            addRotPt(100, 0, 0); // 0 pressure line back
            addRotPt(50, 40, 0);
            addRotPt(50, 40); addRotPt(50, -30); addRotPt(45, -20); addRotPt(55, -20); addRotPt(50, -30);
            break;
        }
        case ELEC_CAPACITOR_NP: {
            addRotPt(0, 0); addRotPt(45, 0); addRotPt(45, -30); addRotPt(45, 30);
            // backtrack with 0 pressure
            addRotPt(45, 0, 0); addRotPt(55, 0, 0);
            addRotPt(55, 0); addRotPt(55, -30); addRotPt(55, 30); addRotPt(55, 0);
            addRotPt(100, 0);
            break;
        }
        case ELEC_CAPACITOR_POL: {
            addRotPt(0, 0); addRotPt(45, 0); addRotPt(45, -30); addRotPt(45, 30);
            addRotPt(45, 0, 0); addRotPt(55, 0, 0);
            addRotPt(55, 0);
            for(int i=-30; i<=30; i+=10) addRotPt(55 + 10*(1-std::cos(i*M_PI/60.0)), i);
            addRotPt(55, 0, 0); addRotPt(100, 0, 0); addRotPt(100, 0);
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
            addRotPt(40, 0, 0); addRotPt(60, 0, 0);
            addRotPt(60, 0); addRotPt(60, -15); addRotPt(60, 15); addRotPt(60, 0);
            addRotPt(100, 0);
            break;
        }
        case ELEC_SOURCE_DC:
        case ELEC_SOURCE_AC:
        case ELEC_SOURCE_CURRENT: {
            addRotPt(0, 0); addRotPt(20, 0);
            for(int i=0; i<=360; i+=10) addRotPt(50 + 30*std::cos(i*M_PI/180), 30*std::sin(i*M_PI/180));
            addRotPt(80, 0, 0); addRotPt(100, 0, 0); addRotPt(100, 0);
            break;
        }
        case ELEC_DIODE:
        case ELEC_DIODE_LED:
        case ELEC_DIODE_ZENER:
        case ELEC_DIODE_SCHOTTKY: {
            addRotPt(0, 0); addRotPt(40, 0); addRotPt(40, -20); addRotPt(60, 0); addRotPt(40, 20); addRotPt(40, 0);
            addRotPt(60, 0, 0); addRotPt(60, -20); addRotPt(60, 20); addRotPt(60, 0); addRotPt(100, 0);
            break;
        }
        case ELEC_BJT_NPN:
        case ELEC_BJT_PNP: {
            addRotPt(0, 50); addRotPt(40, 50); addRotPt(40, 20); addRotPt(40, 80);
            addRotPt(40, 30, 0); addRotPt(80, 0); addRotPt(100, 0);
            addRotPt(80, 0, 0); addRotPt(40, 70, 0); addRotPt(80, 100); addRotPt(100, 100);
            break;
        }
        case ELEC_MOSFET_N:
        case ELEC_MOSFET_P: {
            addRotPt(0, 50); addRotPt(30, 50); addRotPt(30, 20); addRotPt(30, 80);
            addRotPt(40, 25, 0); addRotPt(40, 45);
            addRotPt(40, 48, 0); addRotPt(40, 68);
            addRotPt(40, 70, 0); addRotPt(40, 90);
            addRotPt(40, 35, 0); addRotPt(100, 35);
            addRotPt(40, 80, 0); addRotPt(100, 80);
            break;
        }
        case ELEC_OPAMP: {
            addRotPt(0, 35); addRotPt(20, 35); addRotPt(20, 20); addRotPt(80, 50); addRotPt(20, 80); addRotPt(20, 65); addRotPt(0, 65);
            addRotPt(80, 50, 0); addRotPt(100, 50);
            break;
        }
        case ELEC_GATE_AND:
        case ELEC_GATE_NAND: {
            addRotPt(0, 30); addRotPt(20, 30); addRotPt(20, 10); addRotPt(50, 10);
            for(int i=-90; i<=90; i+=10) addRotPt(50 + 40*std::cos(i*M_PI/180), 50 + 40*std::sin(i*M_PI/180));
            addRotPt(50, 90); addRotPt(20, 90); addRotPt(20, 70); addRotPt(0, 70);
            addRotPt(90, 50, 0); addRotPt(100, 50);
            break;
        }
        case ELEC_GATE_OR:
        case ELEC_GATE_NOR: {
            addRotPt(0, 30); addRotPt(20, 30); addRotPt(20, 10);
            for(int i=-90; i<=0; i+=10) addRotPt(20 + 80*std::cos(i*M_PI/180), 50 + 80*std::sin(i*M_PI/180));
            for(int i=0; i<=90; i+=10) addRotPt(20 + 80*std::cos(i*M_PI/180), 50 + 80*std::sin(i*M_PI/180));
            addRotPt(20, 90); addRotPt(20, 70); addRotPt(0, 70);
            addRotPt(100, 50, 0); addRotPt(110, 50);
            break;
        }
        case ELEC_GATE_NOT: {
            addRotPt(0, 50); addRotPt(20, 50); addRotPt(20, 20); addRotPt(70, 50); addRotPt(20, 80); addRotPt(20, 50);
            addRotPt(75, 50, 0);
            for(int i=0; i<=360; i+=20) addRotPt(75 + 5*std::cos(i*M_PI/180), 50 + 5*std::sin(i*M_PI/180));
            addRotPt(80, 50, 0); addRotPt(100, 50);
            break;
        }
        case ELEC_GATE_XOR:
        case ELEC_GATE_XNOR: {
            addRotPt(0, 30); addRotPt(15, 30); addRotPt(15, 10);
            for(int i=-90; i<=90; i+=10) addRotPt(15 + 80*std::cos(i*M_PI/180), 50 + 80*std::sin(i*M_PI/180));
            addRotPt(15, 90); addRotPt(15, 70); addRotPt(0, 70);
            addRotPt(10, 10, 0);
            for(int i=-90; i<=90; i+=10) addRotPt(10 + 20*std::cos(i*M_PI/180), 50 + 40*std::sin(i*M_PI/180));
            addRotPt(95, 50, 0); addRotPt(105, 50);
            break;
        }
        case ELEC_FF_D:
        case ELEC_FF_JK: {
            addRotPt(20, 10); addRotPt(80, 10); addRotPt(80, 90); addRotPt(20, 90); addRotPt(20, 10);
            addRotPt(0, 30, 0); addRotPt(20, 30);
            addRotPt(0, 70, 0); addRotPt(20, 70); addRotPt(30, 75); addRotPt(20, 80);
            addRotPt(80, 30, 0); addRotPt(100, 30);
            addRotPt(80, 70, 0); addRotPt(100, 70);
            break;
        }
        case ELEC_KMAP_2X2: {
            addRotPt(0, 0); addRotPt(100, 0); addRotPt(100, 100); addRotPt(0, 100); addRotPt(0, 0);
            addRotPt(50, 0, 0); addRotPt(50, 100);
            addRotPt(0, 50, 0); addRotPt(100, 50);
            addRotPt(0, 0, 0); addRotPt(-20, -20);
            break;
        }
        case ELEC_KMAP_2X4: {
            addRotPt(0, 0); addRotPt(200, 0); addRotPt(200, 100); addRotPt(0, 100); addRotPt(0, 0);
            addRotPt(50, 0, 0); addRotPt(50, 100);
            addRotPt(100, 0, 0); addRotPt(100, 100);
            addRotPt(150, 0, 0); addRotPt(150, 100);
            addRotPt(0, 50, 0); addRotPt(200, 50);
            addRotPt(0, 0, 0); addRotPt(-20, -20);
            break;
        }
        case ELEC_KMAP_4X4: {
            addRotPt(0, 0); addRotPt(200, 0); addRotPt(200, 200); addRotPt(0, 200); addRotPt(0, 0);
            addRotPt(50, 0, 0); addRotPt(50, 200);
            addRotPt(100, 0, 0); addRotPt(100, 200);
            addRotPt(150, 0, 0); addRotPt(150, 200);
            addRotPt(0, 50, 0); addRotPt(200, 50);
            addRotPt(0, 100, 0); addRotPt(200, 100);
            addRotPt(0, 150, 0); addRotPt(200, 150);
            addRotPt(0, 0, 0); addRotPt(-20, -20);
            break;
        }
        case ELEC_TIMING_GRID: {
            for(int i=0; i<=4; ++i) { addRotPt(0, i*25, 0); addRotPt(100, i*25); }
            for(int i=0; i<=4; ++i) { addRotPt(i*25, 0, 0); addRotPt(i*25, 100); }
            break;
        }
        case ELEC_TRUTH_TABLE: {
            addRotPt(0, 0); addRotPt(100, 0); addRotPt(100, 100); addRotPt(0, 100); addRotPt(0, 0);
            addRotPt(0, 20, 0); addRotPt(100, 20);
            addRotPt(50, 0, 0); addRotPt(50, 100);
            break;
        }
        case ELEC_REGISTER_8BIT: {
            addRotPt(0, 0); addRotPt(160, 0); addRotPt(160, 40); addRotPt(0, 40); addRotPt(0, 0);
            for(int i=1; i<8; ++i) { addRotPt(i*20, 0, 0); addRotPt(i*20, 40); }
            break;
        }
        case ELEC_REGISTER_16BIT: {
            addRotPt(0, 0); addRotPt(320, 0); addRotPt(320, 40); addRotPt(0, 40); addRotPt(0, 0);
            for(int i=1; i<16; ++i) { addRotPt(i*20, 0, 0); addRotPt(i*20, 40); }
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

    return {shape, range};
}
