#include "XournalppCursor.h"

#include <cmath>

#include "control/Control.h"

#include "Util.h"
#include "XournalView.h"
#include "pixbuf-utils.h"


// NOTE:  Every cursor change must result in the setting of this->currentCursor to the new cursor type even for custom
// cursors.
// Custom cursors can also compare then set this->currentCursorFlavour to notice changes in colour or size etc. The
// flavour calculation is specific to each cursor type and is calculated differently within each type.


//  All the cursors we want to use. WARNING Make sure to set their css names in cssCursors[] below WARNING
enum AVAILABLECURSORS {
    CRSR_nullptr = 0,  // <--- Do Not Modify
    CRSR_BUSY,
    CRSR_MOVE,
    CRSR_MOVING,
    CRSR_GRAB,
    CRSR_GRABBING,
    CRSR_TOP_LEFT_CORNER,
    CRSR_TOP_RIGHT_CORNER,
    CRSR_BOTTOM_LEFT_CORNER,
    CRSR_BOTTOM_RIGHT_CORNER,
    CRSR_SB_H_DOUBLE_ARROW,
    CRSR_EXCHANGE,
    CRSR_PIRATE,
    CRSR_SB_V_DOUBLE_ARROW,
    CRSR_ARROW,
    CRSR_BLANK_CURSOR,
    CRSR_XTERM,
    CRSR_DEFAULT,
    CRSR_HAND2,
    CRSR_TCROSS,
    CRSR_PENORHIGHLIGHTER,
    CRSR_ERASER,
    CRSR_DRAWDIRNONE,       // drawdir* keep these consecutive and in order: none,shift,ctrl,shiftctrl
    CRSR_DRAWDIRSHIFT,      // "
    CRSR_DRAWDIRCTRL,       // "
    CRSR_DRAWDIRSHIFTCTRL,  // "


    CRSR_END_OF_CURSORS
};


struct cursorStruct {
    const gchar* cssName;
    const gchar* cssBackupName;
};


//  our enum mapped to css name and a place to store the cursor pointer.
// Note: including enum as error check (for now?)
cursorStruct cssCursors[CRSR_END_OF_CURSORS];


XournalppCursor::XournalppCursor(Control* control): control(control) {
    // clang-format off
	// NOTE: Go ahead and use a fancy css cursor... but specify a common backup cursor. 
	cssCursors[CRSR_nullptr                ] = 	{"",""};
	cssCursors[CRSR_BUSY                ] = 	{"wait", 		""					};
	cssCursors[CRSR_MOVE                ] = 	{"all-scroll", 	""					};
	cssCursors[CRSR_MOVING              ] = 	{"grabbing", 	""					};
	cssCursors[CRSR_GRAB                ] = 	{"grab", 		""					};
	cssCursors[CRSR_GRABBING            ] = 	{"grabbing", 	""					};
	cssCursors[CRSR_TOP_LEFT_CORNER     ] = 	{"nw-resize", 	""					};
	cssCursors[CRSR_TOP_RIGHT_CORNER    ] = 	{"ne-resize", 	""					};
	cssCursors[CRSR_BOTTOM_LEFT_CORNER  ] = 	{"sw-resize", 	""					};
	cssCursors[CRSR_BOTTOM_RIGHT_CORNER ] = 	{"se-resize", 	""					};
	cssCursors[CRSR_SB_H_DOUBLE_ARROW   ] = 	{"ew-resize", 	""					};
	cssCursors[CRSR_EXCHANGE            ] = 	{"exchange", 	"arrow"				};
	cssCursors[CRSR_PIRATE              ] = 	{"pirate", 		"arrow" 			};
	cssCursors[CRSR_SB_V_DOUBLE_ARROW   ] = 	{"ns-resize", 	""					};
	cssCursors[CRSR_ARROW               ] = 	{"default", 	""					};
	cssCursors[CRSR_BLANK_CURSOR        ] = 	{"none", 		""					};
	cssCursors[CRSR_XTERM               ] = 	{"text", 		""					};
	cssCursors[CRSR_DEFAULT             ] = 	{"default", 	""					};
	cssCursors[CRSR_HAND2               ] = 	{"hand2", 		""					};
	cssCursors[CRSR_TCROSS              ] = 	{"crosshair", 	""					};
	cssCursors[CRSR_PENORHIGHLIGHTER    ] = 	{"",""};			// custom cursors - enum used only for check
	cssCursors[CRSR_ERASER              ] = 	{"",""};			// "
	cssCursors[CRSR_DRAWDIRNONE         ] = 	{"",""};			// "
	cssCursors[CRSR_DRAWDIRSHIFT        ] = 	{"",""};			// "
	cssCursors[CRSR_DRAWDIRCTRL         ] = 	{"",""};			// "
	cssCursors[CRSR_DRAWDIRSHIFTCTRL    ] = 	{"",""};			// "
};
// clang-format on

