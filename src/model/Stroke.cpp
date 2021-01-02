#include "Stroke.h"

#include <cmath>
#include <numeric>

// TODO Remuve after use
#include <locale>

#include "model/Spline.h"
#include "serializing/ObjectInputStream.h"
#include "serializing/ObjectOutputStream.h"

#include "i18n.h"

Stroke::Stroke(): AudioElement(ELEMENT_STROKE) {}

Stroke::~Stroke() = default;

/**
 * Clone style attributes, but not the data (position, width etc.)
 */
void Stroke::applyStyleFrom(const Stroke* other) {
    setColor(other->getColor());
    setToolType(other->getToolType());
    setWidth(other->getWidth());
    setFill(other->getFill());
    setLineStyle(other->getLineStyle());

    cloneAudioData(other);
}

auto Stroke::cloneStroke() const -> Stroke* {
    auto* s = new Stroke();
    s->applyStyleFrom(this);
    s->points = this->points;
    s->x = this->x;
    s->y = this->y;
    s->width = this->width;  // stroke width, not bounding box width
    s->Element::width = this->Element::width;
    s->Element::height = this->Element::height;
    s->snappedBounds = this->snappedBounds;
    s->sizeCalculated = this->sizeCalculated;
    return s;
}

auto Stroke::clone() -> Element* { return this->cloneStroke(); }

void Stroke::serialize(ObjectOutputStream& out) {
    out.writeObject("Stroke");

    serializeAudioElement(out);

    out.writeDouble(this->width);

    out.writeInt(this->toolType);

    out.writeInt(fill);

    out.writeData(this->points.data(), this->points.size(), sizeof(Point));

    this->lineStyle.serialize(out);

    out.endObject();
}

void Stroke::readSerialized(ObjectInputStream& in) {
    in.readObject("Stroke");

    readSerializedAudioElement(in);

    this->width = in.readDouble();

    this->toolType = static_cast<StrokeTool>(in.readInt());

    this->fill = in.readInt();

    Point* p{};
    int count{};
    in.readData(reinterpret_cast<void**>(&p), &count);
    this->points = std::vector<Point>{p, p + count};
    g_free(p);
    this->lineStyle.readSerialized(in);

    in.endObject();
}

/**
 * Option to fill the shape:
 *  -1: The shape is not filled
 * 255: The shape is fully opaque filled
 * ...
 *   1: The shape is nearly fully transparent filled
 */
auto Stroke::getFill() const -> int { return fill; }

/**
 * Option to fill the shape:
 *  -1: The shape is not filled
 * 255: The shape is fully opaque filled
 * ...
 *   1: The shape is nearly fully transparent filled
 */
void Stroke::setFill(int fill) { this->fill = fill; }

void Stroke::setWidth(double width) { this->width = width; }

auto Stroke::getWidth() const -> double { return this->width; }

auto Stroke::rescaleWithMirror() -> bool { return true; }

auto Stroke::isInSelection(ShapeContainer* container) -> bool {
    for (auto&& p: this->points) {
        double px = p.x;
        double py = p.y;

        if (!container->contains(px, py)) {
            return false;
        }
    }

    return true;
}

void Stroke::setFirstPoint(double x, double y) {
    if (!this->points.empty()) {
        Point& p = this->points.front();
        p.x = x;
        p.y = y;
        this->sizeCalculated = false;
    }
}

void Stroke::setLastPoint(double x, double y) { setLastPoint({x, y}); }

void Stroke::setLastPoint(const Point& p) {
    if (!this->points.empty()) {
        this->points.back() = p;
        this->sizeCalculated = false;
    }
}

void Stroke::addPoint(const Point& p) {
    this->points.emplace_back(p);
    this->sizeCalculated = false;
}

auto Stroke::getPointCount() const -> int { return this->points.size(); }

auto Stroke::getPointVector() const -> std::vector<Point> const& { return points; }

void Stroke::deletePointsFrom(int index) { points.resize(std::min(size_t(index), points.size())); }

void Stroke::deletePoint(int index) { this->points.erase(std::next(begin(this->points), index)); }

auto Stroke::getPoint(int index) const -> Point {
    if (index < 0 || index >= this->points.size()) {
        g_warning("Stroke::getPoint(%i) out of bounds!", index);
        return Point(0, 0, Point::NO_PRESSURE);
    }
    return points.at(index);
}

