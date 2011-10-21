/*
 * Xournal++
 *
 * The "Model" of a EditSelection
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */


#ifndef __EDITSELECTIONCONTENTS_H__
#define __EDITSELECTIONCONTENTS_H__

#include "../../view/ElementContainer.h"
#include <XournalType.h>
#include <glib.h>

#include "../Tool.h"
#include "../../model/Font.h"
#include "../../model/Element.h"
#include "../../model/PageRef.h"
#include <ListIterator.h>

class UndoRedoHandler;
class Layer;
class PageView;
class Selection;
class Element;
class UndoAction;
class EditSelectionContents;
class DeleteUndoAction;

class EditSelectionContents: public ElementContainer, public Serializeable {
public:
	EditSelectionContents(double x, double y, double width, double height, PageRef sourcePage, Layer * sourceLayer, PageView * sourceView);
	virtual ~EditSelectionContents();

public:
	/**
	 * Sets the tool size for pen or eraser, returs an undo action
	 * (or NULL if nothing is done)
	 */
	UndoAction * setSize(ToolSize size, const double * thicknessPen, const double * thicknessHilighter, const double * thicknessEraser);

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

	/**
	 * Fills de undo item if the selection is deleted
	 * the selection is cleared after
	 */
	void fillUndoItem(DeleteUndoAction * undo);

public:
	/**
	 * Add an element to the this selection
	 */
	void addElement(Element * e);

	/**
	 * Returns all containig elements of this selections
	 */
	ListIterator<Element *> getElements();

public:
	/**
	 * paints the selection
	 */
	void paint(cairo_t * cr, double x, double y, double width, double height, double zoom);

	/**
	 * Finish the editing
	 */
	void finalizeSelection(double x, double y, double width, double height, bool aspectRatio, Layer * layer, PageRef targetPage, PageView * targetView, UndoRedoHandler * undo);

private:
	/**
	 * Delete our internal View buffer,
	 * it will be recreated when the selection is painted next time
	 */
	void deleteViewBuffer();

	/**
	 * Callback to redrawing the buffer asynchrony
	 */
	static bool repaintSelection(EditSelectionContents * selection);

public:
	/**
	 * Gets the original width of the contents
	 */
	double getOriginalWidth();

	/**
	 * Gets the original height of the contents
	 */
	double getOriginalHeight();

public:
	// Serialize interface
	void serialize(ObjectOutputStream & out);
	void readSerialized(ObjectInputStream & in) throw (InputStreamException);

private:
	XOJ_TYPE_ATTRIB;

	/**
	 * The original size to calculate the zoom factor for reascaling the items
	 */
	double originalWidth;
	double originalHeight;

	/**
	 * The original position, to calculate the offset for moving the objects
	 */
	double originalX;
	double originalY;

	/**
	 * The offset to the original selection
	 */
	double relativeX;
	double relativeY;

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

	/**
	 * Source Page for Undo operations
	 */
	PageRef sourcePage;

	/**
	 * Source Layer for Undo operations
	 */
	Layer * sourceLayer;

	/**
	 * Source View for Undo operations
	 */
	PageView * sourceView;
};

#endif /* __EDITSELECTIONCONTENTS_H__ */
