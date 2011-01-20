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

class EditSelection: public MemoryCheckObject {
public:
	EditSelection(double x, double y, double width, double height, XojPage * page, Redrawable * view);
	EditSelection(Selection * selection, Redrawable * view);
	EditSelection(Element * e, Redrawable * view, XojPage * page);
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

	MoveUndoAction * getUndoAction();

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
	void initAttributes();

	void drawAnchorRect(cairo_t * cr, double x, double y, double zoom);

	void addElementInt(Element * e);

private:
	double x;
	double y;
	double width;
	double height;

	double relativeX;
	double relativeY;

	CursorSelectionType selType;
	double selX;
	double selY;

	double mouseX;
	double mouseY;

	double offsetX;
	double offsetY;

	Redrawable * inputView;
	Redrawable * view;

	MoveUndoAction * undo;
	MoveUndoAction * lastUndoAction;

	GList * selected;
	XojPage * page;
	Layer * layer;

	DocumentView * documentView;

	friend class MoveUndoAction;
};

class MoveUndoAction: public UndoAction {
public:
	MoveUndoAction(XojPage * page, EditSelection * selection);
	~MoveUndoAction();

	void finalize(EditSelection * selection);

	virtual bool undo(Control * control);
	virtual bool redo(Control * control);
	XojPage ** getPages();
	virtual String getText();

private:
	void acceptPositions(GList * pos);
	void switchLayer(GList * entries, Layer * oldLayer, Layer * newLayer);
	void repaint();
private:
	GList * originalPos;
	GList * newPos;
	XojPage * newPage;

	Layer * oldLayer;
	Layer * newLayer;

	Redrawable * origView;
	Redrawable * newView;

	friend class EditSelection;
};

class SizeUndoAction: public UndoAction {
public:
	SizeUndoAction(XojPage * page, Layer * layer, Redrawable * view);
	~SizeUndoAction();

	virtual bool undo(Control * control);
	virtual bool redo(Control * control);
	virtual String getText();

	void addStroke(Stroke * s, double originalWidth, double newWidt);
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
