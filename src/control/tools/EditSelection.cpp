#include "EditSelection.h"

#include <cmath>

#include "control/Control.h"
#include "gui/Layout.h"
#include "gui/PageView.h"
#include "gui/XournalView.h"
#include "gui/XournalppCursor.h"
#include "model/Document.h"
#include "model/Element.h"
#include "model/Layer.h"
#include "model/Stroke.h"
#include "model/Text.h"
#include "serializing/ObjectInputStream.h"
#include "serializing/ObjectOutputStream.h"
#include "undo/ColorUndoAction.h"
#include "undo/FontUndoAction.h"
#include "undo/SizeUndoAction.h"
#include "undo/UndoRedoHandler.h"

#include "EditSelectionContents.h"
#include "Selection.h"


constexpr size_t MINPIXSIZE = 5;  // smallest can scale down to, in pixels.

EditSelection::EditSelection(UndoRedoHandler* undo, const PageRef& page, XojPageView* view):
        snappingHandler(view->getXournal()->getControl()->getSettings()),
        x(0),
        y(0),
        rotation(0),
        width(0),
        height(0),
        snappedBounds(Rectangle<double>{}) {
    contstruct(undo, view, page);
}

EditSelection::EditSelection(UndoRedoHandler* undo, Selection* selection, XojPageView* view):
        snappingHandler(view->getXournal()->getControl()->getSettings()) {
    calcSizeFromElements(selection->selectedElements);

    contstruct(undo, view, view->getPage());

    for (Element* e: selection->selectedElements) {
        addElement(e, this->sourceLayer->indexOf(e));
        this->sourceLayer->removeElement(e, false);
    }

    view->rerenderPage();
}

EditSelection::EditSelection(UndoRedoHandler* undo, Element* e, XojPageView* view, const PageRef& page):
        snappingHandler(view->getXournal()->getControl()->getSettings()) {
    calcSizeFromElements(std::vector<Element*>{e});
    contstruct(undo, view, page);

    addElement(e, this->sourceLayer->indexOf(e));
    this->sourceLayer->removeElement(e, false);

    view->rerenderElement(e);
}

EditSelection::EditSelection(UndoRedoHandler* undo, const vector<Element*>& elements, XojPageView* view,
                             const PageRef& page):
        snappingHandler(view->getXournal()->getControl()->getSettings()) {
    calcSizeFromElements(elements);
    contstruct(undo, view, page);

    for (Element* e: elements) {
        addElement(e, this->sourceLayer->indexOf(e));
        this->sourceLayer->removeElement(e, false);
    }

    view->rerenderPage();
}

void EditSelection::calcSizeFromElements(vector<Element*> elements) {
    if (elements.empty()) {
        x = 0;
        y = 0;
        width = 0;
        height = 0;
        snappedBounds = Rectangle<double>{};
        return;
    }

    Element* first = elements.front();
    Range range(first->getX(), first->getY());
    Rectangle<double> rect = first->getSnappedBounds();

    for (Element* e: elements) {
        range.addPoint(e->getX(), e->getY());
        range.addPoint(e->getX() + e->getElementWidth(), e->getY() + e->getElementHeight());
        rect.unite(e->getSnappedBounds());  // size has already been calculated, so skip it
    }

    // make the visible bounding box large enough so that anchors do not colapse even for horizontal/vertical strokes
    // stroke
    x = range.getX() - 1.5 * this->btnWidth;
    y = range.getY() - 1.5 * this->btnWidth;
    width = range.getWidth() + 3 * this->btnWidth;
    height = range.getHeight() + 3 * this->btnWidth;

    snappedBounds = rect;
}

void EditSelection::contstruct(UndoRedoHandler* undo, XojPageView* view, const PageRef& sourcePage) {
    this->view = view;
    this->undo = undo;
    this->sourcePage = sourcePage;
    this->sourceLayer = this->sourcePage->getSelectedLayer();

    this->aspectRatio = false;

    this->relMousePosX = 0;
    this->relMousePosY = 0;
    this->relMousePosRotX = 0;
    this->relMousePosRotY = 0;
    this->mouseDownType = CURSOR_SELECTION_NONE;


    int dpi = this->view->getXournal()->getControl()->getSettings()->getDisplayDpi();
    this->btnWidth = std::max(10, dpi / 8);

    this->contents = new EditSelectionContents(this->getRect(), this->snappedBounds, this->sourcePage,
                                               this->sourceLayer, this->view);

    cairo_matrix_init_identity(&this->cmatrix);
    this->view->getXournal()->getCursor()->setRotationAngle(0);
}

