/*
 * Xournal++
 *
 * Base class for device input handling
 * Data to do an input
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <gdk/gdk.h>  // for GdkModifierType
#include <glib.h>     // for guint32

class PositionInputData {
public:
    bool isShiftDown() const;
    bool isControlDown() const;
    bool isAltDown() const;

public:
    double x;
    double y;
    double pressure;
    guint32 timestamp;

    /**
     * State flags from GDKevent (Shift down etc.)
     */
    GdkModifierType state;
};
