#include "XournalppCursor.h"

#include <cmath>             // for cos, sin, M_PI, NAN, fmod
#include <cstdint>           // for uint32_t
#include <initializer_list>  // for initializer_list

#include <cairo.h>                  // for cairo_move_to, cairo_lin...
#include <gdk-pixbuf/gdk-pixbuf.h>  // for GdkPixbuf
#include <glib-object.h>            // for g_object_unref
#include <gtk/gtk.h>                // for gtk_widget_get_window

#include "control/Control.h"                 // for Control
#include "control/ToolEnums.h"               // for TOOL_HAND, TOOL_PEN, TOO...
#include "control/ToolHandler.h"             // for ToolHandler
#include "control/settings/Settings.h"       // for Settings
#include "control/settings/SettingsEnums.h"  // for STYLUS_CURSOR_BIG, STYLU...
#include "control/zoom/ZoomControl.h"        // for ZoomControl
#include "gui/MainWindow.h"                  // for MainWindow
#include "util/Color.h"                      // for argb_to_GdkRGBA, rgb_to_...
#include "util/safe_casts.h"                 // for ceil_cast

#include "XournalView.h"  // for XournalView


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
    CRSR_RESIZE,

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
    cssCursors[CRSR_RESIZE              ] =     {"",""};            // "
};
// clang-format on

constexpr auto RESIZE_CURSOR_SIZE = 16;
constexpr auto DELTA_ANGLE_ARROW_HEAD = M_PI / 6.0;
constexpr auto LENGTH_ARROW_HEAD = 0.7;
constexpr auto RESIZE_CURSOR_HASH_PRECISION = 1000;


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
    if (type == TOOL_HAND || type == TOOL_VERTICAL_SPACE || type == TOOL_ERASER) {
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

void XournalppCursor::setRotationAngle(double angle) { this->angle = angle; }
void XournalppCursor::setMirror(bool mirror) { this->mirror = mirror; }

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
        gtk_widget_set_cursor_from_name(win->getWindow(), cssCursors[CRSR_BUSY].cssName);
    } else {
        gtk_widget_set_cursor(win->getWindow(), nullptr);
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
                    [[fallthrough]];
                case CURSOR_SELECTION_BOTTOM_RIGHT:
                    cursor = getResizeCursor(45);
                    break;
                case CURSOR_SELECTION_TOP_RIGHT:
                    [[fallthrough]];
                case CURSOR_SELECTION_BOTTOM_LEFT:
                    cursor = getResizeCursor(135);
                    break;
                case CURSOR_SELECTION_LEFT:
                    [[fallthrough]];
                case CURSOR_SELECTION_RIGHT:
                    cursor = getResizeCursor(180);
                    break;
                case CURSOR_SELECTION_TOP:
                    [[fallthrough]];
                case CURSOR_SELECTION_BOTTOM:
                    cursor = getResizeCursor(90);
                    break;
                case CURSOR_SELECTION_ROTATE:
                    setCursor(CRSR_EXCHANGE);
                    break;
                case CURSOR_SELECTION_DELETE:
                    setCursor(CRSR_PIRATE);
                    break;
                default:
                    break;
            }
        } else if (type == TOOL_PEN || type == TOOL_HIGHLIGHTER) {
            if (this->inputDevice == INPUT_DEVICE_MOUSE && !this->mouseDown)  // mouse and not pressed
            {
                setCursor(CRSR_ARROW);
            } else {
                if (type == TOOL_PEN) {
                    cursor = getPenCursor();
                } else  // must be:  if (type == TOOL_HIGHLIGHTER)
                {
                    cursor = getHighlighterCursor();
                }
            }
        } else if (type == TOOL_ERASER) {
            EraserVisibility visibility = control->getSettings()->getEraserVisibility();
            if ((this->inputDevice == INPUT_DEVICE_PEN || this->inputDevice == INPUT_DEVICE_ERASER) &&
                (visibility == ERASER_VISIBILITY_NEVER || (visibility == ERASER_VISIBILITY_HOVER && this->mouseDown) ||
                 (visibility == ERASER_VISIBILITY_TOUCH && !this->mouseDown))) {
                setCursor(CRSR_BLANK_CURSOR);
            } else {
                cursor = getEraserCursor();
            }
        }

        else if (type == TOOL_TEXT) {
            if (this->invisible) {
                setCursor(CRSR_BLANK_CURSOR);
            } else {
                setCursor(CRSR_XTERM);
            }
        } else if (type == TOOL_IMAGE) {
            setCursor(CRSR_TCROSS);
        } else if (type == TOOL_FLOATING_TOOLBOX) {
            setCursor(CRSR_DEFAULT);
        } else if (type == TOOL_VERTICAL_SPACE) {
            setCursor(CRSR_SB_V_DOUBLE_ARROW);
        } else if (type == TOOL_SELECT_OBJECT) {
            setCursor(CRSR_DEFAULT);
        } else if (type == TOOL_PLAY_OBJECT) {
            setCursor(CRSR_HAND2);
        } else if (type == TOOL_SELECT_PDF_TEXT_LINEAR) {
            setCursor(CRSR_XTERM);
        } else  // other selections are handled before anyway, because you can move a selection with every tool
        {
            setCursor(CRSR_TCROSS);
        }
    }

    if (cursor != nullptr) {
        gtk_widget_set_cursor(xournal->getWidget(), cursor);
        g_object_unref(cursor);
    }
    gtk_widget_set_sensitive(xournal->getWidget(), !this->busy);
}

