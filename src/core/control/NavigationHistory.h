/*
 * Xournal++
 *
 * Navigation history for jumping back/forward between document positions
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <cstddef>
#include <optional>
#include <vector>

#include "model/PageRef.h"
#include "util/Rectangle.h"

class Control;

class NavigationHistory {
public:
    explicit NavigationHistory(Control* control);
    ~NavigationHistory();

    /**
     * Record the current view as a navigation history entry.
     */
    void recordNavPoint();

    /**
     * @return Whether navigation history can move by dir (-1 back, +1 forward).
     */
    bool canNavigate(int dir) const;

    /**
     * Navigate the history by dir (-1 back, +1 forward).
     */
    void navigate(int dir);

    /**
     * Clear all history entries.
     */
    void reset();

    /**
     * Remove entries for pages that no longer exist.
     */
    void prune();

    /**
     * Update the enabled state of navigation actions.
     */
    void updateActions();

private:
    struct NavState {
        PageRef page;
        std::optional<xoj::util::Rectangle<double>> rect;
    };

    NavState captureState() const;
    bool isSameState(const NavState& a, const NavState& b) const;
    bool scrollToState(const NavState& state);

    static constexpr size_t MAX_HISTORY_LEN = 50;

    Control* control;
    std::vector<NavState> history;
    size_t historyIdx = 0;
};
