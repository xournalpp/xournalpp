/*
 * Xournal++
 *
 * Handles the XournalppCursor
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>
#include <vector>

#include <gtk/gtk.h>

#include "control/tools/CursorSelectionType.h"
#include "gui/inputdevices/InputEvents.h"

#include "XournalType.h"

class Control;

class XournalppCursor {
public:
    XournalppCursor(Control* control);
    virtual ~XournalppCursor();

    void setCursorBusy(bool busy);
    void updateCursor();

    void setMouseSelectionType(CursorSelectionType selectionType);

    void setMouseDown(bool mouseDown);
    void setInvisible(bool invisible);
    void setInsidePage(bool insidePage);
    void activateDrawDirCursor(bool enable, bool shift = false, bool ctrl = false);
    void setInputDeviceClass(InputDeviceClass inputDevice);

private:
    void setCursor(int id);

    GdkCursor* getPenCursor();

    GdkCursor* getEraserCursor();
    GdkCursor* getHighlighterCursor();

    GdkCursor* createHighlighterOrPenCursor(int size, double alpha);
    GdkCursor* createCustomDrawDirCursor(int size, bool shift, bool ctrl);

private:
    InputDeviceClass inputDevice = INPUT_DEVICE_MOUSE;

    Control* control = nullptr;
    bool busy = false;
    bool insidePage = false;
    CursorSelectionType selectionType = CURSOR_SELECTION_NONE;

    bool mouseDown = false;
    bool invisible = false;

    // One shot drawDir custom cursor -drawn instead of pen/stylus then cleared.
    bool drawDirActive = false;
    bool drawDirShift = false;
    bool drawDirCtrl = false;

    // avoid re-assigning same cursor
    guint currentCursor = 0;        // enum AVAILABLECURSORS
    gulong currentCursorFlavour{};  // for different flavours of a cursor (i.e. drawdir, pen and hilighter custom
                                    // cursors)
};
