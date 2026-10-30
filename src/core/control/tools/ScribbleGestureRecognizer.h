/*
 * Xournal++
 *
 * Geometric scribble gesture candidate recognition
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <optional>
#include <span>

#include "control/settings/ScribbleSettings.h"
#include "util/Point.h"

class Point;

class ScribbleGestureRecognizer {
public:
    struct Geometry {
        xoj::util::Point<double> origin;
        xoj::util::Point<double> axis;
        double scale;
        double extent;  // Characteristic pass length used to confirm target revisits.
    };

    // The normalized frame lets target checks reuse the candidate's dominant axis without recomputing it.
    static auto analyze(std::span<const Point> points, ScribbleSettings settings = {}) -> std::optional<Geometry>;

    // A candidate is not permission to erase: target contact and input context must be checked separately.
    static auto isCandidate(std::span<const Point> points, ScribbleSettings settings = {}) -> bool;
};
