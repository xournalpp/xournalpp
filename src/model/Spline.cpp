#include "Spline.h"

#include <cmath>
#include <numeric>

// For debug purposes. Remove before merge
#include "i18n.h"
#define EXTRA_CAREFUL

Spline::Spline(const Point& firstKnot): firstKnot(firstKnot) {}

Spline::Spline(const Point& firstKnot, size_t size): firstKnot(firstKnot) { segments.reserve(size); }

Spline::Spline(const Point& firstKnot, const PartialSplineSegment& firstSegment):
        firstKnot(firstKnot), segments(1, firstSegment) {}


void Spline::addLineSegment(const Point& q) { segments.emplace_back(getLastKnot(), q); }

void Spline::addQuadraticSegment(const Point& cp, const Point& q) {
    segments.emplace_back(getLastKnot().relativeLineTo(cp, 2.0 / 3.0), q.relativeLineTo(cp, 2.0 / 3.0), q);
}

void Spline::addCubicSegment(const Point& fp, const Point& sp, const Point& q) { segments.emplace_back(fp, sp, q); }

void Spline::addCubicSegment(const MathVect3& fVelocity, const MathVect3 sVelocity, const Point& q) {
    segments.emplace_back(getLastKnot(), fVelocity, sVelocity, q);
}

void Spline::setFirstKnot(const Point& p) { firstKnot = p; }

auto Spline::getFirstKnot() const -> const Point& { return firstKnot; }

auto Spline::getLastKnot() const -> const Point& {
    if (segments.empty()) {
        return firstKnot;
    }
    return segments.back().secondKnot;
}

auto Spline::getSegments() const -> const std::vector<PartialSplineSegment>& { return segments; }

auto Spline::getBoundingBox() const -> Rectangle<double> {
    Rectangle<double> result(firstKnot.x, firstKnot.y, 0.0, 0.0);
    const Point* first = &firstKnot;
    for (auto&& seg: segments) {
        result.unite(seg.getBoundingBox(*first));
        first = &seg.secondKnot;
    }
    return result;
}

void Spline::toPoints(std::vector<Point>& points) const {
    const Point* first = &firstKnot;
    points.push_back(*first);
    for (auto&& seg: segments) {
        seg.toPoints(*first, points);
        first = &(seg.secondKnot);
    }
}

auto Spline::size() const -> size_t { return segments.size(); }

void Spline::resize(size_t n) {
    if (n < segments.size()) {
        segments.resize(n);
    }
}

void Spline::debugPrint() const {
    /**
     * 3D svg-like dump of the spline
     */
    g_message("%s", FC(FORMAT_STR("Spline {1}") % (uint64_t)this));
    g_message("M %f %f %f", firstKnot.x, firstKnot.y, firstKnot.z);
    for (auto&& seg: segments) {
        g_message("C %f %f %f, %f %f %f, %f %f %f", seg.firstControlPoint.x, seg.firstControlPoint.y,
                  seg.firstControlPoint.z, seg.secondControlPoint.x, seg.secondControlPoint.y, seg.secondControlPoint.z,
                  seg.secondKnot.x, seg.secondKnot.y, seg.secondKnot.z);
    }
}

/**
 * Catmull-Rom interpolation
 */
auto Spline::getCentripetalCatmullRomInterpolation(const std::vector<Point>& points) -> Spline {
    /**
     * Reference: https://qroph.github.io/2018/07/30/smooth-paths-using-catmull-rom-splines.html
     */

    size_t pointCount = points.size();
    if (pointCount == 0) {
        g_warning("Catmull-Rom interpolation on 0 points. This should never happen");
        return Spline(Point(-1, -1));
    }

    Spline result(points[0]);

    if (pointCount == 1) {
        return result;
    }

    if (pointCount == 2) {
        result.addLineSegment(points[1]);
        return result;
    }

    MathVect3 v01(points[0], points[1]);
    MathVect3 v12(points[1], points[2]);

//     if (pointCount == 3) { // Is this special case really necessary?
//         /**
//          * Heuristically produce two quadratic spline segments
//          */
//         MathVect3 u(points[0], points[2]);
//         u /= u.norm();
// 
//         double c01 = -v01.norm() / 3.0;
//         result.addCubicSegment(2.0 / 3.0 * v01 + c01 * u, c01 * u, points[1]);
// 
//         double c12 = v12.norm() / 3.0;
//         result.addCubicSegment(c12 * u, c12 * u - 2.0 / 3.0 * v12, points[2]);
// 
//         return result;
//     }

    CatmullRomComputer crc(v01, v12);

    /**
     * Heuristic: the first spline segment is quadratic.
     */
    MathVect3 u = -crc.t01 * crc.m;
    result.addCubicSegment(u + 2.0 / 3.0 * v01, u, points[1]);

    MathVect3 fVelocity = crc.t12 * crc.m;

    auto it = std::next(points.begin(), 3);  // skip the first three points
    while (it != points.end()) {
        crc.addStep(MathVect3(it[-1], *it));
        result.addCubicSegment(fVelocity, -crc.t01 * crc.m, it[-1]);
        fVelocity = crc.t12 * crc.m;
        it++;
    }

    /**
     * Heuristic: the last spline segment is quadratic.
     */
    result.addCubicSegment(fVelocity, fVelocity - 2.0 / 3.0 * crc.getLastVector(), points.back());

    return result;
}

