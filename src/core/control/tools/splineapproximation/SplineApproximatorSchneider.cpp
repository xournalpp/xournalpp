#include "SplineApproximatorSchneider.h"

#include <algorithm>

#ifdef SHOW_APPROX_STATS
#include <glib.h>  // for g_message
#endif

#include "util/Assert.h"

#include "SplineApproximatorSingleSegmentFitter.h"

SplineApproximator::Schneider::Schneider(const std::vector<Point>& points):
        points(points), spline(this->points.front()) {}

void SplineApproximator::Schneider::run() {
    APPROX_STATS(startTimer());
    size_t maxIndex = this->points.size() - 1;
    if (maxIndex <= 1) {
        // 2 points or less. No need to approximate
        APPROX_STATS(stopTimer());
        return;
    }

    const Point& firstPoint = this->points.front();

    /**
     * Compute the tangent vectors
     */
    MathVect3 firstTangentVector(firstPoint, this->points[1]);
    firstTangentVector.normalize();
    MathVect3 secondTangentVector(this->points.back(), this->points[maxIndex - 1]);
    secondTangentVector.normalize();

    // Populate the buffer
    this->buffer.reserve(maxIndex);
    double length = 0.0;

    for (auto pointIt1 = this->points.cbegin(), pointIt2 = std::next(pointIt1); pointIt2 != this->points.cend();
         pointIt1 = pointIt2++) {
        length += pointIt1->lineLengthTo(*pointIt2);
        this->buffer.emplace_back(firstPoint, *pointIt2, length, firstTangentVector);
    }

    xoj_assert(this->buffer.size() == maxIndex);

    fitCubic(this->buffer.begin(), firstTangentVector, secondTangentVector, this->buffer.end() - 1, 0.0);
    APPROX_STATS(stopTimer());
}

void SplineApproximator::Schneider::fitCubic(std::vector<BufferData>::iterator beginData,
                                             const MathVect3& firstTangentVector, const MathVect3& secondTangentVector,
                                             std::vector<BufferData>::iterator lastData, double ref) {

    xoj_assert(this->buffer.begin() <= beginData && beginData <= lastData && lastData < this->buffer.end());

    const Point& secondKnot = this->points[static_cast<size_t>(std::distance(this->buffer.begin(), lastData)) + 1];

    if (lastData == beginData) {
        /**
         * The buffer range corresponds to two consecutive points
         * Use Wu-Barsky heuristic
         */
        double scale = (lastData->chordLength - ref) / 3.0;
        this->spline.addCubicSegment(scale * firstTangentVector, scale * secondTangentVector, secondKnot);
        return;
    }

    std::vector<BufferData>::iterator worstPoint;

    if (SingleSegmentFitter segmentFitter(firstTangentVector, secondTangentVector, beginData, lastData, ref);
        segmentFitter.findBestCubicSegment()) {
        // Success. Push the segment found and return
        this->spline.addCubicSegment(segmentFitter.getFirstVelocity(), segmentFitter.getSecondVelocity(), secondKnot);
        return;
    } else {
        worstPoint = segmentFitter.getWorstPoint();
    }

    /**
     * Fitting failed. Split at worst point and try fitting both halves separately
     */
    MathVect3 middleTangentVector = getMiddleTangent(worstPoint);

    // First half
    fitCubic(beginData, firstTangentVector, -middleTangentVector, worstPoint, ref);

    // Repopulate the buffer for the second half
    auto pointIt = std::next(this->points.cbegin(), std::distance(this->buffer.begin(), worstPoint) + 1);
    const Point& firstPoint = *(pointIt++);

    std::transform(std::next(worstPoint), std::next(lastData), pointIt, std::next(worstPoint),
                   [&firstPoint, &middleTangentVector](const BufferData& d, const Point& p) {
                       return BufferData(firstPoint, p, d.chordLength, middleTangentVector);
                   });

    fitCubic(std::next(worstPoint), middleTangentVector, secondTangentVector, lastData, worstPoint->chordLength);
}

MathVect3 SplineApproximator::Schneider::getMiddleTangent(std::vector<BufferData>::iterator it) {
    auto i = std::next(this->points.begin(), std::distance(this->buffer.begin(), it) + 1);
    MathVect3 result(i[-1], i[1]);
    result.normalize();
    return result;
}

auto SplineApproximator::Schneider::getSpline() -> Spline { return this->spline; }

#ifdef SHOW_APPROX_STATS
void SplineApproximator::Schneider::printStats() {
    size_t nbPoints = this->points.size();
    size_t nbSegments = this->spline.nbSegments();
    totalNbSegments += nbSegments;
    totalNbPoints += nbPoints;
    g_message("Schneider: %3zu pts => %3zu segs. Total %4zu pts => %4zu segs (~ %4zu pts). Timer: %zu µs.", nbPoints,
              nbSegments, totalNbPoints, totalNbSegments, (totalNbSegments * 7 + 3) / 3, timeSpent.count());
}
size_t SplineApproximator::Schneider::totalNbPoints = 0;
size_t SplineApproximator::Schneider::totalNbSegments = 0;
std::chrono::microseconds SplineApproximator::Schneider::timeSpent = std::chrono::microseconds(0);
#endif
