/*
 * Xournal++
 *
 * Enum available stabilizer
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

namespace StrokeStabilizer {

enum class AveragingMethod { NONE, ARITHMETIC, VELOCITY_GAUSSIAN };

constexpr bool isValid(AveragingMethod am) {
    return am == AveragingMethod::NONE || am == AveragingMethod::ARITHMETIC || am == AveragingMethod::VELOCITY_GAUSSIAN;
}

enum class Preprocessor { NONE, DEADZONE, INERTIA };

constexpr bool isValid(Preprocessor pp) {
    return pp == Preprocessor::NONE || pp == Preprocessor::DEADZONE || pp == Preprocessor::INERTIA;
}
}  // namespace StrokeStabilizer