Spline::CatmullRomComputer::CatmullRomComputer(const MathVect3& u, const MathVect3& v): head(0) {
    t12 = std::pow(u.dx * u.dx + u.dy * u.dy + u.dz * u.dz, 0.25);  // = sqrt(u.norm())
    diff[0] = u;
    addStep(v);
}

void Spline::CatmullRomComputer::addStep(const MathVect3& u) {
    t01 = t12;
    t12 = std::pow(u.dx * u.dx + u.dy * u.dy + u.dz * u.dz, 0.25);  // = sqrt(u.norm())
    const size_t oldhead = head;
    head = (head + 1) % 2;
    diff[head] = u;
    double inverse = 1.0 / (t01 + t12);
    m = ((1.0 / t12 - inverse) / 3.0) * diff[head] + ((1.0 / t01 - inverse) / 3.0) * diff[oldhead];
}

auto Spline::CatmullRomComputer::getLastVector() -> const MathVect3& { return diff[head]; }


/**
 * Schneider approximation
 */
auto Spline::getSchneiderApproximation(const std::vector<Point>& points) -> Spline {
    if (points.empty()) {
        g_warning("Spline::getSchneiderApproximation called on empty vector. This should never happen");
        return Spline(Point(0, 0));
    }
    SchneiderApproximater approximator(points);
#ifdef EXTRA_CAREFUL
    approximator.printStats();
#endif
    return approximator.getSpline();
}

Spline::SchneiderApproximater::SchneiderApproximater(const std::vector<Point>& points):
        points(points), spline(points.front()), chordLength{0.0} {
    size_t maxIndex = points.size() - 1;
    if (maxIndex > 0) {
        /**
         * Fill the chordLength
         */
        chordLength.reserve(points.size());
        double length = 0.0;
        auto it = std::next(points.cbegin());
        for (; it != points.cend(); it++) {
            length += it->lineLengthTo(it[-1]);
            chordLength.push_back(length);
        }

        /**
         * Compute the tangent vectors
         */
        MathVect3 firstTangentVector(points[0], points[1]);
        MathVect3 secondTangentVector(points[maxIndex], points[maxIndex - 1]);
        firstTangentVector.normalize();
        secondTangentVector.normalize();

#ifdef EXTRA_CAREFUL
//         g_message("points.size() = %zu, chordLength.size() = %zu", points.size(), chordLength.size());
        if (firstTangentVector.dx == 0.0 && firstTangentVector.dy == 0.0) {  // && firstTangentVector.dz == 0.0) {
            g_warning("Spline::SchneiderApproximater::SchneiderApproximater: first tangent vector is 0");
        }
        if (secondTangentVector.dx == 0.0 && secondTangentVector.dy == 0.0) {  // && secondTangentVector.dz == 0.0) {
            g_warning("Spline::SchneiderApproximater::SchneiderApproximater: second tangent vector is 0");
        }
#endif
        fitCubic(0, firstTangentVector, secondTangentVector, maxIndex);
    }
}

void Spline::SchneiderApproximater::printStats() {
    size_t nbPoints = points.size();
    size_t nbSegments = spline.size();
    totalNbSegments += nbSegments;
    totalNbPoints += nbPoints;
    g_message("Schneider: %3zu pts => %3zu segs. Total %4zu pts => %4zu segs",
              nbPoints, nbSegments, totalNbPoints, totalNbSegments);
}
size_t Spline::SchneiderApproximater::totalNbPoints = 0;
size_t Spline::SchneiderApproximater::totalNbSegments = 0;


auto Spline::SchneiderApproximater::getSpline() -> Spline { return spline; }

