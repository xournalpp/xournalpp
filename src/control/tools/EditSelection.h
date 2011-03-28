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

#include "../../view/ElementContainer.h"
#include "../Tool.h"
#include "../../model/Font.h"
#include "CursorSelectionType.h"

#include "../../util/XournalType.h"

class UndoRedoHandler;
class XojPage;
class Layer;
class PageView;
class Selection;
class Element;
class UndoAction;

class EditSelection: public ElementContainer {
public:
	EditSelection(UndoRedoHandler * undo, double x, double y, double width, double height, XojPage * page, PageView * view);
	EditSelection(UndoRedoHandler * undo, Selection * selection, PageView * view);
	EditSelection(UndoRedoHandler * undo, Element * e, PageView * view, XojPage * page);
	virtual ~EditSelection();

private:
	/**
	 * Our internal constructor
	 */
	void contstruct(UndoRedoHandler * undo, PageView * view, XojPage * sourcePage);

public:
	/**
	 * get the X coordinate relative to the provided view (getView())
	 * in document coordinates
	 */
	double getXOnView();

	/**
	 * get the Y coordinate relative to the provided view (getView())
	 * in document coordinates
	 */
	double getYOnView();

	/**
	 * get the width in document coordinates (multiple with zoom)
	 */
	double getWidth();

	/**
	 * get the height in document coordinates (multiple with zoom)
	 */
	double getHeight();

	/**
	 * get the source page (where the selection was done)
	 */
	XojPage * getSourcePage();

	/**
	 * get the target page if not the same as the source page, if the selection is moved to a new page
	 */
	XojPage * getTargetPage();

	/**
	 * Get the X coordinate in View coordinates (absolute)
	 */
	int getXOnViewAbsolute();

	/**
	 * Get the Y coordinate in View coordinates (absolute)
	 */
	int getYOnViewAbsolute();

	/**
	 * Get the width in View coordinates
	 */
	int getViewWidth();

	/**
	 * Get the height in View coordinates
	 */
	int getViewHeight();

public:
	/**
	 * Sets the tool size for pen or eraser, returs an undo action
	 * (or NULL if nothing is done)
	 */
	UndoAction * setSize(ToolSize size, const double * thiknessPen, const double * thiknessHilighter, const double * thiknessEraser);

	/**
	 * Set the color of all elements, return an undo action
	 * (Or NULL if nothing done, e.g. because there is only an image)
	 */
	UndoAction * setColor(int color);

	/**
	 * Sets the font of all containing text elements, return an undo action
	 * (or NULL if there are no Text elements)
	 */
	UndoAction * setFont(XojFont & font);

public:
	/**
	 * Add an element to the this selection
	 */
	void addElement(Element * e);

	/**
	 * Returns all containig elements of this selections
	 */
	ListIterator<Element *> getElements();

	/**
	 * Finish the current movement
	 * (should be called in the mouse-button-released event handler)
	 */
	void mouseUp();

	/**
	 * Move the selection
	 */
	void moveSelection(double dx, double dy);

	/**
	 * Get the cursor type for the current position (if 0 then the default cursor should be used)
	 */
	CursorSelectionType getSelectionTypeForPos(double x, double y, double zoom);

	/**
	 * Paints the selection to cr, with the given zoom factor. The coordinates of cr
	 * should be relative to the provided view by getView() (use translateEvent())
	 */
	void paint(cairo_t * cr, double zoom);

	/**
	 * Callback to redrawing the buffer asynchrony
	 */
	static bool repaintSelection(EditSelection * selection);

	/**
	 * If the selection is outside the visible area correct the coordinates
	 */
	void ensureWithinVisibleArea();

public:
	/**
	 * Handles mouse input for moving and resizing, coordinates are relative to "view"
	 */
	void mouseDown(CursorSelectionType type, double x, double y);

	/**
	 * Handles mouse input for moving and resizing, coordinates are relative to "view"
	 */
	void mouseMove(double x, double y);

	/**
	 * If the selection should moved (or rescaled)
	 */
	bool isMoving();

public:
	PageView * getView();

private:
	/**
	 * Delete our internal View buffer,
	 * it will be recreated when the selection is painted next time
	 */
	void deleteViewBuffer();

	/**
	 * Draws an indicator where you can scale the selection
	 */
	void drawAnchorRect(cairo_t * cr, double x, double y, double zoom);

	/**
	 * Finishes all pending changes, move the elements, scale the elements and add
	 * them to new layer if any or to the old if no new layer
	 */
	void finalizeSelection();

	/**
	 * Translate all coordinates which are relative to the current view to the new view,
	 * and set the attribute view to the new view
	 */
	void translateToView(PageView * v);

private: // DATA
	XOJ_TYPE_ATTRIB;


	/**
	 * The position relative to the current view
	 */
	double x;
	double y;

	/**
	 * The size
	 */
	double width;
	double height;

	/**
	 * The original size to calculate the zoom factor for reascaling the items
	 */
	double originalWidth;
	double originalHeight;

	/**
	 * The new position
	 */
	double offsetX;
	double offsetY;

	/**
	 * The offset to the original selection
	 */
	int relativeX;
	int relativeY;

	/**
	 * Mouse coordinates for moving / resizing
	 */
	CursorSelectionType mouseDownType;
	double relMousePosX;
	double relMousePosY;

	/**
	 * If both scale axes should have the same scale factor, e.g. for Text
	 * (we cannot only set the font size for text)
	 */
	bool aspectRatio;

	/**
	 * The source page (form where the Elements come)
	 */
	XojPage * sourcePage;

	/**
	 * The source layer (form where the Elements come)
	 */
	Layer * sourceLayer;

	/**
	 * The selected element (the only one which are handled by this instance)
	 */
	GList * selected;

	/**
	 * The rendered elements
	 */
	cairo_surface_t * crBuffer;

	/**
	 * The source id for the rescaling task
	 */
	int rescaleId;

private: // HANDLER
	/**
	 * The page view for the anchor
	 */
	PageView * view;

	/**
	 * Undo redo handler
	 */
	UndoRedoHandler * undo;

};

#endif /* __EDITSELECTION_H__ */
