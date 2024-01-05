/*
 * Xournal++
 *
 * [Header description]
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <memory>

#include "PositionInputData.h"  // for PositionInputData

class InputContext;
class XojPageView;
struct InputEvent;

/**
 * Abstract class for a specific input state
 */
class AbstractInputHandler {
private:
    bool blocked = false;

protected:
    InputContext* inputContext;
    bool inputRunning = false;

protected:
    XojPageView* getPageAtCurrentPosition(InputEvent const& event) const;
    std::shared_ptr<XojPageView> getPageViewRefAtCurrentPosition(InputEvent const& event) const;
    PositionInputData getInputDataRelativeToCurrentPage(XojPageView* page, InputEvent const& event) const;

public:
    explicit AbstractInputHandler(InputContext* inputContext);
    virtual ~AbstractInputHandler();

    void block(bool block);
    bool isBlocked() const;
    virtual void onBlock();
    virtual void onUnblock();
    bool handle(InputEvent const& event);
    virtual bool handleImpl(InputEvent const& event) = 0;
};
