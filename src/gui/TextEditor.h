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

#include <gtk/gtk.h>

#include "gui/Redrawable.h"
#include "model/Text.h"
#include "undo/TextUndoAction.h"
#include "undo/UndoAction.h"

class XojPageView;

class TextEditor {
public:
    TextEditor(XojPageView* gui, GtkWidget* widget, Text* text, bool ownText);
    virtual ~TextEditor();

    /** Represents the different kinds of text selection */
    enum class SelectType { word, paragraph, all };

    void paint(cairo_t* cr, GdkRectangle* rect, double zoom);

    bool onKeyPressEvent(GdkEventKey* event);
    bool onKeyReleaseEvent(GdkEventKey* event);

    void toggleOverwrite();
    void selectAtCursor(TextEditor::SelectType ty);
    void toggleBold();
    void incSize();
    void decSize();
    void moveCursor(GtkMovementStep step, int count, bool extendSelection);
    void deleteFromCursor(GtkDeleteType type, int count);
    void backspace();
    void copyToCliboard();
    void cutToClipboard();
    void pasteFromClipboard();
    string getSelection();

    Text* getText();
    void textCopyed();

    void mousePressed(double x, double y);
    void mouseMoved(double x, double y);
    void mouseReleased();

    UndoAction* getFirstUndoAction();

    void setText(const string& text);
    void setFont(XojFont font);
    UndoAction* setColor(Color color);

private:
    void repaintEditor();
    void drawCursor(cairo_t* cr, double x, double y, double height, double zoom);
    void repaintCursor();
    void resetImContext();

    int getByteOffset(int charOffset);
    int getCharOffset(int byteOffset);

    static void bufferPasteDoneCallback(GtkTextBuffer* buffer, GtkClipboard* clipboard, TextEditor* te);

    static void iMCommitCallback(GtkIMContext* context, const gchar* str, TextEditor* te);
    static void iMPreeditChangedCallback(GtkIMContext* context, TextEditor* te);
    static bool iMRetrieveSurroundingCallback(GtkIMContext* context, TextEditor* te);
    static bool imDeleteSurroundingCallback(GtkIMContext* context, gint offset, gint n_chars, TextEditor* te);

    void moveCursor(const GtkTextIter* newLocation, gboolean extendSelection);

    static gint blinkCallback(TextEditor* te);

    void calcVirtualCursor();
    void jumpALine(GtkTextIter* textIter, int count);

    void findPos(GtkTextIter* iter, double x, double y);
    void markPos(double x, double y, bool extendSelection);

    void contentsChanged(bool forceCreateUndoAction = false);

private:
    XojPageView* gui = nullptr;
    GtkWidget* widget = nullptr;
    GtkWidget* textWidget = nullptr;
    GtkIMContext* imContext = nullptr;
    GtkTextBuffer* buffer = nullptr;
    PangoLayout* layout = nullptr;
    Text* text = nullptr;

    string preeditString;
    string lastText;

    std::vector<std::reference_wrapper<TextUndoAction>> undoActions;

    double virtualCursor = 0;
    double markPosX = 0;
    double markPosY = 0;

    int cursorBlinkTime = 0;
    int cursorBlinkTimeout = 0;
    int blinkTimeout = 0;  // handler id

    bool ownText = false;
    bool markPosExtendSelection = false;
    bool markPosQueue = false;
    bool needImReset = false;
    bool mouseDown = false;
    bool cursorOverwrite = false;
    bool cursorVisible = false;
};
