/*
 * Xournal++
 *
 * Interface for touch disable implementations
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once


class TouchDisableInterface {
public:
    TouchDisableInterface();
    virtual ~TouchDisableInterface();

public:
    virtual void enableTouch() = 0;
    virtual void disableTouch() = 0;
    virtual void init();

private:
};