XournalppCursor::~XournalppCursor() = default;


void XournalppCursor::setInputDeviceClass(InputDeviceClass device) { this->inputDevice = device; }


// pen or hi-light cursor will be a DrawDir cursor instead
void XournalppCursor::activateDrawDirCursor(bool enable, bool shift, bool ctrl) {
    this->drawDirActive = enable;
    this->drawDirShift = shift;
    this->drawDirCtrl = ctrl;
}


void XournalppCursor::setMouseDown(bool mouseDown) {
    if (this->mouseDown == mouseDown) {
        return;
    }

    this->mouseDown = mouseDown;
    ToolHandler* handler = control->getToolHandler();
    ToolType type = handler->getToolType();

    // Not always an update is needed
    if (type == TOOL_HAND || type == TOOL_VERTICAL_SPACE) {
        updateCursor();
    }
}


void XournalppCursor::setMouseSelectionType(CursorSelectionType selectionType) {
    if (this->selectionType == selectionType) {
        return;
    }
    this->selectionType = selectionType;
    updateCursor();
}


/*This handles setting the busy cursor for the main window and calls
 * updateCursor to set the busy cursor for the XournalWidget region.
 */
void XournalppCursor::setCursorBusy(bool busy) {
    MainWindow* win = control->getWindow();
    if (!win) {
        return;
    }

    if (this->busy == busy) {
        return;
    }

    this->busy = busy;

    if (busy) {
        GdkWindow* window = gtk_widget_get_window(win->getWindow());
        GdkCursor* cursor = gdk_cursor_new_from_name(gdk_window_get_display(window), cssCursors[CRSR_BUSY].cssName);
        gdk_window_set_cursor(window, cursor);
        g_object_unref(cursor);
    } else {
        if (gtk_widget_get_window(win->getWindow())) {
            gdk_window_set_cursor(gtk_widget_get_window(win->getWindow()), nullptr);
        }
    }

    updateCursor();
}


void XournalppCursor::setInsidePage(bool insidePage) {
    if (this->insidePage == insidePage) {
        return;
    }

    this->insidePage = insidePage;

    updateCursor();
}


void XournalppCursor::setInvisible(bool invisible) {
    if (this->invisible == invisible) {
        return;
    }

    this->invisible = invisible;

    updateCursor();
}