void Spline::SchneiderApproximater::fitCubic(size_t lowerIndex, const MathVect3& firstTangentVector,
                                             const MathVect3& secondTangentVector, size_t upperIndex) {

#ifdef EXTRA_CAREFUL
//     g_message("fitCubic: %zu, %zu", lowerIndex, upperIndex);
    if (lowerIndex >= upperIndex || upperIndex >= points.size()) {
        g_warning("Spline::SchneiderApproximater::fitCubic called on invalid indices");
        return;
    }
#endif

    const Point& secondKnot = points[upperIndex];

    if (upperIndex == lowerIndex + 1) {
        /**
         * Use Wu-Barsky heuristic
         * TODO try it out with Catmull-Rom
         */
        double scale = points[lowerIndex].lineLengthTo(secondKnot) / 3.0;

#ifdef EXTRA_CAREFUL
//         g_message("   Bof: %zu -- %zu", lowerIndex, upperIndex);
        if ((!points[lowerIndex].equalsPos(spline.getLastKnot())) || points[lowerIndex].z != spline.getLastKnot().z) {
            g_warning("Spline::SchneiderApproximater::fitCubic: Wrong last knot!");
        }
#endif

        spline.addCubicSegment(scale * firstTangentVector, scale * secondTangentVector, secondKnot);
        return;
    }

    // Declared before the scope starts
    size_t middleIndex;
    MathVect3 middleTangentVector;
    { // Scope of segmentFitter
        SingleSegmentFitter segmentFitter(firstTangentVector, secondTangentVector,
                                          std::next(points.cbegin(), lowerIndex), std::next(points.cbegin(), upperIndex),
                                          getStandardParametrization(lowerIndex, upperIndex));
        
        bool parametrizationValid = true;
        double lastError = ITERATION_ERROR;
        
        for (size_t i = 0 ; i < NUMBER_OF_ITERATIONS ; i++) {
            segmentFitter.findBestCubicSegment();
            double maxError = segmentFitter.computeMaxError();
            if (maxError < ERROR && parametrizationValid) {
//                 g_message("   OK!!");
                spline.addCubicSegment(segmentFitter.getFirstVelocity(), segmentFitter.getSecondVelocity(), secondKnot);
                return;
            }
            if (maxError >= lastError) {
                /**
                * The error is either to big or getting bigger
                */
                break;
            }
            lastError = maxError;
            parametrizationValid = segmentFitter.reparametrize();
//             g_message("  - reparametrize");
        }

        /**
        * Fitting failed. Split at worst point and try fitting both sides separately
        */
        auto worstPoint = segmentFitter.getWorstPoint();
        middleTangentVector = getMiddleTangent(worstPoint);
        middleIndex = std::distance(points.cbegin(), worstPoint);
    }  // End of scope of segmentFitter
    
    fitCubic(lowerIndex, firstTangentVector, -middleTangentVector, middleIndex);
    fitCubic(middleIndex, middleTangentVector, secondTangentVector, upperIndex);
}

std::vector<double> Spline::SchneiderApproximater::getStandardParametrization(size_t lowerIndex, size_t upperIndex) {
    auto beginIt = std::next(chordLength.cbegin(), lowerIndex);
    auto endIt = std::next(chordLength.cbegin(), upperIndex + 1);
    double reference = *beginIt;
    double length = chordLength[upperIndex] - reference;

    std::vector<double> result;
    result.reserve(upperIndex - lowerIndex + 1);

    if (fuzzyVanish(length)) {  // (length == 0.0) {
        g_warning("Spline::SchneiderApproximater::getStandardParametrization: length zero");
        result.resize(upperIndex - lowerIndex + 1, 0.0);
    } else {
        std::transform(beginIt, endIt, std::back_inserter(result),
                       [reference, length](double u) { return (u - reference) / length; });
    }
#ifdef EXTRA_CAREFUL
//     string msg = "stdParam:";
//     for (auto&& u: result) {
//         msg += " " + std::to_string(u);
//     }
//     g_message("%s", msg.c_str());
#endif
    return result;
}

MathVect3 Spline::SchneiderApproximater::getMiddleTangent(std::vector<Point>::const_iterator it) {
    MathVect3 result(it[-1], it[1]);
    result.normalize();
    return result;
}


