/*
 * Xournal Extended
 *
 * A selection while you are selection, not for editing, only for selection
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __EDITSELECTION_H__
#define __EDITSELECTION_H__

#include <gtk/gtk.h>
#include "../model/Page.h"
#include "Redrawable.h"
#include "UndoRedoHandler.h"

enum CursorSelectionType {
	CURSOR_SELECTION_NONE = 0,
	//Inside the selection
	CURSOR_SELECTION_MOVE = 1,
	// Edges
	CURSOR_SELECTION_TOP_LEFT,
	CURSOR_SELECTION_TOP_RIGHT,
	CURSOR_SELECTION_BOTTOM_LEFT,
	CURSOR_SELECTION_BOTTOM_RIGHT,
	// Sides
	CURSOR_SELECTION_LEFT,
	CURSOR_SELECTION_RIGHT,
	CURSOR_SELECTION_TOP,
	CURSOR_SELECTION_BOTTOM,
};

class XournalWidget;
class MoveUndoAction;

class EditSelection: public MemoryCheckObject, public ElementContainer {
public:
	EditSelection(UndoRedoHandler * undo, double x, double y, double width, double height, XojPage * page,
			Redrawable * view);
	EditSelection(UndoRedoHandler * undo, Selection * selection, Redrawable * view);
	EditSelection(UndoRedoHandler * undo, Element * e, Redrawable * view, XojPage * page);
	~EditSelection();
	void paint(cairo_t * cr, GdkEventExpose *event, double zoom);

	CursorSelectionType getSelectionTypeForPos(double x, double y, double zoom);
	void setEditMode(CursorSelectionType selType, double x, double y);
	void move(double x, double y, Redrawable * view, XournalWidget * xournal);
	void doMove(double dx, double dy, Redrawable * view, XournalWidget * xournal);
	CursorSelectionType getEditMode();
	void finalizeEditing();

	Redrawable * getInputView();
	Redrawable * getView();
	XojPage * getPage();

	void fillUndoItemAndDelete(DeleteUndoAction * undo);

	/**
	 * Gets the selected objects, this should not be edited or freed
	 */
	GList * getElements();

	double getX();
	double getY();
	double getWidth();
	double getHeight();

	UndoAction * setSize(ToolSize size, const double * thiknessPen, const double * thiknessHilighter,
			const double * thiknessEraser);

	UndoAction * setColor(int color);

	UndoAction * setFont(XojFont & font);

	void addElement(Element * e);

private:
	void deleteViewBuffer();

	void initAttributes();

	void drawAnchorRect(cairo_t * cr, double x, double y, double zoom);

	void addElementInt(Element * e);

	static bool repaintSelection(EditSelection * selection);

private:
	double x;
	double y;
	double width;
	double height;

	double originalWidth;
	double originalHeight;

	double relativeX;
	double relativeY;

	CursorSelectionType selType;
	double selX;
	double selY;

	double mouseX;
	double mouseY;

	double offsetX;
	double offsetY;

	bool aspectRatio;

	Redrawable * inputView;
	Redrawable * view;

	UndoRedoHandler * undo;
	MoveUndoAction * moveUndoAction;

	GList * selected;
	XojPage * page;
	Layer * layer;

	DocumentView * documentView;

	cairo_surface_t * crBuffer;
	int rescaleId;

	friend class MoveUndoAction;
};

class SizeUndoAction: public UndoAction {
public:
	SizeUndoAction(XojPage * page, Layer * layer, Redrawable * view);
	~SizeUndoAction();

	virtual bool undo(Control * control);
	virtual bool redo(Control * control);
	virtual String getText();

	void addStroke(Stroke * s, double originalWidth, double newWidt, double * originalPressure, double * newPressure,
			int pressureCount);

public:
	static double * getPressure(Stroke * s);

private:
	GList * data;

	Layer * layer;
	Redrawable * view;
};

class ColorUndoAction: public UndoAction {
public:
	ColorUndoAction(XojPage * page, Layer * layer, Redrawable * view);
	~ColorUndoAction();

	virtual bool undo(Control * control);
	virtual bool redo(Control * control);
	virtual String getText();

	void addStroke(Element * e, int originalColor, double newColor);
private:
	GList * data;

	Layer * layer;
	Redrawable * view;
};

class FontUndoAction: public UndoAction {
public:
	FontUndoAction(XojPage * page, Layer * layer, Redrawable * view);
	~FontUndoAction();

	virtual bool undo(Control * control);
	virtual bool redo(Control * control);
	virtual String getText();

	void addStroke(Text * e, XojFont & oldFont, XojFont & newFont);
private:
	GList * data;

	Layer * layer;
	Redrawable * view;
};

#endif /* __EDITSELECTION_H__ */