void XournalppCursor::updateCursor() {
    MainWindow* win = control->getWindow();
    if (!win) {
        return;
    }

    XournalView* xournal = win->getXournal();
    if (!xournal) {
        return;
    }

    GdkCursor* cursor = nullptr;


    if (this->busy) {
        setCursor(CRSR_BUSY);
    } else {
        ToolHandler* handler = control->getToolHandler();
        ToolType type = handler->getToolType();


        if (type == TOOL_HAND) {
            if (this->mouseDown) {
                setCursor(CRSR_GRABBING);
            } else {
                setCursor(CRSR_GRAB);
            }
        } else if (!this->insidePage) {
            setCursor(CRSR_DEFAULT);
        } else if (this->selectionType) {
            switch (this->selectionType) {
                case CURSOR_SELECTION_MOVE:
                    if (this->mouseDown) {
                        setCursor(CRSR_MOVING);
                    } else {
                        setCursor(CRSR_MOVE);
                    }
                    break;
                case CURSOR_SELECTION_TOP_LEFT:
                    setCursor(CRSR_TOP_LEFT_CORNER);
                    break;
                case CURSOR_SELECTION_TOP_RIGHT:
                    setCursor(CRSR_TOP_RIGHT_CORNER);
                    break;
                case CURSOR_SELECTION_BOTTOM_LEFT:
                    setCursor(CRSR_BOTTOM_LEFT_CORNER);
                    break;
                case CURSOR_SELECTION_BOTTOM_RIGHT:
                    setCursor(CRSR_BOTTOM_RIGHT_CORNER);
                    break;
                case CURSOR_SELECTION_LEFT:
                case CURSOR_SELECTION_RIGHT:
                    setCursor(CRSR_SB_H_DOUBLE_ARROW);
                    break;
                case CURSOR_SELECTION_ROTATE:
                    setCursor(CRSR_EXCHANGE);
                    break;
                case CURSOR_SELECTION_DELETE:
                    setCursor(CRSR_PIRATE);
                    break;
                case CURSOR_SELECTION_TOP:
                case CURSOR_SELECTION_BOTTOM:
                    setCursor(CRSR_SB_V_DOUBLE_ARROW);
                    break;
                default:
                    break;
            }
        } else if (type == TOOL_PEN || type == TOOL_HILIGHTER) {
            if (this->inputDevice == INPUT_DEVICE_MOUSE && !this->mouseDown)  // mouse and not pressed
            {
                setCursor(CRSR_ARROW);
            } else {
                if (type == TOOL_PEN) {
                    cursor = getPenCursor();
                } else  // must be:  if (type == TOOL_HILIGHTER)
                {
                    cursor = getHighlighterCursor();
                }
            }
        } else if (type == TOOL_ERASER) {
            cursor = getEraserCursor();
        }

        else if (type == TOOL_TEXT) {
            if (this->invisible) {
                setCursor(CRSR_BLANK_CURSOR);
            } else {
                setCursor(CRSR_XTERM);
            }
        } else if (type == TOOL_IMAGE) {
            setCursor(CRSR_DEFAULT);
        } else if (type == TOOL_FLOATING_TOOLBOX) {
            setCursor(CRSR_DEFAULT);
        } else if (type == TOOL_VERTICAL_SPACE) {
            if (this->mouseDown) {
                setCursor(CRSR_SB_V_DOUBLE_ARROW);
            }
        } else if (type == TOOL_SELECT_OBJECT) {
            setCursor(CRSR_DEFAULT);
        } else if (type == TOOL_PLAY_OBJECT) {
            setCursor(CRSR_HAND2);
        } else  // other selections are handled before anyway, because you can move a selection with every tool
        {
            setCursor(CRSR_TCROSS);
        }
    }

    GdkWindow* window = gtk_widget_get_window(xournal->getWidget());
    if (window) {
        if (cursor != nullptr) {
            gdk_window_set_cursor(window, cursor);
        }
        gtk_widget_set_sensitive(xournal->getWidget(), !this->busy);
    }

    gdk_display_sync(gdk_display_get_default());

    if (cursor != nullptr) {
        g_object_unref(cursor);
    }
}


auto XournalppCursor::getEraserCursor() -> GdkCursor* {

    if (CRSR_ERASER == this->currentCursor) {
        return nullptr;  // cursor already set
    }
    this->currentCursor = CRSR_ERASER;


    // Eraser's size follow a quadratic increment, so the cursor will do the same
    double cursorSize = control->getToolHandler()->getThickness() * 2 * control->getZoomControl()->getZoom();
    cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, cursorSize, cursorSize);
    cairo_t* cr = cairo_create(surface);
    cairo_rectangle(cr, 0, 0, cursorSize, cursorSize);
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_fill(cr);
    cairo_rectangle(cr, 0, 0, cursorSize, cursorSize);
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_stroke(cr);
    cairo_destroy(cr);
    GdkCursor* cursor =
            gdk_cursor_new_from_surface(gdk_display_get_default(), surface, cursorSize / 2.0, cursorSize / 2.0);
    cairo_surface_destroy(surface);
    return cursor;
}