Spline::SchneiderApproximater::SingleSegmentFitter::SingleSegmentFitter(const MathVect3& fTgt, const MathVect3& sTgt,
                                                          std::vector<Point>::const_iterator pointsBegin,
                                                          std::vector<Point>::const_iterator pointsLast,
                                                          std::vector<double> parametrization):
        fTgt(fTgt),
        sTgt(sTgt),
        diff(*pointsBegin, *pointsLast),
        sp_fTgt_sTgt(MathVect3::scalarProduct(fTgt, sTgt)),
        sp_fTgt_diff(MathVect3::scalarProduct(fTgt, diff)),
        sp_sTgt_diff(MathVect3::scalarProduct(sTgt, diff)),
        fTgtZero(fTgt.dx == 0.0 && fTgt.dy == 0.0), // && fTgt.dz == 0.0),  // No need to fuzzy compare (see fTgt.normalize())
        sTgtZero(sTgt.dx == 0.0 && sTgt.dy == 0.0), // && sTgt.dz == 0.0),  // No need to fuzzy compare (see sTgt.normalize())
        pointsBegin(pointsBegin),
        pointsEnd(std::next(pointsLast)),
        worstPoint(std::next(pointsBegin, std::distance(pointsBegin, pointsEnd) / 2)),
        parametrization(parametrization) {
#ifdef EXTRA_CAREFUL
    if(parametrization.size() != std::distance(pointsBegin, pointsEnd)) {
        g_warning("Spline::SchneiderApproximater::SingleSegmentFitter::SingleSegmentFitter: wrong parametrization size %zu vs %zu", parametrization.size(), std::distance(pointsBegin, pointsEnd));
    }
#endif
}

void Spline::SchneiderApproximater::SingleSegmentFitter::findBestCubicSegment() {
    /**
     * Find the best possible (single) spline segment fitting the data points, with prescribed endpoints and with
     * initial and final velocity vectors proportional to the provided tangent vectors
     *
     * The fitted parameters are the norms of the two velocity vectors.
     */
    double C[2][2] = {{0.0, 0.0}, {0.0, 0.0}};
    double X[2] = {0.0, 0.0};

    const Point firstPoint = *(pointsBegin);

    auto itPts = pointsBegin;
    auto itParam = parametrization.cbegin();
    while (itPts != pointsEnd) {
        /**
         * Fill the matrices. Compared to Schneider's original code
         *      https://github.com/erich666/GraphicsGems/blob/master/gems/FitCurves.c
         * or to
         *      https://github.com/paperjs/paper.js/
         * this has been optimized by:
         *  * computing most scalar products only once
         *  * multiplying by the scale factors after taking the scalar products
         *  * expressing the points' coordinates relatively to the first point so the computation of X has less terms
         */
        double u = *itParam;
        double t = 1 - u;
        double b = 3 * u * t;
        //         double b0 = t * t * t;
        double b1 = b * t;
        double b2 = b * u;
        //         double b3 = u * u * u;
        double b23 = b2 + u * u * u;

        C[0][0] += b1 * b1;
        C[0][1] += b1 * b2;
        C[1][1] += b2 * b2;

        MathVect3 step(firstPoint, *itPts);

        X[0] += b1 * (MathVect3::scalarProduct(fTgt, step) - (b23 * sp_fTgt_diff));
        X[1] += b2 * (MathVect3::scalarProduct(sTgt, step) - (b23 * sp_sTgt_diff));

        itPts++;
        itParam++;
    }
    C[0][0] = (fTgtZero ? 0.0 : C[0][0]);
    C[0][1] *= sp_fTgt_sTgt;
    C[1][0] = C[0][1];
    C[1][1] = (sTgtZero ? 0.0 : C[1][1]);
#ifdef EXTRA_CAREFUL
    if (itParam != parametrization.cend()) {
        g_warning("Spline::SchneiderApproximater::SingleSegmentFitter::findBestCubicSegment: Not the end of parametrization");
    }
#endif

    /**
     * Will contain the result: best norms of the velocity vectors
     */
    double alpha0;
    double alpha1;

    double detC = C[0][0] * C[1][1] - C[0][1] * C[1][0];
//     g_message(" ======== detC = %f", detC);
    if (!fuzzyVanish(detC)) { // detC != 0.0) {
        /**
         * Use Kramer's rule to solve the system
         */
        alpha0 = (C[1][1] * X[0] - C[0][1] * X[1]) / detC;
        alpha1 = (C[0][0] * X[1] - C[1][0] * X[0]) / detC;
        
//         g_message(" --- Nominal fit: %f  %f", alpha0, alpha1);
    } else {
        /**
         * The system is under-determined. Try assuming alpha0 == alpha1.
         */
        double c = C[0][0] + C[0][1];
        if (!fuzzyVanish(c)) {  //(c != 0.0) {
//             g_message(" --- Subnominal fit");
            alpha0 = X[0] / c;
        } else {
            c = C[1][0] + C[1][1];
            if (!fuzzyVanish(c)) {  //(c != 0.0) {
//                 g_message(" --- Subnominal fit");
                alpha0 = X[1] / c;
            } else {
                alpha0 = 0.0;
            }
        }
        alpha1 = alpha0;
    }

    // TODO: Should this test be fuzzy?
    if (alpha0 <= 0.0 || alpha1 <= 0.0) {
//         g_message(" --- Wu-Barsky");
        /**
         * Fallback to the Wu-Barsky heuristic
         * TODO: does it make sense to try out Catmull-Rom here?
         */
        alpha0 = alpha1 = diff.norm() / 3.0;
    }
    /**
     * The code in
     *      https://github.com/paperjs/paper.js/blob/master/src/path/PathFitter.js
     * has an additional check here: are the projections of the calculated velocity vectors onto the line
     *          firstPoint -- points[upperIndex]
     * in the "right" order? This test was not present in Schneider's original algorithm. What are its benefits?
     */
    
    fVelocity = alpha0 * fTgt;
    sVelocity = alpha1 * sTgt;
}

