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

#include <deque>
#include <utility>
#include <vector>

#include "control/Tool.h"
#include "model/Element.h"
#include "model/Font.h"
#include "model/PageRef.h"
#include "view/ElementContainer.h"

#include "CursorSelectionType.h"
#include "XournalType.h"

class UndoRedoHandler;
class Layer;
class XojPageView;
class Selection;
class Element;
class UndoAction;
class EditSelectionContents;
class DeleteUndoAction;

class EditSelectionContents: public ElementContainer, public Serializeable {
public:
    EditSelectionContents(Rectangle<double> bounds, Rectangle<double> snappedBounds, const PageRef& sourcePage,
                          Layer* sourceLayer, XojPageView* sourceView);
    virtual ~EditSelectionContents();

public:
    /**
     * Sets the tool size for pen or eraser, returs an undo action
     * (or nullptr if nothing is done)
     */
    UndoAction* setSize(ToolSize size, const double* thicknessPen, const double* thicknessHilighter,
                        const double* thicknessEraser);

    /**
     * Set the color of all elements, return an undo action
     * (Or nullptr if nothing done, e.g. because there is only an image)
     */
    UndoAction* setColor(Color color);

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
     * @param orderInSourceLayer: specifies the index of the element from the source layer,
     * in case we want to replace it back where it came from.
     */
    void addElement(Element* e, Layer::ElementIndex order);

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
    void finalizeSelection(Rectangle<double> bounds, Rectangle<double> snappedBounds, bool aspectRatio, Layer* layer,
                           const PageRef& targetPage, XojPageView* targetView, UndoRedoHandler* undo);

    void updateContent(Rectangle<double> bounds, Rectangle<double> snappedBounds, double rotation, bool aspectRatio,
                       Layer* layer, const PageRef& targetPage, XojPageView* targetView, UndoRedoHandler* undo,
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
    XojPageView* getSourceView();


    /**
     * Gets the original X of the contents
     */
    double getOriginalX() const;

    /**
     * Gets the original Y of the contents
     */
    double getOriginalY() const;


    /**
     * Gets the original width of the contents
     */
    double getOriginalWidth() const;

    /**
     * Gets the original height of the contents
     */
    double getOriginalHeight() const;

    UndoAction* copySelection(PageRef page, XojPageView* view, double x, double y);

public:
    // Serialize interface
    void serialize(ObjectOutputStream& out);
    void readSerialized(ObjectInputStream& in);

private:
    /**
     * The original dimensions to calculate the zoom factor for reascaling the items and the offset for moving the
     * selection
     */
    Rectangle<double> originalBounds;
    Rectangle<double> lastBounds;
    Rectangle<double> lastSnappedBounds;

    /**
     * The given rotation. Original rotation should always be zero (double)
     */
    double rotation = 0;
    double lastRotation = 0;  // for undoing multiple rotations during one selection edit.

    /**
     * The offset to the original selection
     */
    double relativeX = -9999999999;
    double relativeY = -9999999999;

    /**
     * The setting for whether line width is restored after resizing operation (checked at creation time)
     */
    bool restoreLineWidth;

    /**
     * The selected element (the only one which are handled by this instance)
     */
    std::vector<Element*> selected;

    /**
     * Mapping of elements in the selection to the indexes from the original selection layer.
     * Defines a insert order over the selection.
     */
    std::deque<std::pair<Element*, Layer::ElementIndex>> insertOrder;

    /**
     * The rendered elements
     */
    cairo_surface_t* crBuffer = nullptr;

    /**
     * The source id for the rescaling task
     */
    int rescaleId = 0;

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