auto XournalppCursor::getHighlighterCursor() -> GdkCursor* {
    if (this->drawDirActive) {
        return createCustomDrawDirCursor(48, this->drawDirShift, this->drawDirCtrl);
    }


    return createHighlighterOrPenCursor(5, 120 / 255.0);
}


auto XournalppCursor::getPenCursor() -> GdkCursor* {
    if (this->drawDirActive) {
        return createCustomDrawDirCursor(48, this->drawDirShift, this->drawDirCtrl);
    }


    return createHighlighterOrPenCursor(3, 1.0);
}


auto XournalppCursor::createHighlighterOrPenCursor(int size, double alpha) -> GdkCursor* {
    int rgb = control->getToolHandler()->getColor();
    double r = ((rgb >> 16) & 0xff) / 255.0;
    double g = ((rgb >> 8) & 0xff) / 255.0;
    double b = (rgb & 0xff) / 255.0;
    bool big = control->getSettings()->isShowBigCursor();
    bool bright = control->getSettings()->isHighlightPosition();
    int height = size;
    int width = size;

    // create a hash of variables so we notice if one changes despite being the same cursor type:
    gulong flavour = (big ? 1 : 0) | (bright ? 2 : 0) | static_cast<gulong>(64 * alpha) << 2 |
                     static_cast<gulong>(size) << 9 | static_cast<gulong>(rgb) << 14;

    if (CRSR_PENORHIGHLIGHTER == this->currentCursor && flavour == this->currentCursorFlavour) {
        return nullptr;
    }
    this->currentCursor = CRSR_PENORHIGHLIGHTER;
    this->currentCursorFlavour = flavour;

    if (big || bright) {
        height = width = 60;
    }

    // We change the drawing method, now the center with the colored dot of the pen
    // is at the center of the cairo surface, and when we load the cursor, we load it
    // with the relative offset
    int centerX = width / 2;
    int centerY = height / 2;
    cairo_surface_t* crCursor = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    cairo_t* cr = cairo_create(crCursor);

    if (big) {
        // When using highlighter, paint the icon with the current color
        if (size == 5) {
            cairo_set_source_rgb(cr, r, g, b);
        } else {
            cairo_set_source_rgb(cr, 1, 1, 1);
        }
        cairo_set_line_width(cr, 1.2);

        // Starting point
        cairo_move_to(cr, centerX + 2, centerY);
        // Pencil cursor
        cairo_line_to(cr, centerX + 2, centerY - 4);
        cairo_line_to(cr, centerX + 15, centerY - 17.5);
        cairo_line_to(cr, centerX + 19, centerY - 14);
        cairo_line_to(cr, centerX + 6, centerY);

        cairo_close_path(cr);
        cairo_fill_preserve(cr);
        cairo_set_source_rgb(cr, 0, 0, 0);
        cairo_stroke(cr);

        cairo_fill_preserve(cr);
    }

    if (bright) {
        // A yellow transparent circle with no border
        cairo_set_line_width(cr, 0);
        cairo_set_source_rgba(cr, 255, 255, 0, 0.5);
        cairo_arc(cr, centerX, centerY, 30, 0, 2 * 3.1415);
        cairo_fill_preserve(cr);
        cairo_set_source_rgb(cr, 0, 0, 0);
        cairo_stroke(cr);
    }

    cairo_set_source_rgba(cr, r, g, b, alpha);
    // Correct the offset of the coloured dot for big-cursor mode
    cairo_rectangle(cr, centerX, centerY, size, size);
    cairo_fill(cr);
    cairo_destroy(cr);
    GdkPixbuf* pixbuf = xoj_pixbuf_get_from_surface(crCursor, 0, 0, width, height);
    cairo_surface_destroy(crCursor);
    GdkCursor* cursor = gdk_cursor_new_from_pixbuf(
            gtk_widget_get_display(control->getWindow()->getXournal()->getWidget()), pixbuf, centerX, centerY);
    g_object_unref(pixbuf);
    return cursor;
}


