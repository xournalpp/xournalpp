/*
 * Xournal++
 *
 * A selection for editing, every selection (Rect, Lasso...) is
 * converted to this one if the selection is finished
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <memory>  // for unique_ptr
#include <utility>  // for pair
#include <vector>   // for vector

#include <cairo.h>  // for cairo_t, cairo_matrix_t
#include <glib.h>   // for GSource

#include "control/ToolEnums.h"               // for ToolSize
#include "model/Element.h"                   // for Element, Element::Index
#include "model/ElementContainer.h"          // for ElementContainer
#include "model/ElementInsertionPosition.h"  // for InsertionOrder
#include "model/PageRef.h"                   // for PageRef
#include "undo/UndoAction.h"                 // for UndoAction (ptr only)
#include "util/Color.h"                      // for Color
#include "util/Rectangle.h"                  // for Rectangle
#include "util/serializing/Serializable.h"   // for Serializable

#include "CursorSelectionType.h"     // for CursorSelectionType, CURS...
#include "SnapToGridInputHandler.h"  // for SnapToGridInputHandler

class UndoRedoHandler;
class Layer;
class XojPageView;
class Selection;
class EditSelectionContents;
class DeleteUndoAction;
class LineStyle;
class ObjectInputStream;
class ObjectOutputStream;
class XojFont;
class Document;
class EditSelection;

namespace SelectionFactory {
auto createFromFloatingElement(Control* ctrl, const PageRef& page, Layer* layer, XojPageView* view, ElementPtr e)
        -> std::unique_ptr<EditSelection>;
auto createFromFloatingElements(Control* ctrl, const PageRef& page, Layer* layer, XojPageView* view,
                                InsertionOrder elts)  //
        -> std::pair<std::unique_ptr<EditSelection>, Range>;
auto createFromElementOnActiveLayer(Control* ctrl, const PageRef& page, XojPageView* view, Element* e,
                                    Element::Index pos = Element::InvalidIndex)  //
        -> std::unique_ptr<EditSelection>;
auto createFromElementsOnActiveLayer(Control* ctrl, const PageRef& page, XojPageView* view, InsertionOrderRef elts)
        -> std::unique_ptr<EditSelection>;
/**
 * @brief Creates a new instance containing base->getElements() and *e. The content of *base is cleared but *base is not
 * destroyed.
 */
auto addElementFromActiveLayer(Control* ctrl, EditSelection* base, Element* e, Element::Index pos)
        -> std::unique_ptr<EditSelection>;
/**
 * @brief Creates a new instance containing base->getElements() and the content of elts. The content of *base is cleared
 * but *base is not destroyed.
 */
auto addElementsFromActiveLayer(Control* ctrl, EditSelection* base, const InsertionOrderRef& elts)
        -> std::unique_ptr<EditSelection>;
};  // namespace SelectionFactory

class EditSelection: public ElementContainer, public Serializable {
public:
    EditSelection(Control* ctrl, InsertionOrder elts, const PageRef& page, Layer* layer, XojPageView* view,
                  const Range& bounds, const Range& snappingBounds);

    /// Construct an empty selection
    EditSelection(Control* ctrl, const PageRef& page, Layer* layer, XojPageView* view);

    ~EditSelection() override;

public:
    /**
     * get the X coordinate relative to the provided view (getView())
     * in document coordinates
     */
    double getXOnView() const;

    /**
     * Get the Y coordinate relative to the provided view (getView())
     * in document coordinates
     */
    double getYOnView() const;

    /**
     * @return The original X coordinates of the provided view in document
     * coordinates.
     */
    double getOriginalXOnView();

    /**
     * @return The original Y coordinates of the provided view in document
     * coordinates.
     */
    double getOriginalYOnView();

    /**
     * Get the width in document coordinates (multiple with zoom)
     */
    double getWidth() const;

    /**
     * Get the height in document coordinates (multiple with zoom)
     */
    double getHeight() const;

    /**
     * Get the bounding rectangle in document coordinates (multiple with zoom)
     */
    xoj::util::Rectangle<double> getRect() const;

    /**
     * gets the minimal bounding box containing all elements of the selection used for e.g. grid snapping
     */
    xoj::util::Rectangle<double> getSnappedBounds() const;

    /**
     * get the original bounding rectangle in document coordinates
     */
    xoj::util::Rectangle<double> getOriginalBounds() const;

    /**
     * Get the rotation angle of the selection
     */
    double getRotation() const;

    /**
     * Get if the selection supports being rotated
     */
    bool isRotationSupported() const;

    /**
     * Get the source page (where the selection was done)
     */
    PageRef getSourcePage() const;

    /**
     * Get the source layer (form where the Elements come)
     */
    Layer* getSourceLayer() const;

    /**
     * Get the X coordinate in View coordinates (absolute)
     */
    int getXOnViewAbsolute() const;

    /**
     * Get the Y coordinate in View coordinates (absolute)
     */
    int getYOnViewAbsolute() const;

    inline XojPageView* getView() const { return view; }

public:
    /**
     * Sets the tool size for pen or eraser, returns an undo action
     * (or nullptr if nothing is done)
     */
    UndoActionPtr setSize(ToolSize size, const double* thicknessPen, const double* thicknessHighlighter,
                          const double* thicknessEraser);

