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
#include "../../model/PageRef.h"
#include "CursorSelectionType.h"

#include <XournalType.h>

class UndoRedoHandler;
class Layer;
class PageView;
class Selection;
class Element;
class UndoAction;
class EditSelectionContents;
class DeleteUndoAction;

class EditSelection: public ElementContainer, public Serializeable
{
public:
	EditSelection(UndoRedoHandler* undo, PageRef page, PageView* view);
	EditSelection(UndoRedoHandler* undo, Selection* selection, PageView* view);
	EditSelection(UndoRedoHandler* undo, Element* e, PageView* view, PageRef page);
	virtual ~EditSelection();

private:
	/**
	 * Our internal constructor
	 */
	void contstruct(UndoRedoHandler* undo, PageView* view, PageRef sourcePage);

public:
	/**
	 * get the X coordinate relative to the provided view (getView())
	 * in document coordinates
	 */
	double getXOnView();

	/**
	 * Get the Y coordinate relative to the provided view (getView())
	 * in document coordinates
	 */
	double getYOnView();

	/**
	 * Get the width in document coordinates (multiple with zoom)
	 */
	double getWidth();

	/**
	 * Get the height in document coordinates (multiple with zoom)
	 */
	double getHeight();

	/**
	 * Get the source page (where the selection was done)
	 */
	PageRef getSourcePage();

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
	UndoAction* setSize(ToolSize size, const double* thicknessPen,
	                    const double* thicknessHilighter, const double* thicknessEraser);

	/**
	 * Set the color of all elements, return an undo action
	 * (Or NULL if nothing done, e.g. because there is only an image)
	 */
	UndoAction* setColor(int color);

	/**
	 * Sets the font of all containing text elements, return an undo action
	 * (or NULL if there are no Text elements)
	 */
	UndoAction* setFont(XojFont& font);

	/**
	 * Fills de undo item if the selection is deleted
	 * the selection is cleared after
	 */
	void fillUndoItem(DeleteUndoAction* undo);

public:
	/**
	 * Add an element to the this selection
	 */
	void addElement(Element* e);

	/**
	 * Returns all containig elements of this selections
	 */
	ListIterator<Element*> getElements();

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
	void paint(cairo_t* cr, double zoom);

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
	PageView* getView();

public:
	// Serialize interface
	void serialize(ObjectOutputStream& out);
	void readSerialized(ObjectInputStream& in) throw (InputStreamException);

private:

	/**
	 * Draws an indicator where you can scale the selection
	 */
	void drawAnchorRect(cairo_t* cr, double x, double y, double zoom);

	/**
	 * Finishes all pending changes, move the elements, scale the elements and add
	 * them to new layer if any or to the old if no new layer
	 */
	void finalizeSelection();

	/**
	 * Gets the PageView where the selection is located on
	 */
	PageView* getBestMatchingPageView();

	/**
	 * Translate all coordinates which are relative to the current view to the new view,
	 * and set the attribute view to the new view
	 */
	void translateToView(PageView* v);

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
	PageRef sourcePage;

	/**
	 * The source layer (form where the Elements come)
	 */
	Layer* sourceLayer;

	/**
	 * The contents of the selection
	 */
	EditSelectionContents* contents;

private: // HANDLER
	/**
	 * The page view for the anchor
	 */
	PageView* view;

	/**
	 * Undo redo handler
	 */
	UndoRedoHandler* undo;

};

#endif /* __EDITSELECTION_H__ */
