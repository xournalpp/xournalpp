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

#include <memory>   // for unique_ptr
#include <utility>  // for pair
#include <vector>   // for vector

#include <cairo.h>  // for cairo_surface_t, cairo_t

#include "control/ToolEnums.h"               // for ToolSize
#include "model/Element.h"                   // for Element::Index, Element
#include "model/ElementContainer.h"          // for ElementContainer
#include "model/ElementInsertionPosition.h"  // for InsertionOrder
#include "model/PageRef.h"                   // for PageRef
#include "undo/UndoAction.h"                 // for UndoAction (ptr only)
#include "util/Color.h"                      // for Color
#include "util/Rectangle.h"                  // for Rectangle
#include "util/serializing/Serializable.h"   // for Serializable

#include "CursorSelectionType.h"  // for CursorSelectionType

class UndoRedoHandler;
class Layer;
class XojPageView;
class DeleteUndoAction;
class LineStyle;
class ObjectInputStream;
class ObjectOutputStream;
class XojFont;

class EditSelectionContents: public ElementContainer, public Serializable {
public:
    EditSelectionContents(xoj::util::Rectangle<double> bounds, xoj::util::Rectangle<double> snappedBounds,
                          const PageRef& sourcePage, Layer* sourceLayer, XojPageView* sourceView);
    ~EditSelectionContents() override;

public:
    /**
     * Sets the line style for all strokes, returns an undo action
     * (or nullptr if nothing is done)
     */
    UndoActionPtr setLineStyle(LineStyle style);

    /**
     * Sets the tool size for pen or eraser, returns an undo action
     * (or nullptr if nothing is done)
     */
    UndoActionPtr setSize(ToolSize size, const double* thicknessPen, const double* thicknessHighlighter,
                          const double* thicknessEraser);

    /**
     * Set the color of all elements, return an undo action
     * (Or nullptr if nothing done, e.g. because there is only an image)
     */
    UndoActionPtr setColor(Color color);

    /**
     * Sets the font of all containing text elements, return an undo action
     * (or nullptr if there are no Text elements)
     */
    UndoActionPtr setFont(const XojFont& font);

    /**
     * Fills the undo item if the selection is deleted
     * the selection is cleared after
     */
    void fillUndoItem(DeleteUndoAction* undo);

    /**
     * Fills the stroke, return an undo action
     * (Or nullptr if nothing done, e.g. because there is only an image)
     */
    UndoActionPtr setFill(int alphaPen, int alphaHighligther);

public:
    /**
     * Add an element to the this selection
     * @param orderInSourceLayer: specifies the index of the element from the source layer,
     * in case we want to replace it back where it came from.
     */
    void addElement(ElementPtr e, Element::Index order);

    /**
     * Returns all containing elements of this selection
     */
    auto getElements() const -> std::vector<Element*> const&;

    void forEachElement(std::function<void(Element*)> f) const override;

    /**
     * Returns the insert order of this selection
     */
    auto getInsertionOrder() const -> const InsertionOrder&;

    /** replaces all elements by a new vector of elements
     * @param newElements: the elements which should replace the old elements
     * */
    void replaceInsertionOrder(InsertionOrder newInsertionOrder);

    /**
     * Returns InsertionOrder of this selection and clears it
     */
    auto stealInsertionOrder() -> InsertionOrder;

    /**
     * Creates an undo/redo item for translating by (dx, dy), and then updates the bounding boxes accordingly.
     */
    void addMoveUndo(UndoRedoHandler* undo, double dx, double dy);

public:
    /**
     * paints the selection
     */
    void paint(cairo_t* cr, double x, double y, double rotation, double width, double height, double zoom);

    /// Applies the transformation to the selected elements, empties the selection and return the modified elements
    InsertionOrder makeMoveEffective(const xoj::util::Rectangle<double>& bounds,
                                     const xoj::util::Rectangle<double>& snappedBounds, bool preserveAspectRatio);

    /**
     * Finish the editing
     */
    void finalizeSelection(xoj::util::Rectangle<double> bounds, xoj::util::Rectangle<double> snappedBounds,
                           bool aspectRatio, Layer* layer, const PageRef& targetPage, XojPageView* targetView,
                           UndoRedoHandler* undo);

    void updateContent(xoj::util::Rectangle<double> bounds, xoj::util::Rectangle<double> snappedBounds, double rotation,
                       bool aspectRatio, Layer* layer, const PageRef& targetPage, XojPageView* targetView,
                       UndoRedoHandler* undo, CursorSelectionType type);

private:
    /**
     * Delete our internal View buffer,
     * it will be recreated when the selection is painted next time
     */
    void deleteViewBuffer();

    /**
     * Callback to redrawing the buffer asynchrony
     */
    static auto repaintSelection(EditSelectionContents* selection) -> bool;

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
     * Gets the complete original bounding box as rectangle
     */
    auto getOriginalBounds() const -> xoj::util::Rectangle<double>;

public:
    // Serialize interface
    void serialize(ObjectOutputStream& out) const override;
    void readSerialized(ObjectInputStream& in) override;

private:
    /**
     * The original dimensions to calculate the zoom factor for rescaling the items and the offset for moving the
     * selection
     */
    xoj::util::Rectangle<double> originalBounds;
    xoj::util::Rectangle<double> lastBounds;
    xoj::util::Rectangle<double> lastSnappedBounds;

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
     *
     * Invariant: the insert order must be sorted by index in ascending order.
     */
    InsertionOrder insertionOrder;

    /**
     * The rendered elements
     */
    cairo_surface_t* crBuffer = nullptr;

    /**
     * The source id for the rescaling task
     */
    guint rescaleId = 0;

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
