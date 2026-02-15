#include "NavigationHistory.h"

#include <cmath>    // for abs
#include <cstddef>  // for ptrdiff_t
#include <memory>   // for unique_ptr

#include "control/Control.h"
#include "control/ScrollHandler.h"
#include "control/actions/ActionDatabase.h"
#include "enums/Action.enum.h"
#include "gui/MainWindow.h"
#include "gui/XournalView.h"
#include "model/Document.h"
#include "util/Rectangle.h"
#include "util/Util.h"

NavigationHistory::NavigationHistory(Control* control): control(control) {}

NavigationHistory::~NavigationHistory() = default;

NavigationHistory::NavState NavigationHistory::captureState() const {
    NavState state;
    MainWindow* win = control->getWindow();
    if (!win) {
        return state;
    }

    auto* xournal = win->getXournal();
    if (!xournal) {
        return state;
    }

    size_t pageNo = control->getCurrentPageNo();
    Document* doc = control->getDocument();
    doc->lock();
    if (pageNo < doc->getPageCount()) {
        state.page = doc->getPage(pageNo);
    }
    doc->unlock();

    if (!state.page) {
        return state;
    }

    std::unique_ptr<xoj::util::Rectangle<double>> visibleRect(xournal->getVisibleRect(pageNo));
    if (visibleRect) {
        state.rect = *visibleRect;
    }

    return state;
}

bool NavigationHistory::isSameState(const NavState& a, const NavState& b) const {
    if (a.page.get() != b.page.get()) {
        return false;
    }
    if (a.rect.has_value() != b.rect.has_value()) {
        return false;
    }
    if (!a.rect) {
        return true;
    }

    constexpr double EPSILON = 0.5;
    return std::abs(a.rect->x - b.rect->x) < EPSILON &&
           std::abs(a.rect->y - b.rect->y) < EPSILON &&
           std::abs(a.rect->width - b.rect->width) < EPSILON &&
           std::abs(a.rect->height - b.rect->height) < EPSILON;
}

void NavigationHistory::prune() {
    if (history.empty()) {
        return;
    }

    Document* doc = control->getDocument();
    doc->lock();
    for (size_t i = history.size(); i > 0; --i) {
        size_t idx = i - 1;
        if (!history[idx].page || doc->indexOf(history[idx].page) == npos) {
            history.erase(history.begin() + static_cast<long>(idx));
            if (idx < historyIdx && historyIdx > 0) {
                historyIdx--;
            }
        }
    }
    doc->unlock();

    if (historyIdx > history.size()) {
        historyIdx = history.size();
    }
}

void NavigationHistory::reset() {
    history.clear();
    historyIdx = 0;
    updateActions();
}

void NavigationHistory::updateActions() {
    ActionDatabase* actionDB = control->getActionDatabase();
    if (!actionDB) {
        return;
    }

    actionDB->enableAction(Action::NAVIGATE_BACK, canNavigate(-1));
    actionDB->enableAction(Action::NAVIGATE_FORWARD, canNavigate(1));
}

void NavigationHistory::recordNavPoint() {
    NavState state = captureState();
    if (!state.page) {
        return;
    }

    prune();

    if (historyIdx < history.size()) {
        history.erase(history.begin() + static_cast<long>(historyIdx), history.end());
    }

    if (historyIdx > 0 && isSameState(state, history[historyIdx - 1])) {
        updateActions();
        return;
    }

    if (historyIdx >= MAX_HISTORY_LEN) {
        size_t removeCount = historyIdx - MAX_HISTORY_LEN + 1;
        history.erase(history.begin(), history.begin() + static_cast<long>(removeCount));
        historyIdx = MAX_HISTORY_LEN - 1;
    }

    history.push_back(state);
    historyIdx++;

    updateActions();
}

bool NavigationHistory::canNavigate(int dir) const {
    if (dir == 0) {
        return false;
    }

    if (dir < 0) {
        return historyIdx >= static_cast<size_t>(-dir);
    }

    return historyIdx + static_cast<size_t>(dir) < history.size();
}

bool NavigationHistory::scrollToState(const NavState& state) {
    ScrollHandler* scrollHandler = control->getScrollHandler();
    if (!state.page || !scrollHandler) {
        return false;
    }

    Document* doc = control->getDocument();
    doc->lock();
    size_t pageNo = doc->indexOf(state.page);
    doc->unlock();

    if (pageNo == npos) {
        return false;
    }

    if (state.rect) {
        const auto& r = *state.rect;
        scrollHandler->scrollToPage(pageNo, {r.x, r.y, r.x + r.width, r.y + r.height});
    } else {
        scrollHandler->scrollToPage(pageNo);
    }

    return true;
}

void NavigationHistory::navigate(int dir) {
    prune();

    if (!canNavigate(dir)) {
        updateActions();
        return;
    }

    NavState current = captureState();
    if (!current.page) {
        return;
    }

    if (historyIdx < history.size()) {
        history[historyIdx] = current;
    } else {
        history.push_back(current);
    }

    historyIdx = static_cast<size_t>(static_cast<ptrdiff_t>(historyIdx) + dir);

    if (!scrollToState(history[historyIdx])) {
        prune();
    }

    updateActions();
}
