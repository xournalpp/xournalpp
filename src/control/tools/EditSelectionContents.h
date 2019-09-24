/*
 * Xournal++
 *
 * The "Model" of a EditSelection
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "CursorSelectionType.h"

#include "control/Tool.h"
#include "model/Element.h"
#include "model/Font.h"
#include "model/PageRef.h"
#include "view/ElementContainer.h"

#include <XournalType.h>

class UndoRedoHandler;
class Layer;
class XojPageView;
class Selection;
class Element;
class UndoAction;
class EditSelectionContents;
class DeleteUndoAction;

class EditSelectionContents : public ElementContainer, public Serializeable
{
public:
	EditSelectionContents(double x, double y, double width, double height,
						  PageRef sourcePage, Layer* sourceLayer, XojPageView* sourceView);
	virtual ~EditSelectionContents();

public:
	/**
	 * Sets the tool size for pen or eraser, returs an undo action
	 * (or nullptr if nothing is done)
	 */
	UndoAction* setSize(ToolSize size, const double* thicknessPen, const double* thicknessHilighter, const double* thicknessEraser);

	/**
	 * Set the color of all elements, return an undo action
	 * (Or nullptr if nothing done, e.g. because there is only an image)
	 */
	UndoAction* setColor(int color);

	/**
	 * Sets the font of all containing text elements, return an undo action
	 * (or nullptr if there are no Text elements)
	 */
	UndoAction* setFont(XojFont& font);

	/**
	 * Fills the undo item if the selection is deleted
	 * the selection is cleared after
	 */
	void fillUndoItem(DeleteUndoAction* undo);

	/**
	 * Fills the stroke, return an undo action
	 * (Or nullptr if nothing done, e.g. because there is only an image)
	 */
	UndoAction* setFill(int alphaPen, int alphaHighligther);

public:
	/**
	 * Add an element to the this selection
	 */
	void addElement(Element* e);

	/**
	 * Returns all containig elements of this selections
	 */
	vector<Element*>* getElements();

public:
	/**
	 * paints the selection
	 */
	void paint(cairo_t* cr, double x, double y, double rotation, double width, double height, double zoom);

	/**
	 * Finish the editing
	 */
	void finalizeSelection(double x, double y, double width, double height, bool aspectRatio,
						   Layer* layer, PageRef targetPage, XojPageView* targetView, UndoRedoHandler* undo);

	void updateContent(double x, double y, double rotation, double width, double height, bool aspectRatio,
					   Layer* layer, PageRef targetPage, XojPageView* targetView, UndoRedoHandler* undo,
					   CursorSelectionType type);

private:
	/**
	 * Delete our internal View buffer,
	 * it will be recreated when the selection is painted next time
	 */
	void deleteViewBuffer();

	/**
	 * Callback to redrawing the buffer asynchrony
	 */
	static bool repaintSelection(EditSelectionContents* selection);

public:
	
	/**
	 * Gets the original view of the contents
	 */
	XojPageView * getSourceView();
	
	
	/**
	 * Gets the original X of the contents
	 */
	double getOriginalX();

	/**
	 * Gets the original Y of the contents
	 */
	double getOriginalY();

	
	/**
	 * Gets the original width of the contents
	 */
	double getOriginalWidth();

	/**
	 * Gets the original height of the contents
	 */
	double getOriginalHeight();

	UndoAction* copySelection(PageRef page, XojPageView *view, double x, double y);

public:
	// Serialize interface
	void serialize(ObjectOutputStream& out);
	void readSerialized(ObjectInputStream& in);

private:
	/**
	 * The original size to calculate the zoom factor for reascaling the items
	 */
	double originalWidth, originalHeight;
	double lastWidth, lastHeight;

	/**
	 * The original position, to calculate the offset for moving the objects
	 */
	double originalX, originalY;
	double lastX, lastY;

	/**
	 * The given rotation. Original rotation should always be zero (double)
	 */
	double rotation = 0;
	double lastRotation = 0; // for undoing multiple rotations during one selection edit.

	/**
	 * The offset to the original selection
	 */
	double relativeX;
	double relativeY;

	/**
	 * The selected element (the only one which are handled by this instance)
	 */
	vector<Element*> selected;

	/**
	 * The rendered elements
	 */
	cairo_surface_t* crBuffer;

	/**
	 * The source id for the rescaling task
	 */
	int rescaleId;

	/**
	 * Source Page for Undo operations
	 */
	PageRef sourcePage;

	/**
	 * Source Layer for Undo operations
	 */
	Layer* sourceLayer;

	/**
	 * Source View for Undo operations
	 */
	XojPageView* sourceView;
};
