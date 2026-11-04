/*
 * Xournal++
 * @license GNU GPLv2 or later
 */
#pragma once

#include <algorithm>

struct ScribbleSettings {
    static constexpr int DEFAULT_SENSITIVITY = 50;
    static constexpr int DEFAULT_MINIMUM_PASSES = 4;
    int sensitivity = DEFAULT_SENSITIVITY;
    int minimumPasses = DEFAULT_MINIMUM_PASSES;

    auto normalized() const -> ScribbleSettings {
        return {std::clamp(sensitivity, 0, 100), std::clamp(minimumPasses, 2, 8)};
    }
    bool operator==(const ScribbleSettings&) const = default;
};