void XournalppCursor::setCursor(int cursorID) {
    if (cursorID == this->currentCursor) {
        return;
    }

    MainWindow* win = control->getWindow();
    if (!win) {
        return;
    }

    XournalView* xournal = win->getXournal();
    if (!xournal) {
        return;
    }

    GdkWindow* window = gtk_widget_get_window(xournal->getWidget());
    if (!window) {
        return;
    }

    GdkCursor* cursor = gdk_cursor_new_from_name(gdk_window_get_display(window), cssCursors[cursorID].cssName);
    if (cursor == nullptr)  // failed to get a cursor, try backup cursor.
    {
        if (cursorID != CRSR_nullptr) {
            cursor = gdk_cursor_new_from_name(gdk_window_get_display(window), cssCursors[cursorID].cssBackupName);

            // Null cursor is ok but not wanted ... warn user
            if (cursor == nullptr) {
                if (CRSR_nullptr == this->currentCursor) {
                    return;  // We've already been here
                }
                g_warning("CSS Cursor and backup not valid '%s', '%s'", cssCursors[cursorID].cssName,
                          cssCursors[cursorID].cssBackupName);
            }
        }
        cursorID = cursor == nullptr ? CRSR_nullptr : cursorID;
    }

    this->currentCursor = cursorID;
    gdk_window_set_cursor(gtk_widget_get_window(xournal->getWidget()), cursor);
    gdk_window_set_cursor(window, cursor);
    if (cursor) {
        g_object_unref(cursor);
    }
}


auto XournalppCursor::createCustomDrawDirCursor(int size, bool shift, bool ctrl) -> GdkCursor* {
    bool big = control->getSettings()->isShowBigCursor();
    bool bright = control->getSettings()->isHighlightPosition();

    int newCursorID = CRSR_DRAWDIRNONE + (shift ? 1 : 0) + (ctrl ? 2 : 0);
    gulong flavour =
            (big ? 1 : 0) | (bright ? 2 : 0) | static_cast<gulong>(size) << 2;  // hash of variables for comparison only

    if (newCursorID == this->currentCursor && flavour == this->currentCursorFlavour) {
        return nullptr;
    }
    this->currentCursor = newCursorID;
    this->currentCursorFlavour = flavour;

    int height = size;
    int width = size;
    int fontSize = 8;
    if (big || bright) {
        height = width = 60;
        fontSize = 12;
    }
    int centerX = width - width / 4;
    int centerY = height - height / 4;


    cairo_surface_t* crCursor = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    cairo_t* cr = cairo_create(crCursor);
    cairo_set_line_width(cr, 1.2);

    // Starting point
    cairo_move_to(cr, centerX, height / 2);
    cairo_line_to(cr, centerX, height);
    cairo_stroke(cr);

    cairo_move_to(cr, width / 2, centerY);
    cairo_line_to(cr, width, centerY);
    cairo_stroke(cr);

    if (ctrl) {
        cairo_text_extents_t extents;
        const char* utf8 = "CONTROL";
        double x = NAN, y = NAN;
        cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
        cairo_set_font_size(cr, fontSize);
        cairo_text_extents(cr, utf8, &extents);
        x = 0;
        y = extents.height;
        cairo_move_to(cr, x, y);
        cairo_show_text(cr, utf8);
    }

    if (shift) {
        cairo_text_extents_t extents;
        const char* utf8 = "SHIFT";
        double x = NAN, y = NAN;
        cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
        cairo_set_font_size(cr, fontSize);
        cairo_text_extents(cr, utf8, &extents);
        x = 0;
        y = extents.height * 2.5;
        cairo_move_to(cr, x, y);
        cairo_show_text(cr, utf8);
    }

    cairo_destroy(cr);
    GdkPixbuf* pixbuf = xoj_pixbuf_get_from_surface(crCursor, 0, 0, width, height);
    cairo_surface_destroy(crCursor);
    GdkCursor* cursor = gdk_cursor_new_from_pixbuf(
            gtk_widget_get_display(control->getWindow()->getXournal()->getWidget()), pixbuf, centerX, centerY);
    g_object_unref(pixbuf);

    return cursor;
}