auto Stroke::getPoints() const -> const Point* { return this->points.data(); }

void Stroke::freeUnusedPointItems() { this->points = {begin(this->points), end(this->points)}; }

void Stroke::setToolType(StrokeTool type) { this->toolType = type; }

auto Stroke::getToolType() const -> StrokeTool { return this->toolType; }

void Stroke::setLineStyle(const LineStyle& style) { this->lineStyle = style; }

auto Stroke::getLineStyle() const -> const LineStyle& { return this->lineStyle; }

void Stroke::move(double dx, double dy) {
    for (auto&& point: points) {
        point.x += dx;
        point.y += dy;
    }

    this->sizeCalculated = false;
}

void Stroke::rotate(double x0, double y0, double th) {
    cairo_matrix_t rotMatrix;
    cairo_matrix_init_identity(&rotMatrix);
    cairo_matrix_translate(&rotMatrix, x0, y0);
    cairo_matrix_rotate(&rotMatrix, th);
    cairo_matrix_translate(&rotMatrix, -x0, -y0);

    for (auto&& p: points) {
        cairo_matrix_transform_point(&rotMatrix, &p.x, &p.y);
    }
    // Width and Height will likely be changed after this operation
    calcSize();
}

void Stroke::scale(double x0, double y0, double fx, double fy, double rotation, bool restoreLineWidth) {
    double fz = (restoreLineWidth) ? 1 : sqrt(std::abs(fx * fy));
    cairo_matrix_t scaleMatrix;
    cairo_matrix_init_identity(&scaleMatrix);
    cairo_matrix_translate(&scaleMatrix, x0, y0);
    cairo_matrix_rotate(&scaleMatrix, rotation);
    cairo_matrix_scale(&scaleMatrix, fx, fy);
    cairo_matrix_rotate(&scaleMatrix, -rotation);
    cairo_matrix_translate(&scaleMatrix, -x0, -y0);

    for (auto&& p: points) {
        cairo_matrix_transform_point(&scaleMatrix, &p.x, &p.y);

        if (p.z != Point::NO_PRESSURE) {
            p.z *= fz;
        }
    }
    this->width *= fz;

    this->sizeCalculated = false;
}

auto Stroke::hasPressure() const -> bool {
    if (!this->points.empty()) {
        return this->points[0].z != Point::NO_PRESSURE;
    }
    return false;
}

auto Stroke::getAvgPressure() const -> double {
    return std::accumulate(begin(this->points), end(this->points), 0.0,
                           [](double l, Point const& p) { return l + p.z; }) /
           this->points.size();
}

void Stroke::scalePressure(double factor) {
    if (!hasPressure()) {
        return;
    }
    for (auto&& p: this->points) {
        p.z *= factor;
    }
}

void Stroke::clearPressure() {
    for (auto&& p: points) {
        p.z = Point::NO_PRESSURE;
    }
}

void Stroke::setLastPressure(double pressure) {
    if (!this->points.empty()) {
        this->points.back().z = pressure;
    }
}

void Stroke::setSecondToLastPressure(double pressure) {
    auto const pointCount = this->getPointCount();
    if (pointCount >= 2) {
        this->points[pointCount - 2].z = pressure;
    }
}

void Stroke::setPressure(const vector<double>& pressure) {
    // The last pressure is not used - as there is no line drawn from this point
    if (this->points.size() - 1 != pressure.size()) {
        g_warning("invalid pressure point count: %s, expected %s", std::to_string(pressure.size()).data(),
                  std::to_string(this->points.size() - 1).data());
    }

    auto max_size = std::min(pressure.size(), this->points.size() - 1);
    for (size_t i = 0U; i != max_size; ++i) {
        this->points[i].z = pressure[i];
    }
}

/**
 * checks if the stroke is intersected by the eraser rectangle
 */
auto Stroke::intersects(double x, double y, double halfEraserSize) -> bool {
    return intersects(x, y, halfEraserSize, nullptr);
}

/**
 * checks if the stroke is intersected by the eraser rectangle
 */