/// Assumes ownership of s
static GdkCursor* surfaceToCursor(cairo_surface_t* s, int w, int h, int cx, int cy) {
    GdkPixbuf* pixbuf = gdk_pixbuf_get_from_surface(s, 0, 0, w, h);
    cairo_surface_destroy(s);
    GdkTexture* texture = gdk_texture_new_for_pixbuf(pixbuf);
    g_object_unref(pixbuf);
    GdkCursor* cursor = gdk_cursor_new_from_texture(texture, cx, cy, nullptr);
    g_object_unref(texture);
    return cursor;
}

auto XournalppCursor::getResizeCursor(double deltaAngle) -> GdkCursor* {
    if (this->mirror) {
        deltaAngle = -deltaAngle;
    }
    gulong flavour = static_cast<gulong>(RESIZE_CURSOR_HASH_PRECISION * fmod(angle + deltaAngle, 180.0));
    if (CRSR_RESIZE == this->currentCursor && flavour == this->currentCursorFlavour) {
        return nullptr;
    }
    this->currentCursor = CRSR_RESIZE;
    this->currentCursorFlavour = flavour;

    double a = (this->angle + deltaAngle) * M_PI / 180;
    cairo_surface_t* crCursor = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, RESIZE_CURSOR_SIZE, RESIZE_CURSOR_SIZE);
    cairo_t* cr = cairo_create(crCursor);
    cairo_set_source_rgba(cr, 0.1, 0.1, 0.1, 1);
    cairo_translate(cr, RESIZE_CURSOR_SIZE / 2, RESIZE_CURSOR_SIZE / 2);
    cairo_scale(cr, RESIZE_CURSOR_SIZE / 2, RESIZE_CURSOR_SIZE / 2);
    cairo_set_line_width(cr, 0.2);
    // draw double headed arrow rotated accordingly
    cairo_move_to(cr, cos(a), sin(a));
    cairo_line_to(cr, -cos(a), -sin(a));
    cairo_stroke(cr);
    // head and tail
    for (auto s: {-1, 1}) {
        cairo_move_to(cr, s * cos(a), s * sin(a));
        cairo_rel_line_to(cr, s * cos(a + M_PI + DELTA_ANGLE_ARROW_HEAD) * LENGTH_ARROW_HEAD,
                          s * sin(a + M_PI + DELTA_ANGLE_ARROW_HEAD) * LENGTH_ARROW_HEAD);
        cairo_move_to(cr, s * cos(a), s * sin(a));
        cairo_rel_line_to(cr, s * cos(a + M_PI - DELTA_ANGLE_ARROW_HEAD) * LENGTH_ARROW_HEAD,
                          s * sin(a + M_PI - DELTA_ANGLE_ARROW_HEAD) * LENGTH_ARROW_HEAD);
        cairo_stroke(cr);
    }

    cairo_destroy(cr);
    return surfaceToCursor(crCursor, RESIZE_CURSOR_SIZE, RESIZE_CURSOR_SIZE, RESIZE_CURSOR_SIZE / 2,
                           RESIZE_CURSOR_SIZE / 2);
}

auto XournalppCursor::getEraserCursor() -> GdkCursor* {

    // Eraser's size follow a quadratic increment, so the cursor will do the same
    double cursorSize = control->getToolHandler()->getThickness() * 2.0 * control->getZoomControl()->getZoom();
    gulong flavour = static_cast<gulong>(64 * cursorSize);

    if (CRSR_ERASER == this->currentCursor && flavour == this->currentCursorFlavour) {
        return nullptr;  // cursor already set
    }
    this->currentCursor = CRSR_ERASER;
    this->currentCursorFlavour = flavour;

    cairo_surface_t* surface =
            cairo_image_surface_create(CAIRO_FORMAT_ARGB32, ceil_cast<int>(cursorSize), ceil_cast<int>(cursorSize));
    cairo_t* cr = cairo_create(surface);
    cairo_rectangle(cr, 0, 0, cursorSize, cursorSize);
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_fill(cr);
    cairo_rectangle(cr, 0, 0, cursorSize, cursorSize);
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_stroke(cr);
    cairo_destroy(cr);
    return surfaceToCursor(surface, ceil_cast<int>(cursorSize), ceil_cast<int>(cursorSize),
                           round_cast<int>(cursorSize / 2.), round_cast<int>(cursorSize / 2.));
}


auto XournalppCursor::getHighlighterCursor() -> GdkCursor* {
    if (this->drawDirActive) {
        return createCustomDrawDirCursor(48, this->drawDirShift, this->drawDirCtrl);
    }

    return createHighlighterOrPenCursor(120 / 255.0);
}