EditSelection::~EditSelection() {
    finalizeSelection();

    this->sourcePage = nullptr;
    this->sourceLayer = nullptr;

    delete this->contents;
    this->contents = nullptr;

    this->view = nullptr;
    this->undo = nullptr;
}

/**
 * Finishes all pending changes, move the elements, scale the elements and add
 * them to new layer if any or to the old if no new layer
 */
void EditSelection::finalizeSelection() {
    XojPageView* v = getPageViewUnderCursor();
    if (v == nullptr) {  // Not on any page - move back to original page and position
        double ox = this->snappedBounds.x - this->x;
        double oy = this->snappedBounds.y - this->y;
        this->x = this->contents->getOriginalX();
        this->y = this->contents->getOriginalY();
        this->snappedBounds.x = this->x + ox;
        this->snappedBounds.y = this->y + oy;
        v = this->contents->getSourceView();
    }


    this->view = v;

    PageRef page = this->view->getPage();
    Layer* layer = page->getSelectedLayer();
    this->contents->finalizeSelection(this->getRect(), this->snappedBounds, this->aspectRatio, layer, page, this->view,
                                      this->undo);


    // Calculate new clip region delta due to rotation:
    double addW =
            std::abs(this->width * cos(this->rotation)) + std::abs(this->height * sin(this->rotation)) - this->width;
    double addH =
            std::abs(this->width * sin(this->rotation)) + std::abs(this->height * cos(this->rotation)) - this->height;


    this->view->rerenderRect(this->x - addW / 2.0, this->y - addH / 2.0, this->width + addW, this->height + addH);

    // This is needed if the selection not was 100% on a page
    this->view->getXournal()->repaintSelection(true);
}

/**
 * get the X coordinate relative to the provided view (getView())
 * in document coordinates
 */
auto EditSelection::getXOnView() const -> double { return this->x; }

/**
 * get the Y coordinate relative to the provided view (getView())
 * in document coordinates
 */
auto EditSelection::getYOnView() const -> double { return this->y; }

auto EditSelection::getOriginalXOnView() -> double { return this->contents->getOriginalX(); }

auto EditSelection::getOriginalYOnView() -> double { return this->contents->getOriginalY(); }

/**
 * get the width in document coordinates (multiple with zoom)
 */
auto EditSelection::getWidth() const -> double { return this->width; }

/**
 * get the height in document coordinates (multiple with zoom)
 */
auto EditSelection::getHeight() const -> double { return this->height; }

/**
 * get the bounding rectangle in document coordinates (multiple with zoom)
 */
auto EditSelection::getRect() const -> Rectangle<double> {
    return Rectangle<double>{this->x, this->y, this->width, this->height};
}

/**
 * Get the source page (where the selection was done)
 */
auto EditSelection::getSourcePage() -> PageRef { return this->sourcePage; }

/**
 * Get the source layer (form where the Elements come)
 */
auto EditSelection::getSourceLayer() -> Layer* { return this->sourceLayer; }

/**
 * Get the X coordinate in View coordinates (absolute)
 */
auto EditSelection::getXOnViewAbsolute() -> int {
    double zoom = view->getXournal()->getZoom();
    return this->view->getX() + this->getXOnView() * zoom;
}

/**
 * Get the Y coordinate in View coordinates (absolute)
 */
auto EditSelection::getYOnViewAbsolute() -> int {
    double zoom = view->getXournal()->getZoom();
    return this->view->getY() + this->getYOnView() * zoom;
}

/**
 * Get the width in View coordinates
 */
auto EditSelection::getViewWidth() -> int {
    double zoom = view->getXournal()->getZoom();
    return this->width * zoom;
}

/**
 * Get the height in View coordinates
 */
auto EditSelection::getViewHeight() -> int {
    double zoom = view->getXournal()->getZoom();
    return this->height * zoom;
}

/**
 * Sets the tool size for pen or eraser, returs an undo action
 * (or nullptr if nothing is done)
 */
auto EditSelection::setSize(ToolSize size, const double* thicknessPen, const double* thicknessHilighter,
                            const double* thicknessEraser) -> UndoAction* {
    return this->contents->setSize(size, thicknessPen, thicknessHilighter, thicknessEraser);
}

/**
 * Fills the stroke, return an undo action
 * (Or nullptr if nothing done, e.g. because there is only an image)
 */
auto EditSelection::setFill(int alphaPen, int alphaHighligther) -> UndoAction* {
    return this->contents->setFill(alphaPen, alphaHighligther);
}

/**
 * Set the color of all elements, return an undo action
 * (Or nullptr if nothing done, e.g. because there is only an image)
 */
