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

#include <gdk/gdk.h>      // for GdkEventKey
#include <glib.h>         // for gint, gboolean, gchar
#include <gtk/gtk.h>      // for GtkIMContext, GtkTextIter, GtkWidget
#include <pango/pango.h>  // for PangoAttrList, PangoLayout

#include "model/OverlayBase.h"
#include "model/PageRef.h"  // for PageRef
#include "util/Color.h"     // for Color
#include "util/Range.h"
#include "util/raii/CStringWrapper.h"
#include "util/raii/GObjectSPtr.h"
#include "util/raii/PangoSPtr.h"

class Text;
class XojFont;
class Control;
class TextEditorCallbacks;
struct KeyEvent;

namespace xoj::util {
template <class T>
class DispatchPool;
};

namespace xoj::view {
class TextEditionView;
};

class TextEditor: public OverlayBase {
public:
    TextEditor(Control* control, const PageRef& page, GtkWidget* xournalWidget, double x, double y);
    virtual ~TextEditor();

    /** Represents the different kinds of text selection */
    enum class SelectType { WORD, PARAGRAPH, ALL };

    bool onKeyPressEvent(const KeyEvent& event);
    bool onKeyReleaseEvent(const KeyEvent& event);
    void mousePressed(double x, double y);
    void mouseMoved(double x, double y);
    void mouseReleased();

    /**
     * @brief Returns a pointer to the edited Text element.
     * Warning: The content of the Text element does not need to be up to date with the buffer's content
     * Use `updateTextElementContent` to sync them
     */
    Text* getTextElement() const;

    bool bufferEmpty() const;

    void setFont(XojFont font);
    void setColor(Color color);

    PangoLayout* getUpToDateLayout() const;

    const std::shared_ptr<xoj::util::DispatchPool<xoj::view::TextEditionView>>& getViewPool() const;

    Color getSelectionColor() const;

    const Range& getCursorBox() const;
    const Range& getContentBoundingBox() const;

    bool isCursorVisible() const;

    void deleteFromCursor(GtkDeleteType type, int count);
    void copyToClipboard() const;
    void cutToClipboard();
    void pasteFromClipboard();
    void selectAtCursor(TextEditor::SelectType ty);

private:
    void toggleOverwrite();
    void toggleBoldFace();
    void increaseFontSize();
    void decreaseFontSize();
    void moveCursor(GtkMovementStep step, int count, bool extendSelection);
    void backspace();
    void linebreak();
    void tabulation();

    void afterFontChange();
    void replaceBufferContent(const std::string& text);

    void finalizeEdition();
    void initializeEditionAt(double x, double y);

private:
    /**
     * @brief Add the text to the provided Pango layout.
     * The added text contains both this->text, and the preedit string of the Input Method (this->preeditstring)
     * This function also sets up the attributes of the preedit string (typically underlined)
     */
    void setTextToPangoLayout(PangoLayout* pl) const;

    void setSelectionAttributesToPangoLayout(PangoLayout* pl) const;

    Range computeBoundingBox() const;
    void repaintEditor(bool sizeChanged = true);

    /**
     * @brief Compute the cursor's location
     * @return The bounding box of the cursor, in TextBox coordinates (i.e relative to the text box upper left corner)
     *          The bounding box is returned even if the cursor is currently not visible (blinking...)
     * WARNING: The returned box may have width == 0 (if in insertion mode or at the end of a line). In this case, the
     *          width of the displayed cursor should be decided by the view class (depending on zoom for instance)
     */
    Range computeCursorBox() const;

    void repaintCursorAfterChange();
    void resetImContext();

    static void bufferPasteDoneCallback(GtkTextBuffer* buffer, GdkClipboard* clipboard, TextEditor* te);

    static void iMCommitCallback(GtkIMContext* context, const gchar* str, TextEditor* te);
    static void iMPreeditChangedCallback(GtkIMContext* context, TextEditor* te);
    static bool iMRetrieveSurroundingCallback(GtkIMContext* context, TextEditor* te);
    static bool imDeleteSurroundingCallback(GtkIMContext* context, gint offset, gint n_chars, TextEditor* te);

    void moveCursorIterator(const GtkTextIter* newLocation, gboolean extendSelection);

    void computeVirtualCursorPosition();
    void jumpALine(GtkTextIter* textIter, int count);

    void findPos(GtkTextIter* iter, double x, double y) const;
    void markPos(double x, double y, bool extendSelection);

    void contentsChanged(bool forceCreateUndoAction = false);
    void updateCursorBox();

    void updateTextElementContent();

private:
    Control* control;
    PageRef page;

    /**
     * @brief Pointer to the main window's widget. Used for fetching settings and clipboards, and ringing the bell.
     */
    GtkWidget* xournalWidget;

    /**
     * @brief Text element under edition, clone of the original Text element (if any)
     */
    std::unique_ptr<Text> textElement;
    Text* originalTextElement;

    xoj::util::GObjectSPtr<GtkIMContext> imContext;
    xoj::util::GObjectSPtr<GtkTextBuffer> buffer;
    xoj::util::GObjectSPtr<PangoLayout> layout;

    enum class LayoutStatus { UP_TO_DATE, NEEDS_ATTRIBUTES_UPDATE, NEEDS_COMPLETE_UPDATE };
    mutable LayoutStatus layoutStatus;

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
    Range cursorBox;

    std::shared_ptr<xoj::util::DispatchPool<xoj::view::TextEditionView>> viewPool;

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
        static void callback(TextEditor* te);

    private:
        unsigned int id = 0;  // handler id
    } blinkTimer;
    bool cursorBlink = true;

    bool needImReset = false;
    bool mouseDown = false;
    bool cursorOverwrite = false;
    bool cursorVisible = false;

    // In a blinking period, how much time is the cursor visible vs not visible
    static constexpr unsigned int CURSOR_ON_MULTIPLIER = 2;
    static constexpr unsigned int CURSOR_OFF_MULTIPLIER = 1;
    static constexpr unsigned int CURSOR_DIVIDER = CURSOR_ON_MULTIPLIER + CURSOR_OFF_MULTIPLIER;

    struct KeyBindings;
    static const KeyBindings keyBindings;
};
