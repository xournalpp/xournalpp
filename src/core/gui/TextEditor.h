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

#include <functional>  // for reference_wrapper
#include <string>      // for string
#include <vector>      // for vector

#include <gdk/gdk.h>           // for GdkEventKey
#include <glib.h>              // for gint, gboolean, gchar
#include <gtk/gtk.h>           // for GtkIMContext, GtkTextIter, GtkWidget
#include <pango/pangocairo.h>  // for cairo_t, PangoAttrList, PangoLayout

#include "util/Color.h"  // for Color
#include "util/Rectangle.h"
#include "util/raii/GObjectSPtr.h"
#include "util/raii/PangoSPtr.h"

class XojPageView;
class Text;
class TextUndoAction;
class UndoAction;
class XojFont;

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

    Text* getText();
    void textCopyed();

    void mousePressed(double x, double y);
    void mouseMoved(double x, double y);
    void mouseReleased();

    UndoAction* getFirstUndoAction() const;

    void setText(const std::string& text);
    void setFont(XojFont font);
    void afterFontChange();
    UndoAction* setColor(Color color);

private:
    /**
     * @brief Add the text to the providedd Pango layout.
     * The added text contains both this->text, and the preedit string of the Input Method (this->preeditstring)
     * This function also sets up the attributes of the preedit string (typically underlined)
     */
    void setTextToPangoLayout(PangoLayout* pl) const;

    xoj::util::Rectangle<double> computeBoundingRect();
    void repaintEditor();
    void drawCursor(cairo_t* cr, double x, double y, double height, double zoom) const;
    void repaintCursor();
    void resetImContext();

    int getByteOffset(int charOffset) const;
    int getCharOffset(int byteOffset) const;

    static void bufferPasteDoneCallback(GtkTextBuffer* buffer, GtkClipboard* clipboard, TextEditor* te);

    static void iMCommitCallback(GtkIMContext* context, const gchar* str, TextEditor* te);
    static void iMPreeditChangedCallback(GtkIMContext* context, TextEditor* te);
    static bool iMRetrieveSurroundingCallback(GtkIMContext* context, TextEditor* te);
    static bool imDeleteSurroundingCallback(GtkIMContext* context, gint offset, gint n_chars, TextEditor* te);

    void moveCursor(const GtkTextIter* newLocation, gboolean extendSelection);

    void calcVirtualCursor();
    void jumpALine(GtkTextIter* textIter, int count);

    void findPos(GtkTextIter* iter, double x, double y) const;
    void markPos(double x, double y, bool extendSelection);

    void contentsChanged(bool forceCreateUndoAction = false);

private:
    XojPageView* gui;
    GtkWidget* xournalWidget;
    Text* text;
    std::string lastText;

    xoj::util::GObjectSPtr<GtkWidget> textWidget;
    xoj::util::GObjectSPtr<GtkIMContext> imContext;
    xoj::util::GObjectSPtr<GtkTextBuffer> buffer;
    xoj::util::GObjectSPtr<PangoLayout> layout;

    // InputMethod preedit data
    xoj::util::PangoAttrListSPtr preeditAttrList;
    int preeditCursor;
    std::string preeditString;

    std::vector<std::reference_wrapper<TextUndoAction>> undoActions;

    double virtualCursor = 0;
    double markPosX = 0;
    double markPosY = 0;

    /**
     * Tracks the bounding box of the editor from the last render.
     *
     * Because adding or deleting lines may cause the size of the bounding box to change,
     * we need to rerender the union of the current and previous bboxes.
     */
    xoj::util::Rectangle<double> previousBoundingBox;

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
    bool markPosExtendSelection = false;
    bool markPosQueue = false;
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