auto EditSelection::setColor(Color color) -> UndoAction* { return this->contents->setColor(color); }

/**
 * Sets the font of all containing text elements, return an undo action
 * (or nullptr if there are no Text elements)
 */
auto EditSelection::setFont(XojFont& font) -> UndoAction* { return this->contents->setFont(font); }

/**
 * Fills de undo item if the selection is deleted
 * the selection is cleared after
 */
void EditSelection::fillUndoItem(DeleteUndoAction* undo) { this->contents->fillUndoItem(undo); }

/**
 * Add an element to this selection
 *
 */
void EditSelection::addElement(Element* e, Layer::ElementIndex order) {
    this->contents->addElement(e, order);

    if (e->rescaleOnlyAspectRatio()) {
        this->aspectRatio = true;
    }

    if (e->getType() != ELEMENT_STROKE) {
        // Currently only stroke supports rotation
        supportRotation = false;
    }
}

/**
 * Returns all containig elements of this selections
 */
auto EditSelection::getElements() -> vector<Element*>* { return this->contents->getElements(); }

/**
 * Finish the current movement
 * (should be called in the mouse-button-released event handler)
 */
void EditSelection::mouseUp() {
    if (this->mouseDownType == CURSOR_SELECTION_DELETE) {
        this->view->getXournal()->deleteSelection();
        return;
    }


    PageRef page = this->view->getPage();
    Layer* layer = page->getSelectedLayer();
    this->rotation = snappingHandler.snapAngle(this->rotation, false);

    this->contents->updateContent(this->getRect(), this->snappedBounds, this->rotation, this->aspectRatio, layer, page,
                                  this->view, this->undo, this->mouseDownType);

    this->mouseDownType = CURSOR_SELECTION_NONE;
    updateMatrix();
}

/**
 * Handles mouse input for moving and resizing, coordinates are relative to "view"
 */
void EditSelection::mouseDown(CursorSelectionType type, double x, double y) {
    double zoom = this->view->getXournal()->getZoom();

    this->mouseDownType = type;

    // coordinates relative to top left corner of snapped bounds in coordinate system which is not modified
    this->relMousePosX = x / zoom - this->snappedBounds.x;
    this->relMousePosY = y / zoom - this->snappedBounds.y;

    // coordinates relative to top left corner of snapped bounds in coordinate system which is rotated to make bounding
    // box edges horizontal/vertical
    cairo_matrix_transform_point(&this->cmatrix, &x, &y);
    this->relMousePosRotX = x / zoom - this->snappedBounds.x;
    this->relMousePosRotY = y / zoom - this->snappedBounds.y;
}

/**
 * Handles mouse input for moving and resizing, coordinates are relative to "view"
 */