auto XournalppCursor::getPenCursor() -> GdkCursor* {
    if ((control->getSettings()->getStylusCursorType() == STYLUS_CURSOR_NONE) &&
        !control->getSettings()->isHighlightPosition()) {
        setCursor(CRSR_BLANK_CURSOR);
        return nullptr;
    }
    if (control->getSettings()->getStylusCursorType() == STYLUS_CURSOR_ARROW) {
        setCursor(CRSR_ARROW);
        return nullptr;
    }
    if ((control->getSettings()->getStylusCursorType() != STYLUS_CURSOR_NONE) && this->drawDirActive) {
        return createCustomDrawDirCursor(48, this->drawDirShift, this->drawDirCtrl);
    }

    return createHighlighterOrPenCursor(1.0);
}

auto XournalppCursor::createHighlighterOrPenCursor(double alpha) -> GdkCursor* {
    auto irgb = control->getToolHandler()->getColor();
    if (const auto& params = control->getSettings()->getRecolorParameters(); params.recolorizeMainView) {
        irgb = params.recolor.convertColor(irgb);
    }
    auto drgb = Util::rgb_to_GdkRGBA(irgb);
    auto cursorType = control->getSettings()->getStylusCursorType();
    auto cursor = (cursorType == STYLUS_CURSOR_NONE) ? CRSR_BLANK_CURSOR : CRSR_PENORHIGHLIGHTER;
    bool bright = control->getSettings()->isHighlightPosition();
    double cursorSize = std::min(90., control->getToolHandler()->getThickness() * control->getZoomControl()->getZoom());
    int height = ceil_cast<int>(cursorSize);
    int width = height;

    // create a hash of variables so we notice if one changes despite being the same cursor type:
    gulong flavour = (cursorType == STYLUS_CURSOR_DOT ? 1U : 0U) | (cursorType == STYLUS_CURSOR_BIG ? 2U : 0U) |
                     (bright ? 4U : 0U) | static_cast<gulong>(64 * alpha) << 3U |
                     static_cast<gulong>(cursorSize) << 10U | static_cast<gulong>(uint32_t(irgb)) << 15U;

    if ((cursor == this->currentCursor) && (flavour == this->currentCursorFlavour)) {
        return nullptr;
    }
    this->currentCursor = cursor;
    this->currentCursorFlavour = flavour;

    if ((cursorType == STYLUS_CURSOR_BIG) || bright) {
        height = width = 90;
    }

    // We change the drawing method, now the center with the colored dot of the pen
    // is at the center of the cairo surface, and when we load the cursor, we load it
    // with the relative offset
    int centerX = width / 2;
    int centerY = height / 2;
    cairo_surface_t* crCursor = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    cairo_t* cr = cairo_create(crCursor);

    if (cursorType == STYLUS_CURSOR_BIG) {
        // When using highlighter, paint the icon with the current color
        if (alpha != 1.0) {
            gdk_cairo_set_source_rgba(cr, &drgb);
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
        // Highlight cursor with a circle
        auto&& color = Util::argb_to_GdkRGBA(control->getSettings()->getCursorHighlightColor());
        gdk_cairo_set_source_rgba(cr, &color);
        cairo_arc(cr, centerX, centerY, control->getSettings()->getCursorHighlightRadius(), 0, 2 * M_PI);
        cairo_fill_preserve(cr);
        auto&& borderColor = Util::argb_to_GdkRGBA(control->getSettings()->getCursorHighlightBorderColor());
        gdk_cairo_set_source_rgba(cr, &borderColor);
        cairo_set_line_width(cr, control->getSettings()->getCursorHighlightBorderWidth());
        cairo_stroke(cr);
    }

    if (cursorType != STYLUS_CURSOR_NONE) {
        auto drgbCopy = drgb;
        drgbCopy.alpha = alpha;
        gdk_cairo_set_source_rgba(cr, &drgbCopy);
        cairo_arc(cr, centerX, centerY, cursorSize / 2., 0, 2. * M_PI);
        cairo_fill(cr);
    }

    cairo_destroy(cr);
    return surfaceToCursor(crCursor, width, height, centerX, centerY);
}


void XournalppCursor::setCursor(guint cursorID) {
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

    GdkCursor* cursor = gdk_cursor_new_from_name(cssCursors[cursorID].cssName, nullptr);
    if (cursor == nullptr)  // failed to get a cursor, try backup cursor.
    {
        if (cursorID != CRSR_nullptr) {
            cursor = gdk_cursor_new_from_name(cssCursors[cursorID].cssBackupName, nullptr);

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
    gtk_widget_set_cursor(xournal->getWidget(), cursor);
    if (cursor) {
        g_object_unref(cursor);
    }
}


auto XournalppCursor::createCustomDrawDirCursor(int size, bool shift, bool ctrl) -> GdkCursor* {
    bool big = control->getSettings()->getStylusCursorType() == STYLUS_CURSOR_BIG;
    bool bright = control->getSettings()->isHighlightPosition();

    guint newCursorID = CRSR_DRAWDIRNONE + (shift ? 1 : 0) + (ctrl ? 2 : 0);
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
    return surfaceToCursor(crCursor, width, height, centerX, centerY);
}
