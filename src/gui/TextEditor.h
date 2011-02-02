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
#include "../control/Redrawable.h"
#include "../model/Text.h"

class PageView;
class UndoAction;

class TextEditor {
public:
	TextEditor(PageView * gui, Text * text, bool ownText);
	virtual ~TextEditor();

	void paint(cairo_t * cr, GdkEventExpose *event, double zoom);

	bool onKeyPressEvent(GdkEventKey *event);
	bool onKeyReleaseEvent(GdkEventKey *event);

	void toggleOverwrite();
	void selectAll();
	void moveCursor(GtkMovementStep step, int count, bool extend_selection);
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
private:
	void redrawEditor();
	void drawCursor(cairo_t * cr, double x, double y, double height, double zoom);
	void redrawCursor();
	void resetImContext();

	static void iMCommitCallback(GtkIMContext *context, const gchar *str, TextEditor * te);
	static void iMPreeditChangedCallback(GtkIMContext *context, TextEditor * te);
	static bool iMRetrieveSurroundingCallback(GtkIMContext *context, TextEditor * te);
	static bool imDeleteSurroundingCallback(GtkIMContext *context, gint offset, gint n_chars, TextEditor * te);

	void moveCursor(const GtkTextIter *new_location, gboolean extend_selection);

	static gint blinkCallback(TextEditor * te);

	void calcVirtualCursor();
	void jumpALine(GtkTextIter * textIter, int count);

	void findPos(GtkTextIter * iter, double x, double y);
	void markPos(double x, double y, bool extendSelection);

	void contentsChanged(bool forceCreateUndoAction = false);
private:
	PageView * gui;

	Text * text;
	bool ownText;

	GtkWidget *textWidget;

	GtkIMContext *imContext;
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