void EditSelection::mouseMove(double mouseX, double mouseY, bool alt) {
    double zoom = this->view->getXournal()->getZoom();

    if (this->mouseDownType == CURSOR_SELECTION_MOVE) {
        // compute translation (without snapping)
        double dx = mouseX / zoom - this->snappedBounds.x - this->relMousePosX;
        double dy = mouseY / zoom - this->snappedBounds.y - this->relMousePosY;

        // find corner of reduced bounding box in rotated coordinate system closest to grabbing position
        double cx = this->snappedBounds.x;
        double cy = this->snappedBounds.y;
        if (this->relMousePosRotX > this->snappedBounds.width / 2) {  // closer to the right side
            cx += this->snappedBounds.width;
        }
        if (this->relMousePosRotY > this->snappedBounds.height / 2) {  // closer to the lower side
            cy += this->snappedBounds.height;
        }

        // compute corner of reduced bounding box in unmodified coordinate system closest to grabbing position
        cairo_matrix_t inv = this->cmatrix;
        cairo_matrix_invert(&inv);
        cx *= zoom;
        cy *= zoom;
        cairo_matrix_transform_point(&inv, &cx, &cy);
        cx /= zoom;
        cy /= zoom;

        // compute position where unsnapped corner would move
        Point p = Point(cx + dx, cy + dy);

        // snap this corner
        p = snappingHandler.snapToGrid(p, alt);

        // move
        moveSelection(p.x - cx, p.y - cy);
    } else if (this->mouseDownType == CURSOR_SELECTION_ROTATE && supportRotation)  // catch rotation here
    {
        double rdx = mouseX / zoom - this->snappedBounds.x - this->snappedBounds.width / 2;
        double rdy = mouseY / zoom - this->snappedBounds.y - this->snappedBounds.height / 2;

        double angle = atan2(rdy, rdx);
        this->rotation = angle;
        this->view->getXournal()->getCursor()->setRotationAngle(180 / M_PI * angle);
    } else {
        // Translate mouse position into rotated coordinate system:
        double rx = mouseX;
        double ry = mouseY;
        cairo_matrix_transform_point(&this->cmatrix, &rx, &ry);
        rx /= zoom;
        ry /= zoom;

        double xOffset = rx - this->x;  // x-offset from corner/side that is used for resizing
        double yOffset = ry - this->y;  // y-offset from corner/side that is used for resizing
        // change if corner is not top left;

        double minSize = MINPIXSIZE / zoom;
        double f = NAN;

        // in each case first scale without snapping consideration then snap
        // take care that wSnap and hSnap are not too small
        if (this->mouseDownType == CURSOR_SELECTION_TOP_LEFT) {
            f = sqrt(abs((this->height - yOffset) / this->height * (this->width - xOffset) / this->width));
            f = std::max({f, minSize / this->width, minSize / this->height});
            scaleShift(f, f, true, true);

            double snappedX = snappingHandler.snapHorizontally(this->snappedBounds.x, alt);
            double snappedY = snappingHandler.snapVertically(this->snappedBounds.y, alt);
            double dx = snappedX - this->snappedBounds.x;
            double dy = snappedY - this->snappedBounds.y;
            double fx = (this->snappedBounds.width > minSize) ?
                                (this->snappedBounds.width - dx) / this->snappedBounds.width :
                                1;
            double fy = (this->snappedBounds.height > minSize) ?
                                (this->snappedBounds.height - dy) / this->snappedBounds.height :
                                1;
            f = ((std::abs(dx) < std::abs(dy)) && (fx != 1) || fy == 1) ? fx : fy;
            f = (this->width * f < minSize || this->height * f < minSize) ? 1 : f;
            scaleShift(f, f, true, true);
        } else if (this->mouseDownType == CURSOR_SELECTION_TOP_RIGHT) {
            xOffset -= this->width;
            f = sqrt(abs((this->height - yOffset) / this->height * (this->width + xOffset) / this->width));
            f = std::max({f, minSize / this->width, minSize / this->height});
            this->scaleShift(f, f, false, true);

            double snappedX = snappingHandler.snapHorizontally(this->snappedBounds.x + this->snappedBounds.width, alt);
            double snappedY = snappingHandler.snapVertically(this->snappedBounds.y, alt);
            double dx = snappedX - this->snappedBounds.x - this->snappedBounds.width;
            double dy = snappedY - this->snappedBounds.y;
            double fx = (this->snappedBounds.width > minSize) ?
                                (this->snappedBounds.width + dx) / this->snappedBounds.width :
                                1;
            double fy = (this->snappedBounds.height > minSize) ?
                                (this->snappedBounds.height - dy) / this->snappedBounds.height :
                                1;
            f = ((std::abs(dx) < std::abs(dy)) && (fx != 1) || fy == 1) ? fx : fy;
            f = (this->width * f < minSize || this->height * f < minSize) ? 1 : f;
            this->scaleShift(f, f, false, true);
        } else if (this->mouseDownType == CURSOR_SELECTION_BOTTOM_LEFT) {
            yOffset -= this->height;
            f = sqrt(abs((this->height + yOffset) / this->height * (this->width - xOffset) / this->width));
            f = std::max({f, minSize / this->width, minSize / this->height});
            this->scaleShift(f, f, true, false);

            double snappedX = snappingHandler.snapHorizontally(this->snappedBounds.x, alt);
            double snappedY = snappingHandler.snapVertically(this->snappedBounds.y + this->snappedBounds.height, alt);
            double dx = snappedX - this->snappedBounds.width;
            double dy = snappedY - this->snappedBounds.y - this->snappedBounds.height;
            double fx = (this->snappedBounds.width > minSize) ?
                                (this->snappedBounds.width - dx) / this->snappedBounds.width :
                                1;
            double fy = (this->snappedBounds.height > minSize) ?
                                (this->snappedBounds.height + dy) / this->snappedBounds.height :
                                1;
            f = ((std::abs(dx) < std::abs(dy)) && (fx != 1) || fy == 1) ? fx : fy;
            f = (this->width * f < minSize || this->height * f < minSize) ? 1 : f;
            this->scaleShift(f, f, true, false);
        } else if (this->mouseDownType == CURSOR_SELECTION_BOTTOM_RIGHT) {
            xOffset -= this->width;
            yOffset -= this->height;
            f = sqrt(abs((this->height + yOffset) / this->height * (this->width + xOffset) / this->width));
            f = std::max({f, minSize / this->width, minSize / this->height});
            this->scaleShift(f, f, false, false);

            double snappedX = snappingHandler.snapHorizontally(this->snappedBounds.x + this->snappedBounds.width, alt);
            double snappedY = snappingHandler.snapVertically(this->snappedBounds.y + this->snappedBounds.height, alt);
            double dx = snappedX - this->snappedBounds.x - this->snappedBounds.width;
            double dy = snappedY - this->snappedBounds.y - this->snappedBounds.height;
            double fx = (this->snappedBounds.width > minSize) ?
                                (this->snappedBounds.width + dx) / this->snappedBounds.width :
                                1;
            double fy = (this->snappedBounds.height > minSize) ?
                                (this->snappedBounds.height + dy) / this->snappedBounds.height :
                                1;
            f = ((std::abs(dx) < std::abs(dy)) && (fx != 1) || fy == 1) ? fx : fy;
            f = (this->width * f < minSize || this->height * f < minSize) ? 1 : f;
            this->scaleShift(f, f, false, false);
        } else if (this->mouseDownType == CURSOR_SELECTION_TOP) {
            f = std::max((this->height - yOffset) / this->height, minSize / this->height);
            this->scaleShift(1, f, false, true);

            if (this->snappedBounds.height > minSize) {
                double snappedY = snappingHandler.snapVertically(this->snappedBounds.y, alt);
                f = (this->snappedBounds.height - (snappedY - this->snappedBounds.y)) / this->snappedBounds.height;
                f = (this->height * f < minSize) ? 1 : f;
                this->scaleShift(1, f, false, true);
            }
        } else if (this->mouseDownType == CURSOR_SELECTION_BOTTOM) {
            yOffset -= this->height;
            f = std::max((this->height + yOffset) / this->height, minSize / this->height);
            this->scaleShift(1, f, false, false);

            if (this->snappedBounds.height > minSize) {
                double snappedY =
                        snappingHandler.snapVertically(this->snappedBounds.y + this->snappedBounds.height, alt);
                f = (this->snappedBounds.height + (snappedY - this->snappedBounds.y - this->snappedBounds.height)) /
                    this->snappedBounds.height;
                f = (this->height * f < minSize) ? 1 : f;
                this->scaleShift(1, f, false, false);
            }
        } else if (this->mouseDownType == CURSOR_SELECTION_LEFT) {
            f = std::max((this->width - xOffset) / this->width, minSize / this->width);
            this->scaleShift(f, 1, true, false);

            if (this->snappedBounds.width > minSize) {
                double snappedX = snappingHandler.snapHorizontally(this->snappedBounds.x, alt);
                f = (this->snappedBounds.width - (snappedX - this->snappedBounds.x)) / this->snappedBounds.width;
                f = (this->width * f < minSize) ? 1 : f;
                this->scaleShift(f, 1, true, false);
            }
        } else if (this->mouseDownType == CURSOR_SELECTION_RIGHT) {
            xOffset -= this->width;
            f = std::max((this->width + xOffset) / this->width, minSize / this->width);
            this->scaleShift(f, 1, false, false);

            if (this->snappedBounds.width > minSize) {
                double snappedX =
                        snappingHandler.snapHorizontally(this->snappedBounds.x + this->snappedBounds.width, alt);
                f = (this->snappedBounds.width + (snappedX - this->snappedBounds.x - this->snappedBounds.width)) /
                    this->snappedBounds.width;
                f = (this->width * f < minSize) ? 1 : f;
                this->scaleShift(f, 1, false, false);
            }
        }
    }

    this->view->getXournal()->repaintSelection();

    XojPageView* v = getPageViewUnderCursor();

    if (v && v != this->view) {
        XournalView* xournal = this->view->getXournal();
        int pageNr = xournal->getControl()->getDocument()->indexOf(v->getPage());

        xournal->pageSelected(pageNr);

        translateToView(v);
    }
}