double Spline::SchneiderApproximater::SingleSegmentFitter::computeMaxError() {
    double maxDistance = 0.0;
    
    errors.clear();
    errors.reserve(parametrization.size());
    
    auto itPts = pointsBegin;
    auto itParam = parametrization.cbegin();
    while (itPts != pointsEnd) {
        double u = *itParam;
        double t = 1 - u;
        double ut3 = u * t * 3;
        /**
         * Compute the error vector. To simplify the formulae, the origin is set to firstKnot = *pointBegin.
         */
        MathVect3 error = (t * ut3) * fVelocity + (ut3 * u) * sVelocity + ((1 + 2 * t) * u * u) * diff - MathVect3(*pointsBegin, *itPts);
        
        double dist = MathVect3::scalarProduct(error, error);  // Squared norm
        if (dist > maxDistance) {
            maxDistance = dist;
            worstPoint = itPts;
        }
        errors.push_back(error);

        itParam++;
        itPts++;
    }
//     g_message(" ***** maxError = %f", maxDistance);
    return maxDistance;
}

bool Spline::SchneiderApproximater::SingleSegmentFitter::reparametrize() {
    
    auto errorIt = errors.cbegin();
    for (double& u : parametrization) {
        double t = 1 - u;
        double threeUMinusTwo = 3.0 * u - 2.0;
        double oneMinusThreeU = -1.0 - threeUMinusTwo;
        double twoU = 2.0 * u;
        
        /**
         * Compute the derivatives for Newton-Raphson iteration
         */
        MathVect3 thirdQprimeOfU = t * oneMinusThreeU * fVelocity + (twoU * t) * diff + (-u * threeUMinusTwo) * sVelocity;
        MathVect3 sixthQsecondOfU = threeUMinusTwo * fVelocity + (1 - twoU) * diff + oneMinusThreeU * sVelocity;

        double denominator = 3.0 * MathVect3::scalarProduct(thirdQprimeOfU, thirdQprimeOfU) + 2.0 * MathVect3::scalarProduct(*errorIt, sixthQsecondOfU);

        if (fuzzyVanish(denominator)) { // denominator == 0.0) {
            g_warning("Spline::SchneiderApproximater::SingleSegmentFitter::reparametrize: zero denominator");
            errorIt++;
            continue;
        }
        double numerator = MathVect3::scalarProduct(*errorIt, thirdQprimeOfU);
        u -= numerator / denominator;
        errorIt++;
    }
    
    auto it = std::next(parametrization.cbegin());
    while (it != parametrization.cend()) {
        if (*it <= it[-1]) {
            /**
             * The reparametrization messed the order up. The new parametrization is invalid.
             */
            return false;
        }
        it++;
    }
    return true;
}

MathVect3 Spline::SchneiderApproximater::SingleSegmentFitter::getFirstVelocity() const { return fVelocity; }

MathVect3 Spline::SchneiderApproximater::SingleSegmentFitter::getSecondVelocity() const { return sVelocity; }

std::vector<Point>::const_iterator Spline::SchneiderApproximater::SingleSegmentFitter::getWorstPoint() const {
    return worstPoint;
}
