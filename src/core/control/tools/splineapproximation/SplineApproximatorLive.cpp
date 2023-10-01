#include "SplineApproximatorLive.h"

#include "SplineApproximatorSingleSegmentFitter.h"

#ifdef SHOW_APPROX_STATS
#include <glib.h>  // for g_message
#endif

SplineApproximator::Live::Live(std::shared_ptr<Spline> ptr): dataCount(1), spline(ptr) {
    *this->P0 = this->spline->getFirstKnot();
}

bool SplineApproximator::Live::feedPoint(const Point& p) {
    APPROX_STATS(startTimer());

    this->pushToCircularBuffer(p);

    ++this->dataCount;
    if (this->dataCount <= 3) {
        this->processFirstEvents();
        APPROX_STATS(stopTimer());
        return true;
    }

    this->totalLength += this->P1->lineLengthTo(*this->P2);
    this->buffer.emplace_back(this->firstKnot, *this->P1, this->totalLength, this->firstTangentVector);

    MathVect3 sTgt(*this->P0, *this->P2);
    sTgt.normalize();

    if (SingleSegmentFitter segmentFitter(this->firstTangentVector, sTgt, this->buffer.begin(), this->buffer.end() - 1);
        segmentFitter.findBestCubicSegment()) {
        // Success. Push the segment found and return
        this->liveSegment = SplineSegment(this->firstKnot, segmentFitter.getFirstVelocity(),
                                          segmentFitter.getSecondVelocity(), *this->P1);
        APPROX_STATS(stopTimer());
        return true;
    }

    /**
     * Did not find a spline fitting our points.
     * Split after the last successful fitting
     */
    this->startNewSegment(sTgt);

    APPROX_STATS(stopTimer());
    return false;
}

bool SplineApproximator::Live::finalize() {
    APPROX_STATS(startTimer());
    /**
     * At least 3 points in the stroke.
     * It implies that *P0, *P1, *P2 have values assigned.
     */
    MathVect3 sTgt(*this->P0, *this->P1);
    double normP0P1 = sTgt.norm();
    sTgt.normalize(normP0P1);

    this->totalLength += normP0P1;
    this->buffer.emplace_back(this->firstKnot, *this->P0, this->totalLength, this->firstTangentVector);

    if (SingleSegmentFitter segmentFitter(this->firstTangentVector, sTgt, this->buffer.begin(), this->buffer.end() - 1);
        !segmentFitter.findBestCubicSegment()) {
        this->spline->addCubicSegment(this->liveSegment);
        // Use a quadratic segment euristics similar to Wu-Barsky to reach the last point
        double l = 0.5 * this->P1->lineLengthTo(*this->P0);
        MathVect3 v(*this->P2, *this->P0);
        v.normalize();
        Point Q = (l * v).translatePoint(*this->P1);
        this->liveSegment = SplineSegment(*this->P1, Q, *this->P0);
        this->spline->addQuadraticSegment(Q, *this->P0);

        APPROX_STATS(stopTimer());
        return false;
    } else {
        this->liveSegment = SplineSegment(this->firstKnot, segmentFitter.getFirstVelocity(),
                                          segmentFitter.getSecondVelocity(), *this->P0);
        this->spline->addCubicSegment(this->liveSegment);

        APPROX_STATS(stopTimer());
        return true;
    }
}

void SplineApproximator::Live::pushToCircularBuffer(const Point& p) {
    *this->P3 = p;
    Point* temp = this->P0;
    this->P0 = this->P3;
    this->P3 = this->P2;
    this->P2 = this->P1;
    this->P1 = temp;
}

void SplineApproximator::Live::processFirstEvents() {
    if (this->dataCount == 3) {
        // Just enough points for Wu-Barsky's euristics.

        this->firstKnot = *this->P2;

        this->firstTangentVector = MathVect3(this->firstKnot, *this->P1);
        double scale = this->firstTangentVector.norm();
        this->totalLength = scale;
        this->buffer.emplace_back(this->firstTangentVector, scale, scale);

        this->firstTangentVector.normalize(scale);
        scale /= 3.0;

        // Wu-Barsky
        MathVect3 sTgt(*this->P0, this->firstKnot);
        sTgt.normalize();
        this->liveSegment = SplineSegment(this->firstKnot, scale * this->firstTangentVector, scale * sTgt, *this->P1);
    } else {
        this->liveSegment = this->dataCount == 1 ? SplineSegment(*this->P0) : SplineSegment(*this->P0, *this->P1);
    }
}

void SplineApproximator::Live::startNewSegment(const MathVect3& lastTangentVector) {
    this->spline->addCubicSegment(this->liveSegment);

    this->firstKnot = *this->P2;

    this->firstTangentVector = MathVect3(*this->P3, *this->P1);
    this->firstTangentVector.normalize();

    this->totalLength = this->P1->lineLengthTo(this->firstKnot);
    this->buffer.clear();
    this->buffer.emplace_back(this->firstKnot, *this->P1, this->totalLength, this->firstTangentVector);

    // Wu-Barsky euristic for the liveSegment
    double scale = this->totalLength / 3.0;
    this->liveSegment =
            SplineSegment(this->firstKnot, scale * this->firstTangentVector, scale * lastTangentVector, *this->P1);
}


auto SplineApproximator::Live::getSpline() -> Spline { return *this->spline; }


#ifdef SHOW_APPROX_STATS
void SplineApproximator::Live::printStats() {
    size_t nbSegments = this->spline->nbSegments();
    totalNbSegments += nbSegments;
    totalNbPoints += this->dataCount;
    g_message("Live approximation: %3zu pts => %3zu segs. Total %4zu pts => %4zu segs (~ %4zu pts). Timer: %zu µs.",
              this->dataCount, nbSegments, totalNbPoints, totalNbSegments, (totalNbSegments * 7 + 3) / 3,
              timeSpent.count());
}
size_t SplineApproximator::Live::totalNbPoints = 0;
size_t SplineApproximator::Live::totalNbSegments = 0;
std::chrono::microseconds SplineApproximator::Live::timeSpent = std::chrono::microseconds(0);
#endif