// scales with scale factors fx and fy fixing the corner of the reduced bounding box defined by changeLeft and
// changeTop
void EditSelection::scaleShift(double fx, double fy, bool changeLeft, bool changeTop) {
    double dx = (changeLeft) ? this->snappedBounds.width * (1 - fx) : 0;
    double dy = (changeTop) ? this->snappedBounds.height * (1 - fy) : 0;
    this->width *= fx;
    this->height *= fy;
    this->snappedBounds.width *= fx;
    this->snappedBounds.height *= fy;

    this->x += dx + (this->x - this->snappedBounds.x) * (fx - 1);
    this->y += dy + (this->y - this->snappedBounds.y) * (fy - 1);
    this->snappedBounds.x += dx;
    this->snappedBounds.y += dy;

    // compute new rotation center
    double cx = this->snappedBounds.x + this->snappedBounds.width / 2;
    double cy = this->snappedBounds.y + this->snappedBounds.height / 2;
    // transform it back with old rotation center
    double zoom = this->view->getXournal()->getZoom();
    double cxRot = cx * zoom;
    double cyRot = cy * zoom;
    cairo_matrix_t inv = this->cmatrix;
    cairo_matrix_invert(&inv);
    cairo_matrix_transform_point(&inv, &cxRot, &cyRot);
    cxRot /= zoom;
    cyRot /= zoom;
    // move to compensate for changed rotation centers
    moveSelection(cxRot - cx, cyRot - cy);
}