auto Stroke::intersects(double x, double y, double halfEraserSize, double* gap) -> bool {
    if (this->points.empty()) {
        return false;
    }

    double x1 = x - halfEraserSize;
    double x2 = x + halfEraserSize;
    double y1 = y - halfEraserSize;
    double y2 = y + halfEraserSize;

    double lastX = points[0].x;
    double lastY = points[0].y;
    for (auto&& point: points) {
        double px = point.x;
        double py = point.y;

        if (px >= x1 && py >= y1 && px <= x2 && py <= y2) {
            if (gap) {
                *gap = 0;
            }
            return true;
        }

        double len = hypot(px - lastX, py - lastY);
        if (len >= halfEraserSize) {
            /**
             * The distance of the center of the eraser box to the line passing through (lastx, lasty) and (px, py)
             */
            double p = std::abs((x - lastX) * (lastY - py) + (y - lastY) * (px - lastX)) / len;

            // If the distance p of the center of the eraser box to the (full) line is in the range,
            // we check whether the eraser box is not too far from the line segment through the two points.

            if (p <= halfEraserSize) {
                double centerX = (lastX + px) / 2;
                double centerY = (lastY + py) / 2;
                double distance = hypot(x - centerX, y - centerY);

                // For the above check we imagine a circle whose center is the mid point of the two points of the stroke
                // and whose radius is half the length of the line segment plus half the diameter of the eraser box
                // plus some small padding
                // If the center of the eraser box lies within that circle then we consider it to be close enough

                distance -= halfEraserSize * std::sqrt(2);

                constexpr double PADDING = 0.1;

                if (distance <= len / 2 + PADDING) {
                    if (gap) {
                        *gap = distance;
                    }
                    return true;
                }
            }
        }

        lastX = px;
        lastY = py;
    }

    return false;
}

/**
 * Updates the size
 * The size is needed to only redraw the requested part instead of redrawing
 * the whole page (performance reason).
 * Also used for Selected Bounding box.
 */
void Stroke::calcSize() const {
    if (this->points.empty()) {
        Element::x = 0;
        Element::y = 0;

        // The size of the rectangle, not the size of the pen!
        Element::width = 0;
        Element::height = 0;

        // used for snapping
        Element::snappedBounds = Rectangle<double>{};
    }

    double minX = DBL_MAX;
    double maxX = DBL_MIN;
    double minY = DBL_MAX;
    double maxY = DBL_MIN;

    double minSnapX = DBL_MAX;
    double maxSnapX = DBL_MIN;
    double minSnapY = DBL_MAX;
    double maxSnapY = DBL_MIN;

    bool hasPressure = points[0].z != Point::NO_PRESSURE;
    double halfThick = this->width / 2.0;  //  accommodate for pen width

    for (auto&& p: points) {
        if (hasPressure) {
            halfThick = p.z / 2.0;
        }

        minX = std::min(minX, p.x - halfThick);
        minY = std::min(minY, p.y - halfThick);

        maxX = std::max(maxX, p.x + halfThick);
        maxY = std::max(maxY, p.y + halfThick);

        minSnapX = std::min(minSnapX, p.x);
        minSnapY = std::min(minSnapY, p.y);

        maxSnapX = std::max(maxSnapX, p.x);
        maxSnapY = std::max(maxSnapY, p.y);
    }

    Element::x = minX;
    Element::y = minY;
    Element::width = maxX - minX;
    Element::height = maxY - minY;

    Element::snappedBounds = Rectangle<double>(minSnapX, minSnapY, maxSnapX - minSnapX, maxSnapY - minSnapY);
}

auto Stroke::getEraseable() -> EraseableStroke* { return this->eraseable; }

void Stroke::setEraseable(EraseableStroke* eraseable) { this->eraseable = eraseable; }

void Stroke::debugPrint() {
    g_message("%s", FC(FORMAT_STR("Stroke {1} / hasPressure() = {2}") % (uint64_t)this % this->hasPressure()));

    for (auto&& p: points) {
        g_message("%lf / %lf", p.x, p.y);
    }

    g_message("\n");
}