    /**
     * Set the line style of all strokes, return an undo action
     * (Or nullptr if nothing done)
     */
    UndoActionPtr setLineStyle(LineStyle style);

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
     * @param pos: specifies the index of the element from the source layer,
     * in case we want to replace it back where it came from.
     */
    void addElement(ElementPtr e, Element::Index pos);

    /**
     * Returns all containing elements of this selection
     */
    auto getElements() const -> std::vector<Element*> const&;

    void forEachElement(std::function<void(Element*)> f) const override;

    /**
     * Returns the insert order of this selection
     */
    auto getInsertionOrder() const -> InsertionOrder const&;

    enum class OrderChange {
        BringToFront,
        BringForward,
        SendBackward,
        SendToBack,
    };

    /**
     * Change the insert order of this selection.
     */
    auto rearrangeInsertionOrder(const OrderChange change) -> UndoActionPtr;

    /**
     * Finish the current movement
     * (should be called in the mouse-button-released event handler)
     */
    void mouseUp();

    /**
     * Move the selection
     */
    void moveSelection(double dx, double dy, bool addMoveUndo = false);

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
     * Gets the selection's bounding box in view coordinates. This takes document zoom
     * and selection rotation into account.
     */
    auto getBoundingBoxInView() const -> xoj::util::Rectangle<double>;

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
    void mouseMove(double x, double y, bool alt);

    /**
     * If the user is currently moving the selection.
     */
    bool isMoving();

    void copySelection();

public:
    XojPageView* getView();

public:
    // Serialize interface
    void serialize(ObjectOutputStream& out) const override;
    void readSerialized(ObjectInputStream& in) override;


    /// Applies the transformation to the selected elements, empties the selection and return the modified elements
    InsertionOrder makeMoveEffective();

private:
    /**
     * Draws an indicator where you can scale the selection
     */
    void drawAnchorRect(cairo_t* cr, double x, double y, double zoom);

    /**
     * Draws an indicator where you can rotate the selection
     */
    void drawAnchorRotation(cairo_t* cr, double x, double y, double zoom);


    /**
     * Draws an indicator where you can delete the selection
     */
    void drawDeleteRect(cairo_t* cr, double x, double y, double zoom) const;


    /**
     * Finishes all pending changes, move the elements, scale the elements and add
     * them to new layer if any or to the old if no new layer
     */
    void finalizeSelection();

    /**
     * Gets the PageView under the cursor
     */
    XojPageView* getPageViewUnderCursor();

    /**
     * Translate all coordinates which are relative to the current view to the new view,
     * and set the attribute view to the new view
     */
    void translateToView(XojPageView* v);

    /**
     * Updates rotation matrix
     */
    void updateMatrix();

    /**
     * scales and shifts to update bounding boxes
     */
    void scaleShift(double fx, double fy, bool changeLeft, bool changeTop);

    /**
     * Set edge panning signal.
     */
    void setEdgePan(bool edgePan);

    /**
     * Whether the edge pan signal is set.
     */
    bool isEdgePanning() const;

    static bool handleEdgePan(EditSelection* self);

private:  // DATA
    /**
     * The position (and rotation) relative to the current view
     */
    double x{};
    double y{};
    double rotation = 0;

    /**
     * Use to translate to rotated selection
     */
    cairo_matrix_t cmatrix{};

    /**
     * The size, including the padding and frame
     */
    double width{};
    double height{};

    /**
     * The size and dimensions for snapping
     */
    xoj::util::Rectangle<double> snappedBounds{};


    /**
     * Mouse coordinates for moving / resizing
     */
    CursorSelectionType mouseDownType = CURSOR_SELECTION_NONE;
    double relMousePosX{};
    double relMousePosY{};
    double relMousePosRotX{};
    double relMousePosRotY{};

    /**
     * If both scale axes should have the same scale factor, e.g. for Text
     * (we can only set the font size for text)
     */
    bool preserveAspectRatio = false;

    /**
     * If mirrors are allowed e.g. for strokes
     */
    bool supportMirroring = true;

    /**
     * Support rotation
     */
    bool supportRotation = true;

    /**
     * Size of the editing handles
     */
    int btnWidth{8};

    /**
     * The source page (form where the Elements come)
     */
    PageRef sourcePage;

    /**
     * The source layer (form where the Elements come)
     */
    Layer* sourceLayer{};

    /**
     * The contents of the selection
     */
    std::unique_ptr<EditSelectionContents> contents;

private:  // HANDLER
    /**
     * The page view for the anchor
     */
    XojPageView* view{};

    /**
     * Undo redo handler
     */
    UndoRedoHandler* undo{};

    /**
     * The handler for snapping points
     */
    SnapToGridInputHandler snappingHandler;

    /**
     * Edge pan timer
     */
    GSource* edgePanHandler = nullptr;

    /**
     * Inhibit the next move event after edge panning finishes. This prevents
     * the selection from teleporting if the page has changed during panning.
     * Additionally, this reduces the amount of "jitter" resulting from moving
     * the selection in mouseDown while edge panning.
     */
    bool edgePanInhibitNext = false;
};