auto EditSelection::getPageViewUnderCursor() -> XojPageView* {
    double zoom = view->getXournal()->getZoom();

    // get grabbing hand position
    double hx = this->view->getX() + (this->snappedBounds.x + this->relMousePosX) * zoom;
    double hy = this->view->getY() + (this->snappedBounds.y + this->relMousePosY) * zoom;


    Layout* layout = gtk_xournal_get_layout(this->view->getXournal()->getWidget());
    XojPageView* v = layout->getViewAt(hx, hy);

    return v;
}

/**
 * Translate all coordinates which are relative to the current view to the new view,
 * and set the attribute view to the new view
 */
void EditSelection::translateToView(XojPageView* v) {
    double zoom = view->getXournal()->getZoom();

    double ox = this->snappedBounds.x - this->x;
    double oy = this->snappedBounds.y - this->y;
    int aX1 = getXOnViewAbsolute();
    int aY1 = getYOnViewAbsolute();

    this->x = (aX1 - v->getX()) / zoom;
    this->y = (aY1 - v->getY()) / zoom;
    this->snappedBounds.x = this->x + ox;
    this->snappedBounds.y = this->y + oy;

    this->view = v;

    //	int aX2 = getXOnViewAbsolute();
    //	int aY2 = getYOnViewAbsolute();
    //
    //	if (aX1 != aX2)
    //	{
    //		g_message("aX1 != aX2!! %i / %i", aX1, aX2);
    //	}
    //	if (aY1 != aY2)
    //	{
    //		g_message("aY1 != aY2!! %i / %i", aY1, aY2);
    //	}
}

void EditSelection::copySelection() {
    undo->addUndoAction(UndoActionPtr(contents->copySelection(this->view->getPage(), this->view, this->x, this->y)));
}

/**
 * If the selection should moved (or rescaled)
 */
auto EditSelection::isMoving() -> bool { return this->mouseDownType != CURSOR_SELECTION_NONE; }

/**
 * Move the selection
 */

void EditSelection::updateMatrix() {
    double zoom = this->view->getXournal()->getZoom();
    // store rotation matrix for pointer use; the center of the rotation is the center of the bounding box
    double rx = (this->snappedBounds.x + this->snappedBounds.width / 2) * zoom;
    double ry = (this->snappedBounds.y + this->snappedBounds.height / 2) * zoom;

    cairo_matrix_init_identity(&this->cmatrix);
    cairo_matrix_translate(&this->cmatrix, rx, ry);
    cairo_matrix_rotate(&this->cmatrix, -this->rotation);
    cairo_matrix_translate(&this->cmatrix, -rx, -ry);
}

void EditSelection::moveSelection(double dx, double dy) {
    this->x += dx;
    this->y += dy;
    this->snappedBounds.x += dx;
    this->snappedBounds.y += dy;

    ensureWithinVisibleArea();

    updateMatrix();

    this->view->getXournal()->repaintSelection();
}

/**
 * If the selection is outside the visible area correct the coordinates
 */
void EditSelection::ensureWithinVisibleArea() {
    int viewx = this->view->getX();
    int viewy = this->view->getY();
    double zoom = this->view->getXournal()->getZoom();
    // need to modify this to take into account the position
    // of the object, plus typecast because XojPageView takes ints
    this->view->getXournal()->ensureRectIsVisible(
            static_cast<int>(viewx + this->x * zoom), static_cast<int>(viewy + this->y * zoom),
            static_cast<int>(this->width * zoom), static_cast<int>(this->height * zoom));
}

/**
 * Get the cursor type for the current position (if 0 then the default cursor should be used)
 */