auto Stroke::CRS() -> Stroke* {
    
    int pointCount = getPointCount();
    if (pointCount == 0) {
        return new Stroke();
    }
    
    /**
     * Set the pressure value for the last point
     */
    if (pointCount >= 2 && points[pointCount-2].z != Point::NO_PRESSURE){
        if (pointCount == 2) {
            setLastPressure(points[pointCount-2].z);
        } else {
            // Linearly extrapolate the pressure using the second and third to last ppints
            setLastPressure(2 * points[pointCount - 2].z - points[pointCount - 3].z);
        }
    }
    
    Spline catmullRom = Spline::getCentripetalCatmullRomInterpolation(points);
    
    std::list<Point> crpts =  catmullRom.toPointSequence();
    std::vector<Point> newPoints;
    newPoints.reserve(crpts.size());
    for (Point p: crpts) {
        newPoints.push_back(p);
    }

    Spline schneider_catmull_rom = Spline::getSchneiderApproximation(newPoints);
    
    g_message("                     CRS splines : %zu", schneider_catmull_rom.size());
    
    std::list<Point> pts = schneider_catmull_rom.toPointSequence();
    Stroke* result = new Stroke();
    for (auto&& p : pts) {
        result->addPoint(p);
    }
    result->applyStyleFrom(this);
    
    return result;
}

