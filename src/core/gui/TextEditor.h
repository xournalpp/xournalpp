/*
 * Xournal++
 *
 * Text editor gui (for Text Tool)
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>  // for string

#include <gdk/gdk.h>           // for GdkEventKey
#include <glib.h>              // for gint, gboolean, gchar
#include <gtk/gtk.h>           // for GtkIMContext, GtkTextIter, GtkWidget
#include <pango/pangocairo.h>  // for cairo_t, PangoAttrList, PangoLayout

#include "util/Color.h"  // for Color
#include "util/Range.h"
#include "util/raii/CStringWrapper.h"
#include "util/raii/GObjectSPtr.h"
#include "util/raii/PangoSPtr.h"

class XojPageView;
class Text;
class XojFont;

namespace xoj::util {
template <class T>
class Rectangle;
};

class TextEditor {
public:
    TextEditor(XojPageView* gui, GtkWidget* widget, Text* text, bool ownText);
    virtual ~TextEditor();

    /** Represents the different kinds of text selection */
    enum class SelectType { WORD, PARAGRAPH, ALL };

    void paint(cairo_t* cr, double zoom);

    bool onKeyPressEvent(GdkEventKey* event);
    bool onKeyReleaseEvent(GdkEventKey* event);

    void toggleOverwrite();
    void selectAtCursor(TextEditor::SelectType ty);
    void toggleBoldFace();
    void increaseFontSize();
    void decreaseFontSize();
    void moveCursor(GtkMovementStep step, int count, bool extendSelection);
    void deleteFromCursor(GtkDeleteType type, int count);
    void backspace();
    void copyToClipboard() const;
    void cutToClipboard();
    void pasteFromClipboard();
    std::string getSelection() const;

    Text* getText() const;
    void textCopyed();

    void mousePressed(double x, double y);
    void mouseMoved(double x, double y);
    void mouseReleased();

    void replaceBufferContent(const std::string& text);
    void setFont(XojFont font);
    void afterFontChange();
    void setColor(Color color);

private:
    /**
     * @brief Add the text to the provided Pango layout.
     * The added text contains both this->text, and the preedit string of the Input Method (this->preeditstring)
     * This function also sets up the attributes of the preedit string (typically underlined)
     */
    void setTextToPangoLayout(PangoLayout* pl) const;

    Range computeBoundingBox() const;
    void repaintEditor(bool sizeChanged = true);

    /**
     * @brief Draws the cursor
     * @return The bounding box of the cursor, in TextBox coordinates (i.e relative to the text box upper left corner)
     *          The bounding box is returned even if the cursor is currently not visible (blinking...)
     */
    xoj::util::Rectangle<double> drawCursor(cairo_t* cr, double zoom) const;

    void repaintCursor();
    void resetImContext();

    static void bufferPasteDoneCallback(GtkTextBuffer* buffer, GtkClipboard* clipboard, TextEditor* te);

    static void iMCommitCallback(GtkIMContext* context, const gchar* str, TextEditor* te);
    static void iMPreeditChangedCallback(GtkIMContext* context, TextEditor* te);
    static bool iMRetrieveSurroundingCallback(GtkIMContext* context, TextEditor* te);
    static bool imDeleteSurroundingCallback(GtkIMContext* context, gint offset, gint n_chars, TextEditor* te);

    void moveCursor(const GtkTextIter* newLocation, gboolean extendSelection);

    void computeVirtualCursorPosition();
    void jumpALine(GtkTextIter* textIter, int count);

    void findPos(GtkTextIter* iter, double x, double y) const;
    void markPos(double x, double y, bool extendSelection);

    void contentsChanged(bool forceCreateUndoAction = false);

private:
    XojPageView* gui;
    GtkWidget* xournalWidget;
    Text* text;

    xoj::util::GObjectSPtr<GtkWidget> textWidget;
    xoj::util::GObjectSPtr<GtkIMContext> imContext;
    xoj::util::GObjectSPtr<GtkTextBuffer> buffer;
    xoj::util::GObjectSPtr<PangoLayout> layout;

    // InputMethod preedit data
    int preeditCursor;
    xoj::util::PangoAttrListSPtr preeditAttrList;
    xoj::util::OwnedCString preeditString;

    /**
     * @brief Tracks the bounding box of the editor from the last render.
     *
     * Because adding or deleting lines may cause the size of the bounding box to change,
     * we need to repaint the union of the current and previous bboxes.
     */
    Range previousBoundingBox;

    /**
     * @brief Coordinate of the virtual cursor, in Pango coordinates.
     * (The virtual cursor is used when moving the cursor vertically (e.g. pressing up arrow), to get a good "vertical
     * move" feeling, even if we pass by (say) an empty line)
     */
    int virtualCursorAbscissa = 0;

    // cursor blinking timings. In millisecond.
    unsigned int cursorBlinkingTimeOn = 0;
    unsigned int cursorBlinkingTimeOff = 0;
    struct BlinkTimer {
        BlinkTimer(unsigned int id = 0): id(id) {}
        BlinkTimer(const BlinkTimer&) = delete;
        BlinkTimer(BlinkTimer&&) = delete;
        BlinkTimer& operator=(const BlinkTimer&) = delete;
        BlinkTimer& operator=(BlinkTimer&&) = delete;
        BlinkTimer& operator=(unsigned int newId) {
            if (id) {
                g_source_remove(id);
            }
            id = newId;
            return *this;
        }
        ~BlinkTimer() {
            if (id) {
                g_source_remove(id);
            }
        }
        static bool callback(TextEditor* te);

    private:
        unsigned int id = 0;  // handler id
    } blinkTimer;
    bool cursorBlink = true;

    bool ownText = false;
    bool needImReset = false;
    bool mouseDown = false;
    bool cursorOverwrite = false;
    bool cursorVisible = false;

    // Padding between the text logical box and the frame
    static constexpr int PADDING_IN_PIXELS = 5;
    // Width of the lines making the frame
    static constexpr int BORDER_WIDTH_IN_PIXELS = 1;
    // In a blinking period, how much time is the cursor visible vs not visible
    static constexpr unsigned int CURSOR_ON_MULTIPLIER = 2;
    static constexpr unsigned int CURSOR_OFF_MULTIPLIER = 1;
    static constexpr unsigned int CURSOR_DIVIDER = CURSOR_ON_MULTIPLIER + CURSOR_OFF_MULTIPLIER;
};