auto EditSelection::getSelectionTypeForPos(double x, double y, double zoom) -> CursorSelectionType {
    double x1 = getXOnView() * zoom;
    double x2 = x1 + (this->width * zoom);
    double y1 = getYOnView() * zoom;
    double y2 = y1 + (this->height * zoom);


    cairo_matrix_transform_point(&this->cmatrix, &x, &y);


    const int EDGE_PADDING = (this->btnWidth / 2) + 2;
    const int BORDER_PADDING = (this->btnWidth / 2);

    if (x1 - EDGE_PADDING <= x && x <= x1 + EDGE_PADDING && y1 - EDGE_PADDING <= y && y <= y1 + EDGE_PADDING) {
        return CURSOR_SELECTION_TOP_LEFT;
    }

    if (x2 - EDGE_PADDING <= x && x <= x2 + EDGE_PADDING && y1 - EDGE_PADDING <= y && y <= y1 + EDGE_PADDING) {
        return CURSOR_SELECTION_TOP_RIGHT;
    }

    if (x1 - EDGE_PADDING <= x && x <= x1 + EDGE_PADDING && y2 - EDGE_PADDING <= y && y <= y2 + EDGE_PADDING) {
        return CURSOR_SELECTION_BOTTOM_LEFT;
    }

    if (x2 - EDGE_PADDING <= x && x <= x2 + EDGE_PADDING && y2 - EDGE_PADDING <= y && y <= y2 + EDGE_PADDING) {
        return CURSOR_SELECTION_BOTTOM_RIGHT;
    }

    if (x1 - (20 + this->btnWidth) - BORDER_PADDING <= x && x1 - (20 + this->btnWidth) + BORDER_PADDING >= x &&
        y1 - BORDER_PADDING <= y && y1 + BORDER_PADDING >= y) {
        return CURSOR_SELECTION_DELETE;
    }


    if (supportRotation && x2 - BORDER_PADDING + 8 + this->btnWidth <= x &&
        x <= x2 + BORDER_PADDING + 8 + this->btnWidth && (y2 + y1) / 2 - 4 - BORDER_PADDING <= y &&
        (y2 + y1) / 2 + 4 + BORDER_PADDING >= y) {
        return CURSOR_SELECTION_ROTATE;
    }

    if (!this->aspectRatio) {
        if (x1 <= x && x2 >= x) {
            if (y1 - BORDER_PADDING <= y && y <= y1 + BORDER_PADDING) {
                return CURSOR_SELECTION_TOP;
            }

            if (y2 - BORDER_PADDING <= y && y <= y2 + BORDER_PADDING) {
                return CURSOR_SELECTION_BOTTOM;
            }
        }

        if (y1 <= y && y2 >= y) {
            if (x1 - BORDER_PADDING <= x && x <= x1 + BORDER_PADDING) {
                return CURSOR_SELECTION_LEFT;
            }

            if (x2 - BORDER_PADDING <= x && x <= x2 + BORDER_PADDING) {
                return CURSOR_SELECTION_RIGHT;
            }
        }
    }

    if (x1 <= x && x <= x2 && y1 <= y && y <= y2) {
        return CURSOR_SELECTION_MOVE;
    }

    return CURSOR_SELECTION_NONE;
}

/**
 * Paints the selection to cr, with the given zoom factor. The coordinates of cr
 * should be relative to the provided view by getView() (use translateEvent())
 */
void EditSelection::paint(cairo_t* cr, double zoom) {
    double x = this->x;
    double y = this->y;


    if (std::abs(this->rotation) > __DBL_EPSILON__) {
        this->rotation = snappingHandler.snapAngle(this->rotation, false);


        double rx = (snappedBounds.x + snappedBounds.width / 2) * zoom;
        double ry = (snappedBounds.y + snappedBounds.height / 2) * zoom;

        cairo_translate(cr, rx, ry);
        cairo_rotate(cr, this->rotation);

        // Draw the rotation point for debugging
        // cairo_set_source_rgb(cr, 0, 1, 0);
        // cairo_rectangle(cr, 0, 0, 10, 10);
        // cairo_stroke(cr);

        cairo_translate(cr, -rx, -ry);
    }
    this->contents->paint(cr, x, y, this->rotation, this->width, this->height, zoom);

    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);

    GdkRGBA selectionColor = view->getSelectionColor();

    // set the line always the same size on display
    cairo_set_line_width(cr, 1);

    const double dashes[] = {10.0, 10.0};
    cairo_set_dash(cr, dashes, sizeof(dashes) / sizeof(dashes[0]), 0);
    gdk_cairo_set_source_rgba(cr, &selectionColor);

    cairo_rectangle(cr, x * zoom, y * zoom, width * zoom, height * zoom);

    // for debugging
    // cairo_rectangle(cr, snappedBounds.x * zoom, snappedBounds.y * zoom, snappedBounds.width * zoom,
    // snappedBounds.height * zoom);

    cairo_stroke_preserve(cr);
    auto applied = GdkRGBA{selectionColor.red, selectionColor.green, selectionColor.blue, 0.3};
    gdk_cairo_set_source_rgba(cr, &applied);
    cairo_fill(cr);

    cairo_set_dash(cr, nullptr, 0, 0);

    if (!this->aspectRatio) {
        // top
        drawAnchorRect(cr, x + width / 2, y, zoom);
        // bottom
        drawAnchorRect(cr, x + width / 2, y + height, zoom);
        // left
        drawAnchorRect(cr, x, y + height / 2, zoom);
        // right
        drawAnchorRect(cr, x + width, y + height / 2, zoom);

        if (supportRotation) {
            // rotation handle
            drawAnchorRotation(cr, x + width + (8 + this->btnWidth) / zoom, y + height / 2, zoom);
        }
    }

    // top left
    drawAnchorRect(cr, x, y, zoom);
    // top right
    drawAnchorRect(cr, x + width, y, zoom);
    // bottom left
    drawAnchorRect(cr, x, y + height, zoom);
    // bottom right
    drawAnchorRect(cr, x + width, y + height, zoom);


    drawDeleteRect(cr, x - (20 + this->btnWidth) / zoom, y, zoom);
}