auto Stroke::schneider() -> Stroke* {

    int pointCount = getPointCount();
    if (pointCount == 0) {
        return new Stroke();
    }
    
    /**
     * Set the pressure value for the last point
     */
    if (pointCount >= 2 && points[pointCount-2].z != Point::NO_PRESSURE){
        if (pointCount == 2) {
            setLastPressure(points[pointCount-2].z);
        } else {
            // Linearly extrapolate the pressure using the second and third to last ppints
            setLastPressure(2 * points[pointCount - 2].z - points[pointCount - 3].z);
        }
    }
    
//     Spline catmullRom = Spline::getCentripetalCatmullRomInterpolation(points);
    
    Spline schneider = Spline::getSchneiderApproximation(points);

//     for (auto&& spline : catmullRom) {
//         result.splice(result.end(), spline.toPointSequence());
//     }
//     std::list<Point> result =  catmullRom.toPointSequence();
//     std::vector<Point> newPoints;
//     newPoints.reserve(result.size());
//     for (Point p: result) { newPoints.push_back(p); }
    
//     Spline schneider_catmull_rom = Spline::getSchneiderApproximation(newPoints);
    
    std::list<Point> pts = schneider.toPointSequence();
    Stroke* result = new Stroke();
    for (auto&& p : pts) {
        result->addPoint(p);
    }
    result->applyStyleFrom(this);

    g_message("Nb points: %d, Schneider segments: %zu", getPointCount(), schneider.size());//, schneider_catmull_rom.size());
// #define PLOT_STROKE
#ifdef PLOT_STROKE
    //     printf("\n\n");
    setlocale(LC_NUMERIC, "en_US.UTF-8");
    
    // Header
    printf("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n"
    "<svg xmlns:dc=\"http://purl.org/dc/elements/1.1/\"\n"
    "xmlns:cc=\"http://creativecommons.org/ns#\"\n"
    "xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\"\n"
    "xmlns:svg=\"http://www.w3.org/2000/svg\"\n"
    "xmlns=\"http://www.w3.org/2000/svg\"\n"
    "xmlns:sodipodi=\"http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd\"\n"
    "xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\"\n"
    "width=\"210mm\"\n"
    "height=\"297mm\"\n"
    "viewBox=\"300 0 400 100\"\n"
    "version=\"1.1\"\n"
    "id=\"svg8\"\n"
    "inkscape:version=\"1.0.1 (3bc2e813f5, 2020-09-07, custom)\"\n"
    "sodipodi:docname=\"CatmullRom.svg\">\n"
    "<defs id=\"defs2\" />\n"
    "<sodipodi:namedview id=\"base\"\n"
    "pagecolor=\"#ffffff\"\n"
    "bordercolor=\"#666666\"\n"
    "borderopacity=\"1.0\"\n"
    "inkscape:pageopacity=\"0.0\"\n"
    "inkscape:pageshadow=\"2\"\n"
    "inkscape:zoom=\"0.35\"\n"
    "inkscape:cx=\"400\"\n"
    "inkscape:cy=\"540\"\n"
    "inkscape:document-units=\"mm\"\n"
    "inkscape:current-layer=\"layer2\"\n"
    "inkscape:document-rotation=\"0\"\n"
    "showgrid=\"false\"\n"
    "inkscape:window-width=\"1872\"\n"
    "inkscape:window-height=\"1043\"\n"
    "inkscape:window-x=\"48\"\n"
    "inkscape:window-y=\"0\"\n"
    "inkscape:window-maximized=\"1\" />\n"
    "<metadata id=\"metadata5\"><rdf:RDF><cc:Work rdf:about=\"\"><dc:format>image/svg+xml</dc:format><dc:type rdf:resource=\"http://purl.org/dc/dcmitype/StillImage\" /><dc:title /></cc:Work></rdf:RDF></metadata>");

    // The splines
    printf("<g inkscape:groupmode=\"layer\" id=\"layer2\" inkscape:label=\"Catmull-Rom\">\n");
    printf("<path style=\"fill:none;stroke:#000000;stroke-width:0.5px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1\" d=\"");
    printf("M %f %f ", catmullRom.getFirstKnot().x, catmullRom.getFirstKnot().y);
    for (auto&& seg : catmullRom.getSegments()) {
        printf("C %f %f, %f %f, %f %f ", seg.firstControlPoint.x, seg.firstControlPoint.y, seg.secondControlPoint.x, seg.secondControlPoint.y, seg.secondKnot.x, seg.secondKnot.y);
    }
    printf("\" />\n</g>\n");
    
    printf("<g inkscape:groupmode=\"layer\" id=\"layer3\" inkscape:label=\"Schneider\">\n");
    printf("<path style=\"fill:none;stroke:#000000;stroke-width:0.5px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1\" d=\"");
    printf("M %f %f ", schneider.getFirstKnot().x, schneider.getFirstKnot().y);
    for (auto&& seg : schneider.getSegments()) {
        printf("C %f %f, %f %f, %f %f ", seg.firstControlPoint.x, seg.firstControlPoint.y, seg.secondControlPoint.x, seg.secondControlPoint.y, seg.secondKnot.x, seg.secondKnot.y);
    }
    printf("\" />\n</g>\n");

    
    printf("<g inkscape:groupmode=\"layer\" id=\"layer1\" inkscape:label=\"Original segments\">\n");
    // Original segments
    printf("<path style=\"fill:none;stroke:#ff0000;stroke-width:0.2px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1\" d=\"");
    printf("M %f %f ", points[0].x, points[0].y);
    for (auto&& seg : catmullRom.getSegments()) {
        printf("L %f %f ", seg.secondKnot.x, seg.secondKnot.y);
    }
    printf("\" />\n");

    // Original points
    printf("<g stroke=\"red\" stroke-width=\"0.1\" fill=\"red\">");
    for (auto&& pt : points) {
        printf("<circle cx=\"%f\" cy=\"%f\" r=\"0.2\" />", pt.x, pt.y);
    }
    printf("</g>\n</g>\n</svg>\n");
#endif
    
    /**
     * Pressure values
     */
#ifdef DUMP_PRESSURE
    setlocale(LC_NUMERIC, "fr_FR.UTF-8");
    double length = 0;
    string interpolatePressure = "Interpolated\n0,000000;" + std::to_string(catmullRom.firstKnot.z) + "\n";
    string originalPressure = "Original\n0,000000;" + std::to_string(points[0].z) + "\n";
    int i = 0;
    for (auto&& seg : catmullRom.segments) {
        double dist = points[i].lineLengthTo(seg.secondKnot) / 10.0;
        for (int j = 1 ; j<=10 ; j++) {
            length += dist;
            double p = pow(10 - j, 3) * points[i].z + 3 * pow(10 - j, 2) * j * seg.firstControlPoint.z + 3 * (10 - j) * pow(j, 2) * seg.secondControlPoint.z + pow(j, 3) * seg.secondKnot.z;
            p /= 1000.0;
            interpolatePressure += std::to_string(length) + ";" + std::to_string(p) + "\n";
        }
        i++;
        originalPressure += std::to_string(length) + ";" + std::to_string(points[i].z) + "\n";
//         i++;
    }
    printf("\n%s\n%s\n\n", originalPressure.c_str(), interpolatePressure.c_str());
#endif
    
    return result;
}

// TODO: Change the StrokeHandler's behaviour on pressure
