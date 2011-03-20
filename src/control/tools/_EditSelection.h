/*
 * Xournal++
 *
 * A selection for editing, every selection (Rect, Lasso...) is
 * converted to this one if the selection is finished
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __EDITSELECTION_H__
#define __EDITSELECTION_H__

#include <glib.h>
#include "../../model/Page.h"
#include "../../model/Font.h"
#include "../../gui/Redrawable.h"
#include "../../view/ElementContainer.h"
#include "../../util/MemoryCheck.h"
#include "Selection.h"
#include "../../undo/UndoAction.h"
#include "../Tool.h"
#include "CursorSelectionType.h"

class XournalView;
class DocumentView;
class UndoRedoHandler;
class DeleteUndoAction;
class MoveUndoAction;

class EditSelection: public MemoryCheckObject, public ElementContainer {
public:
	EditSelection(UndoRedoHandler * undo, double x, double y, double width, double height, XojPage * page, Redrawable * view);
	EditSelection(UndoRedoHandler * undo, Selection * selection, Redrawable * view);
	EditSelection(UndoRedoHandler * undo, Element * e, Redrawable * view, XojPage * page);
	~EditSelection();

public:
	void paint(cairo_t * cr, double zoom);

	CursorSelectionType getSelectionTypeForPos(double x, double y, double zoom);
	void setEditMode(CursorSelectionType selType, double x, double y);
	void move(double x, double y, Redrawable * view, XournalView * xournal);
	void doMove(double dx, double dy, Redrawable * view, XournalView * xournal);
	CursorSelectionType getEditMode();
	void finalizeEditing();

	Redrawable * getInputView();
	Redrawable * getView();
	XojPage * getPage();

	void fillUndoItem(DeleteUndoAction * undo);

	/**
	 * Gets the selected objects, this should not be edited or freed
	 */
	GList * getElements();

	double getX();
	double getY();
	double getWidth();
	double getHeight();

	UndoAction * setSize(ToolSize size, const double * thiknessPen, const double * thiknessHilighter, const double * thiknessEraser);

	UndoAction * setColor(int color);

	UndoAction * setFont(XojFont & font);

	void addElement(Element * e);

	/**
	 * This is needed if the selection is "Deleted" then the selection needs to be cleared
	 */
	void clearContents();

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

#endif /* __EDITSELECTION_H__ */