void EditSelection::drawAnchorRotation(cairo_t* cr, double x, double y, double zoom) {
    GdkRGBA selectionColor = view->getSelectionColor();
    gdk_cairo_set_source_rgba(cr, &selectionColor);
    cairo_rectangle(cr, x * zoom - (this->btnWidth / 2), y * zoom - (this->btnWidth / 2), this->btnWidth,
                    this->btnWidth);
    cairo_stroke_preserve(cr);
    cairo_set_source_rgb(cr, 1, 0, 0);
    cairo_fill(cr);
}

/**
 * draws an idicator where you can scale the selection
 */
void EditSelection::drawAnchorRect(cairo_t* cr, double x, double y, double zoom) {
    GdkRGBA selectionColor = view->getSelectionColor();
    gdk_cairo_set_source_rgba(cr, &selectionColor);
    cairo_rectangle(cr, x * zoom - (this->btnWidth / 2), y * zoom - (this->btnWidth / 2), this->btnWidth,
                    this->btnWidth);
    cairo_stroke_preserve(cr);
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_fill(cr);
}


/**
 * draws an idicator where you can delete the selection
 */
void EditSelection::drawDeleteRect(cairo_t* cr, double x, double y, double zoom) const {
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_rectangle(cr, x * zoom - (this->btnWidth / 2), y * zoom - (this->btnWidth / 2), this->btnWidth,
                    this->btnWidth);
    cairo_stroke(cr);
    cairo_set_source_rgb(cr, 1, 0, 0);
    cairo_move_to(cr, x * zoom - (this->btnWidth / 2), y * zoom - (this->btnWidth / 2));
    cairo_rel_move_to(cr, this->btnWidth, 0);
    cairo_rel_line_to(cr, -this->btnWidth, this->btnWidth);
    cairo_rel_move_to(cr, this->btnWidth, 0);
    cairo_rel_line_to(cr, -this->btnWidth, -this->btnWidth);
    cairo_stroke(cr);
}


auto EditSelection::getView() -> XojPageView* { return this->view; }

void EditSelection::serialize(ObjectOutputStream& out) {
    out.writeObject("EditSelection");

    out.writeDouble(this->x);
    out.writeDouble(this->y);
    out.writeDouble(this->width);
    out.writeDouble(this->height);

    out.writeDouble(this->snappedBounds.x);
    out.writeDouble(this->snappedBounds.y);
    out.writeDouble(this->snappedBounds.width);
    out.writeDouble(this->snappedBounds.height);

    this->contents->serialize(out);
    out.endObject();

    out.writeInt(this->getElements()->size());
    for (Element* e: *this->getElements()) {
        e->serialize(out);
    }
}

void EditSelection::readSerialized(ObjectInputStream& in) {
    in.readObject("EditSelection");
    this->x = in.readDouble();
    this->y = in.readDouble();
    this->width = in.readDouble();
    this->height = in.readDouble();

    double xSnap = in.readDouble();
    double ySnap = in.readDouble();
    double wSnap = in.readDouble();
    double hSnap = in.readDouble();
    this->snappedBounds = Rectangle<double>{xSnap, ySnap, wSnap, hSnap};
    this->contents->readSerialized(in);

    in.endObject();
}
