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

    auto addPt = [&](double nx, double ny) {
        Point p(sx + nx, sy + ny);
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
            // Drawn straight from start to end (ignoring rotation for waveforms usually, but we scale)
            double w = dx; double h = dy;
            if(w==0) w=1;
            addPt(0, 0); addPt(0, h); addPt(w/2, h); addPt(w/2, 0); addPt(w, 0); addPt(w, h);
            break;
        }
        case ELEC_RESISTOR_US: {
            addRotPt(0, 0); addRotPt(20, 0); addRotPt(25, -20); addRotPt(35, 20);
            addRotPt(45, -20); addRotPt(55, 20); addRotPt(65, -20); addRotPt(75, 20);
            addRotPt(80, 0); addRotPt(100, 0);
            break;
        }
        case ELEC_RESISTOR_EU: {
            // Draw a rectangle using strokes
            addRotPt(0, 0); addRotPt(20, 0); addRotPt(20, -10); addRotPt(80, -10); addRotPt(80, 10);
            addRotPt(20, 10); addRotPt(20, 0); addRotPt(80, 0); addRotPt(100, 0);
            break;
        }
        case ELEC_CAPACITOR_NP: {
            addRotPt(0, 0); addRotPt(40, 0); addRotPt(40, -30); addRotPt(40, 30);
            addRotPt(40, 0); // Backtrack
            // Need a stroke break natively, but we will fake it by backtracking
            addRotPt(60, 0); addRotPt(60, -30); addRotPt(60, 30); addRotPt(60, 0);
            addRotPt(100, 0);
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
        case ELEC_DIODE: {
            addRotPt(0, 0); addRotPt(40, 0); addRotPt(40, -20); addRotPt(60, 0); addRotPt(40, 20); addRotPt(40, 0);
            addRotPt(60, 0); addRotPt(60, -20); addRotPt(60, 20); addRotPt(60, 0); addRotPt(100, 0);
            break;
        }
        case ELEC_GATE_AND: {
            addRotPt(0, -20); addRotPt(20, -20); addRotPt(20, -40); addRotPt(20, 40); addRotPt(20, 20); addRotPt(0, 20); addRotPt(20, 20);
            // arc
            for(int i=0; i<=180; i+=10) addRotPt(20 + 40*std::sin(i*M_PI/180), -40*std::cos(i*M_PI/180));
            addRotPt(60, 0); addRotPt(100, 0);
            break;
        }
        case ELEC_GND_EARTH: {
            addRotPt(50, -50); addRotPt(50, 0); addRotPt(20, 0); addRotPt(80, 0);
            addRotPt(50, 0); addRotPt(50, 10); addRotPt(30, 10); addRotPt(70, 10);
            addRotPt(50, 10); addRotPt(50, 20); addRotPt(40, 20); addRotPt(60, 20);
            break;
        }
        default: { // Fallback standard line
            addPt(0,0);
            addPt(dx, dy);
            break;
        }
    }

    return {shape, range};
}
