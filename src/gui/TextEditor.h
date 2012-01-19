/*
 * Xournal++
 *
 * Text editor gui (for Text Tool)
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __TEXTEDITOR_H__
#define __TEXTEDITOR_H__

#include <gtk/gtk.h>
#include "../gui/Redrawable.h"
#include "../model/Text.h"
#include "../undo/UndoAction.h"

class PageView;

class TextEditor {
public:
	TextEditor(PageView * gui, GtkWidget * widget, Text * text, bool ownText);
	virtual ~TextEditor();

	void paint(cairo_t * cr, GdkRectangle * rect, double zoom);

	bool onKeyPressEvent(GdkEventKey * event);
	bool onKeyReleaseEvent(GdkEventKey * event);

	void toggleOverwrite();
	void selectAll();
	void moveCursor(GtkMovementStep step, int count, bool extendSelection);
	void deleteFromCursor(GtkDeleteType type, int count);
	void backspace();
	void copyToCliboard();
	void cutToClipboard();
	void pasteFromClipboard();
	String getSelection();

	Text * getText();
	void textCopyed();

	void mousePressed(double x, double y);
	void mouseMoved(double x, double y);
	void mouseReleased();

	UndoAction * getFirstUndoAction();

	void setText(String text);
	void setFont(XojFont font);
	UndoAction * setColor(int color);

private:
	void repaintEditor();
	void drawCursor(cairo_t * cr, double x, double y, double height, double zoom);
	void repaintCursor();
	void resetImContext();

	int getByteOffset(int charOffset);
	int getCharOffset(int byteOffset);

	static void iMCommitCallback(GtkIMContext * context, const gchar * str, TextEditor * te);
	static void iMPreeditChangedCallback(GtkIMContext * context, TextEditor * te);
	static bool iMRetrieveSurroundingCallback(GtkIMContext * context, TextEditor * te);
	static bool imDeleteSurroundingCallback(GtkIMContext * context, gint offset, gint n_chars, TextEditor * te);

	void moveCursor(const GtkTextIter * newLocation, gboolean extendSelection);

	static gint blinkCallback(TextEditor * te);

	void calcVirtualCursor();
	void jumpALine(GtkTextIter * textIter, int count);

	void findPos(GtkTextIter * iter, double x, double y);
	void markPos(double x, double y, bool extendSelection);

	void contentsChanged(bool forceCreateUndoAction = false);

private:
	XOJ_TYPE_ATTRIB;

	PageView * gui;
	GtkWidget * widget;

	Text * text;
	bool ownText;

	GtkWidget * textWidget;

	GtkIMContext * imContext;
	String preeditString;
	bool needImReset;
	GtkTextBuffer * buffer;
	double virtualCursor;

	GList * undoActions;

	double markPosX;
	double markPosY;
	bool markPosExtendSelection;
	bool markPosQueue;

	bool cursorOverwrite;

	bool mouseDown;

	String lastText;

	PangoLayout * layout;

	int cursorBlinkTime;
	int cursorBlinkTimeout;
	bool cursorVisible;

	int blinkTimeout; // handler id
};

#endif /* __TEXTEDITOR_H__ */
