/*
 * Xournal++
 *
 * Zoom change listener
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

class ZoomListener {
public:
    virtual void zoomChanged() = 0;
    virtual void zoomRangeValuesChanged();

    virtual ~ZoomListener();
};
