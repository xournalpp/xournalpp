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

#include <string>
#include <vector>

#include "control/Tool.h"
#include "model/Font.h"
#include "model/PageRef.h"
#include "model/Snapping.h"
#include "view/ElementContainer.h"

#include "CursorSelectionType.h"
#include "SnapToGridInputHandler.h"
#include "XournalType.h"

class UndoRedoHandler;
class Layer;
class XojPageView;
class Selection;
class Element;
class UndoAction;
class EditSelectionContents;
class DeleteUndoAction;

class EditSelection: public ElementContainer, public Serializeable {
public:
    EditSelection(UndoRedoHandler* undo, const PageRef& page, XojPageView* view);
    EditSelection(UndoRedoHandler* undo, Selection* selection, XojPageView* view);
    EditSelection(UndoRedoHandler* undo, Element* e, XojPageView* view, const PageRef& page);
    EditSelection(UndoRedoHandler* undo, const vector<Element*>& elements, XojPageView* view, const PageRef& page);
    virtual ~EditSelection();

private:
    /**
     * Our internal constructor
     */
    void contstruct(UndoRedoHandler* undo, XojPageView* view, const PageRef& sourcePage);

    /**
     * Calculate the size from the element list
     */
    void calcSizeFromElements(vector<Element*> elements);

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
    Rectangle<double> getRect() const;

    /**
     * Get the source page (where the selection was done)
     */
    PageRef getSourcePage();

    /**
     * Get the source layer (form where the Elements come)
     */
    Layer* getSourceLayer();

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
     * Sets the tool size for pen or eraser, returns an undo action
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
     * @param order: specifies the index of the element from the source layer,
     * in case we want to replace it back where it came from.
     * 'InvalidLayerIndex' is a special value that says it has no source layer index (e.g, from clipboard)
     */
    void addElement(Element* e, Layer::ElementIndex order = Layer::InvalidElementIndex);

    /**
     * Returns all containig elements of this selections
     */
    vector<Element*>* getElements();

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
    void mouseMove(double x, double y, bool alt);

    /**
     * If the selection should moved (or rescaled)
     */
    bool isMoving();

    void copySelection();

public:
    XojPageView* getView();

public:
    // Serialize interface
    void serialize(ObjectOutputStream& out);
    void readSerialized(ObjectInputStream& in);

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

private:  // DATA
    /**
     * Support rotation
     */
    bool supportRotation = true;

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
     * The size
     */
    double width{};
    double height{};

    /**
     * The size and dimensions for snapping
     */
    Rectangle<double> snappedBounds{};


    /**
     * Mouse coordinates for moving / resizing
     */
    CursorSelectionType mouseDownType;
    double relMousePosX{};
    double relMousePosY{};
    double relMousePosRotX{};
    double relMousePosRotY{};

    /**
     * If both scale axes should have the same scale factor, e.g. for Text
     * (we cannot only set the font size for text)
     */
    bool aspectRatio{};

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
    EditSelectionContents* contents{};

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
};
