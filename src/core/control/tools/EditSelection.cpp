#include "EditSelection.h"

#include <algorithm>  // for min, max, stable_sort
#include <cmath>      // for abs, cos, sin, cop...
#include <cstddef>    // for size_t
#include <limits>     // for numeric_limits
#include <memory>     // for make_unique, __sha...
#include <string>     // for string

#include <gdk/gdk.h>  // for gdk_cairo_set_sour...

#include "control/Control.h"                       // for Control
#include "control/settings/Settings.h"             // for Settings
#include "control/tools/CursorSelectionType.h"     // for CURSOR_SELECTION_NONE
#include "control/tools/SnapToGridInputHandler.h"  // for SnapToGridInputHan...
#include "control/zoom/ZoomControl.h"              // for ZoomControl
#include "gui/Layout.h"                            // for Layout
#include "gui/PageView.h"                          // for XojPageView
#include "gui/XournalView.h"                       // for XournalView
#include "gui/XournalppCursor.h"                   // for XournalppCursor
#include "gui/widgets/XournalWidget.h"             // for gtk_xournal_get_la...
#include "model/Document.h"                        // for Document
#include "model/Element.h"                         // for Element::Index
#include "model/Layer.h"                           // for Layer
#include "model/LineStyle.h"                       // for LineStyle
#include "model/Point.h"                           // for Point
#include "model/XojPage.h"                         // for XojPage
#include "undo/ArrangeUndoAction.h"                // for ArrangeUndoAction
#include "undo/InsertUndoAction.h"                 // for InsertsUndoAction
#include "undo/UndoRedoHandler.h"                  // for UndoRedoHandler
#include "util/Range.h"                            // for Range
#include "util/i18n.h"                             // for _
#include "util/serializing/ObjectInputStream.h"    // for ObjectInputStream
#include "util/serializing/ObjectOutputStream.h"   // for ObjectOutputStream

#include "EditSelectionContents.h"  // for EditSelectionConte...
#include "Selection.h"              // for Selection

class XojFont;

using std::vector;
using xoj::util::Rectangle;

/// Smallest can scale down to, in pixels.
constexpr size_t MINPIXSIZE = 5;

/// Padding for ui buttons
constexpr int DELETE_PADDING = 20;
constexpr int ROTATE_PADDING = 8;

/// Number of times to trigger edge pan timer per second
constexpr unsigned int PAN_TIMER_RATE = 30;

EditSelection::EditSelection(UndoRedoHandler* undo, const PageRef& page, XojPageView* view):
        x(0),
        y(0),
        rotation(0),
        width(0),
        height(0),
        snappedBounds(Rectangle<double>{}),
        snappingHandler(view->getXournal()->getControl()->getSettings()) {
    construct(undo, view, page);
}

EditSelection::EditSelection(UndoRedoHandler* undo, Selection* selection, XojPageView* view):
        snappingHandler(view->getXournal()->getControl()->getSettings()) {
    calcSizeFromElements(selection->selectedElements);

    construct(undo, view, view->getPage());

    // Construct the insert order
    std::vector<std::pair<Element*, Element::Index>> order;
    for (Element* e: selection->selectedElements) {
        auto i = std::make_pair(e, this->sourceLayer->indexOf(e));
        order.insert(std::upper_bound(order.begin(), order.end(), i, EditSelectionContents::insertOrderCmp), i);
    }

    for (const auto& [e, i]: order) {
        this->addElement(e, i);
        this->sourceLayer->removeElement(e, false);
    }

    view->rerenderPage();
}

EditSelection::EditSelection(UndoRedoHandler* undo, Element* e, XojPageView* view, const PageRef& page):
        snappingHandler(view->getXournal()->getControl()->getSettings()) {
    calcSizeFromElements(std::vector<Element*>{e});
    construct(undo, view, page);

    addElement(e, this->sourceLayer->indexOf(e));
    this->sourceLayer->removeElement(e, false);

    view->rerenderElement(e);
}

EditSelection::EditSelection(UndoRedoHandler* undo, const vector<Element*>& elements, XojPageView* view,
                             const PageRef& page):
        snappingHandler(view->getXournal()->getControl()->getSettings()) {
    calcSizeFromElements(elements);
    construct(undo, view, page);

    for (Element* e: elements) {
        addElement(e, this->sourceLayer->indexOf(e));
        this->sourceLayer->removeElement(e, false);
    }

    view->rerenderPage();
}

EditSelection::EditSelection(UndoRedoHandler* undo, XojPageView* view, const PageRef& page, Layer* layer):
        snappingHandler(view->getXournal()->getControl()->getSettings()) {
    const auto& elements = layer->getElements();
    calcSizeFromElements(elements);
    construct(undo, view, page);

    long i = 0L;
    for (Element* e: elements) {
        addElement(e, i);
        i++;
    }

    layer->clearNoFree();

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

void EditSelection::construct(UndoRedoHandler* undo, XojPageView* view, const PageRef& sourcePage) {
    this->view = view;
    this->undo = undo;
    this->sourcePage = sourcePage;
    this->sourceLayer = this->sourcePage->getSelectedLayer();

    this->aspectRatio = false;
    this->mirror = true;

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
    this->view->getXournal()->getCursor()->setMirror(false);
}

EditSelection::~EditSelection() {
    finalizeSelection();

    this->sourcePage = nullptr;
    this->sourceLayer = nullptr;

    delete this->contents;
    this->contents = nullptr;

    this->view = nullptr;
    this->undo = nullptr;

    if (this->edgePanHandler) {
        g_source_destroy(this->edgePanHandler);
        g_source_unref(this->edgePanHandler);
    }
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
 * gets the minimal bounding box containing all elements of the selection used for e.g. grid snapping
 */
auto EditSelection::getSnappedBounds() const -> Rectangle<double> { return Rectangle<double>{this->snappedBounds}; }

/**
 * get the original bounding rectangle in document coordinates
 */
auto EditSelection::getOriginalBounds() const -> Rectangle<double> { return Rectangle<double>{this->contents->getOriginalBounds()}; }

/**
 * Get the rotation angle of the selection
 */
auto EditSelection::getRotation() const -> double { return this->rotation; }

/**
 * Get if the selection supports being rotated
 */
auto EditSelection::isRotationSupported() const -> bool { return this->supportRotation; }

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
    return this->view->getX() + static_cast<int>(this->getXOnView() * zoom);
}

/**
 * Get the Y coordinate in View coordinates (absolute)
 */
auto EditSelection::getYOnViewAbsolute() -> int {
    double zoom = view->getXournal()->getZoom();
    return this->view->getY() + static_cast<int>(this->getYOnView() * zoom);
}

/**
 * Sets the tool size for pen or eraser, returs an undo action
 * (or nullptr if nothing is done)
 */
auto EditSelection::setSize(ToolSize size, const double* thicknessPen, const double* thicknessHighlighter,
                            const double* thicknessEraser) -> UndoAction* {
    return this->contents->setSize(size, thicknessPen, thicknessHighlighter, thicknessEraser);
}

/**
 * Fills the stroke, return an undo action
 * (Or nullptr if nothing done, e.g. because there is only an image)
 */
auto EditSelection::setFill(int alphaPen, int alphaHighligther) -> UndoAction* {
    return this->contents->setFill(alphaPen, alphaHighligther);
}

/**
 * Set the line style of all elements, return an undo action
 * (Or nullptr if nothing done)
 */
auto EditSelection::setLineStyle(LineStyle style) -> UndoActionPtr { return this->contents->setLineStyle(style); }

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
void EditSelection::addElement(Element* e, Element::Index order) {
    this->contents->addElement(e, order);

    if (e->rescaleOnlyAspectRatio()) {
        this->aspectRatio = true;
    }

    if (!e->rescaleWithMirror()) {
        this->mirror = false;
    }

    if (e->getType() != ELEMENT_STROKE) {
        // Currently only stroke supports rotation
        supportRotation = false;
    }
}

/**
 * Returns all containing elements of this selection
 */
auto EditSelection::getElements() const -> const vector<Element*>& { return this->contents->getElements(); }

/**
 * Returns the insert order of this selection
 */
auto EditSelection::getInsertOrder() const -> std::deque<std::pair<Element*, Element::Index>> const& {
    return this->contents->getInsertOrder();
}

auto EditSelection::rearrangeInsertOrder(const OrderChange change) -> UndoActionPtr {
    using InsertOrder = std::deque<std::pair<Element*, Element::Index>>;
    const InsertOrder oldOrd = this->getInsertOrder();
    InsertOrder newOrd;
    std::string desc = _("Arrange");
    switch (change) {
        case OrderChange::BringToFront:
            // Set to largest positive signed integer
            for (const auto& [e, _]: oldOrd) { newOrd.emplace_back(e, std::numeric_limits<Element::Index>::max()); }
            desc = _("Bring to front");
            break;
        case OrderChange::BringForward:
            // Set indices of elements to range from [max(indices) + 1, max(indices) + 1 + num elements)
            newOrd = oldOrd;
            std::stable_sort(newOrd.begin(), newOrd.end(), EditSelectionContents::insertOrderCmp);
            if (!newOrd.empty()) {
                Element::Index i = newOrd.back().second + 1;
                for (auto& it: newOrd) { it.second = i++; }
            }
            desc = _("Bring forward");
            break;
        case OrderChange::SendBackward:
            // Set indices of elements to range from [min(indices) - 1, min(indices) + num elements - 1)
            newOrd = oldOrd;
            std::stable_sort(newOrd.begin(), newOrd.end(), EditSelectionContents::insertOrderCmp);
            if (!newOrd.empty()) {
                Element::Index i = newOrd.front().second;
                i = i > 0 ? i - 1 : 0;
                for (auto& it: newOrd) { it.second = i++; }
            }
            desc = _("Send backward");
            break;
        case OrderChange::SendToBack:
            Element::Index i = 0;
            for (const auto& [e, _]: oldOrd) {
                newOrd.emplace_back(e, i);
                i++;
            }
            desc = _("Send to back");
            break;
    }

    this->contents->replaceInsertOrder(newOrd);
    PageRef page = this->view->getPage();

    return std::make_unique<ArrangeUndoAction>(page, page->getSelectedLayer(), desc, std::move(oldOrd),
                                               std::move(newOrd));
}

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

    this->sourcePage = page;
    this->sourceLayer = layer;

    this->contents->updateContent(this->getRect(), this->snappedBounds, this->rotation, this->aspectRatio, layer, page,
                                  this->view, this->undo, this->mouseDownType);

    this->mouseDownType = CURSOR_SELECTION_NONE;

    const bool wasEdgePanning = this->isEdgePanning();
    this->setEdgePan(false);
    updateMatrix();
    if (wasEdgePanning) {
        this->ensureWithinVisibleArea();
    }
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
        if ((this->relMousePosRotX > this->snappedBounds.width / 2) ==
            (this->snappedBounds.width > 0)) {  // closer to the right side
            cx += this->snappedBounds.width;
        }
        if ((this->relMousePosRotY > this->snappedBounds.height / 2) ==
            (this->snappedBounds.height > 0)) {  // closer to the lower side
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
        if (!this->edgePanInhibitNext) {
            moveSelection(p.x - cx, p.y - cy);
            this->setEdgePan(true);
        } else {
            this->edgePanInhibitNext = false;
        }
    } else if (this->mouseDownType == CURSOR_SELECTION_ROTATE && supportRotation) {  // catch rotation here
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

        double minSize = MINPIXSIZE / zoom;

        // store pull direction value
        int xSide = 0;
        int ySide = 0;
        if (this->mouseDownType == CURSOR_SELECTION_TOP_LEFT) {
            xSide = -1;
            ySide = -1;
        } else if (this->mouseDownType == CURSOR_SELECTION_TOP_RIGHT) {
            xSide = 1;
            ySide = -1;
        } else if (this->mouseDownType == CURSOR_SELECTION_BOTTOM_LEFT) {
            xSide = -1;
            ySide = 1;
        } else if (this->mouseDownType == CURSOR_SELECTION_BOTTOM_RIGHT) {
            xSide = 1;
            ySide = 1;
        } else if (this->mouseDownType == CURSOR_SELECTION_TOP) {
            ySide = -1;
        } else if (this->mouseDownType == CURSOR_SELECTION_BOTTOM) {
            ySide = 1;
        } else if (this->mouseDownType == CURSOR_SELECTION_LEFT) {
            xSide = -1;
        } else if (this->mouseDownType == CURSOR_SELECTION_RIGHT) {
            xSide = 1;
        }
        // sanity check
        if (xSide || ySide) {
            // get normalized direction vector for input interpretation (dependent on aspect ratio)
            double diag = hypot(xSide * this->width, ySide * this->height);
            double nx = xSide * this->width / diag;
            double ny = ySide * this->height / diag;

            int xMul = (xSide + 1) / 2;
            int yMul = (ySide + 1) / 2;
            double xOffset =
                    (rx - this->x) - this->width * xMul;  // x-offset from corner/side that is used for resizing
            double yOffset =
                    (ry - this->y) - this->height * yMul;  // y-offset from corner/side that is used for resizing

            // calculate scale factor using dot product
            double f = (xOffset * nx + yOffset * ny + diag) / diag;
            f = std::copysign(std::max(std::abs(f), minSize / std::min(std::abs(this->width), std::abs(this->height))),
                              f);
            if (mirror || f > 0) {
                scaleShift(xSide ? f : 1, ySide ? f : 1, xSide == -1, ySide == -1);

                // in each case first scale without snapping consideration then snap
                // take care that wSnap and hSnap are not too small
                double snappedX =
                        snappingHandler.snapHorizontally(this->snappedBounds.x + this->snappedBounds.width * xMul, alt);
                double snappedY =
                        snappingHandler.snapVertically(this->snappedBounds.y + this->snappedBounds.height * yMul, alt);
                double dx = snappedX - this->snappedBounds.x - this->snappedBounds.width * xMul;
                double dy = snappedY - this->snappedBounds.y - this->snappedBounds.height * yMul;
                double fx = (std::abs(this->snappedBounds.width) > minSize) ?
                                    (this->snappedBounds.width + dx * xSide) / this->snappedBounds.width :
                                    1;
                double fy = (std::abs(this->snappedBounds.height) > minSize) ?
                                    (this->snappedBounds.height + dy * ySide) / this->snappedBounds.height :
                                    1;
                f = (((std::abs(dx) < std::abs(dy)) && (fx != 1)) || fy == 1) ? fx : fy;
                f = (std::abs(this->width) * std::abs(f) < minSize || std::abs(this->height) * std::abs(f) < minSize) ?
                            1 :
                            f;
                scaleShift(xSide ? f : 1, ySide ? f : 1, xSide == -1, ySide == -1);

                this->view->getXournal()->getCursor()->setMirror(this->width * this->height < 0);
            }
        }
    }

    this->view->getXournal()->repaintSelection();

    XojPageView* v = getPageViewUnderCursor();

    if (v && v != this->view) {
        XournalView* xournal = this->view->getXournal();
        const auto pageNr = xournal->getControl()->getDocument()->indexOf(v->getPage());

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
    XojPageView* v = layout->getPageViewAt(static_cast<int>(hx), static_cast<int>(hy));

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
    // clone elements in the insert order
    std::deque<std::pair<Element*, Element::Index>> clonedInsertOrder;
    for (auto [e, index]: getInsertOrder()) { clonedInsertOrder.emplace_back(e->clone(), index); }

    // apply transformations and add to layer
    finalizeSelection();

    // restore insert order
    contents->replaceInsertOrder(clonedInsertOrder);

    // add undo action
    PageRef page = this->view->getPage();
    Layer* layer = page->getSelectedLayer();
    undo->addUndoAction(std::make_unique<InsertsUndoAction>(page, layer, getElements()));
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

    updateMatrix();

    this->view->getXournal()->repaintSelection();
}

void EditSelection::setEdgePan(bool pan) {
    if (pan && !this->edgePanHandler) {
        this->edgePanHandler = g_timeout_source_new(1000 / PAN_TIMER_RATE);
        g_source_set_callback(this->edgePanHandler, reinterpret_cast<GSourceFunc>(EditSelection::handleEdgePan), this,
                              nullptr);
        g_source_attach(this->edgePanHandler, nullptr);
    } else if (!pan && this->edgePanHandler) {
        g_source_destroy(this->edgePanHandler);
        g_source_unref(this->edgePanHandler);
        this->edgePanHandler = nullptr;
        this->edgePanInhibitNext = false;
    }
}

bool EditSelection::isEdgePanning() const { return this->edgePanHandler; }

bool EditSelection::handleEdgePan(EditSelection* self) {
    if (self->view->getXournal()->getControl()->getZoomControl()->isZoomPresentationMode()) {
        self->setEdgePan(false);
        return false;
    }


    Layout* layout = gtk_xournal_get_layout(self->view->getXournal()->getWidget());
    const Settings* const settings = self->getView()->getXournal()->getControl()->getSettings();
    const double zoom = self->view->getXournal()->getZoom();

    // Helper function to compute scroll amount for a single dimension, based on visible region and selection bbox
    const auto computeScrollAmt = [&](double visMin, double visLen, double bboxMin, double bboxLen,
                                      double layoutSize) -> double {
        const bool belowMin = bboxMin < visMin;
        const bool aboveMax = bboxMin + bboxLen > visMin + visLen;
        const double visMax = visMin + visLen;
        const double bboxMax = bboxMin + bboxLen;

        // Scroll amount multiplier
        double mult = 0.0;

        // Calculate bonus scroll amount due to proportion of selection out of view.
        const double maxMult = settings->getEdgePanMaxMult();
        int panDir = 0;
        if (aboveMax) {
            panDir = 1;
            mult = maxMult * std::min(bboxLen, bboxMax - visMax) / bboxLen;
        } else if (belowMin) {
            panDir = -1;
            mult = maxMult * std::min(bboxLen, visMin - bboxMin) / bboxLen;
        }

        // Base amount to translate selection (in document coordinates) per timer tick
        const double panSpeed = settings->getEdgePanSpeed();
        const double translateAmt = visLen * panSpeed / (100.0 * PAN_TIMER_RATE);

        // Amount to scroll the visible area by (in layout coordinates), accounting for multiplier
        double layoutScroll = zoom * panDir * (translateAmt * mult);

        // If scrolling past layout boundaries, clamp scroll amount to boundary
        if (visMin + layoutScroll < 0) {
            layoutScroll = -visMin;
        } else if (visMax + layoutScroll > layoutSize) {
            layoutScroll = std::max(0.0, layoutSize - visMax);
        }

        return layoutScroll;
    };

    // Compute scroll (for layout) and translation (for selection) for x and y
    const int layoutWidth = layout->getMinimalWidth();
    const int layoutHeight = layout->getMinimalHeight();
    const auto visRect = layout->getVisibleRect();
    const auto bbox = self->getBoundingBoxInView();
    const auto layoutScrollX = computeScrollAmt(visRect.x, visRect.width, bbox.x, bbox.width, layoutWidth);
    const auto layoutScrollY = computeScrollAmt(visRect.y, visRect.height, bbox.y, bbox.height, layoutHeight);
    const auto translateX = layoutScrollX / zoom;
    const auto translateY = layoutScrollY / zoom;

    // Perform the scrolling
    bool edgePanned = false;
    if (self->isMoving() && (layoutScrollX != 0.0 || layoutScrollY != 0.0)) {
        layout->scrollRelative(layoutScrollX, layoutScrollY);
        self->moveSelection(translateX, translateY);
        edgePanned = true;

        // To prevent the selection from jumping and to reduce jitter, block the selection movement triggered by user
        // input
        self->edgePanInhibitNext = true;
    } else {
        // No panning, so disable the timer.
        self->setEdgePan(false);
    }

    return edgePanned;
}

auto EditSelection::getBoundingBoxInView() const -> Rectangle<double> {
    int viewx = this->view->getX();
    int viewy = this->view->getY();
    double zoom = this->view->getXournal()->getZoom();

    double sin = std::sin(this->rotation);
    double cos = std::cos(this->rotation);
    double w = std::abs(this->width * cos) + std::abs(this->height * sin);
    double h = std::abs(this->width * sin) + std::abs(this->height * cos);
    double cx = this->x + this->width / 2.0;
    double cy = this->y + this->height / 2.0;
    double minx = cx - w / 2.0;
    double miny = cy - h / 2.0;

    return {viewx + minx * zoom, viewy + miny * zoom, w * zoom, h * zoom};
}

void EditSelection::ensureWithinVisibleArea() {
    const Rectangle<double> viewRect = this->getBoundingBoxInView();
    // need to modify this to take into account the position
    // of the object, plus typecast because XojPageView takes ints
    this->view->getXournal()->ensureRectIsVisible(static_cast<int>(viewRect.x), static_cast<int>(viewRect.y),
                                                  static_cast<int>(viewRect.width), static_cast<int>(viewRect.height));
}

/**
 * Get the cursor type for the current position (if 0 then the default cursor should be used)
 */
auto EditSelection::getSelectionTypeForPos(double x, double y, double zoom) -> CursorSelectionType {
    double x1 = getXOnView() * zoom;
    double x2 = x1 + this->width * zoom;
    double y1 = getYOnView() * zoom;
    double y2 = y1 + this->height * zoom;
    double xmin = std::min(x1, x2);
    double xmax = std::max(x1, x2);
    double ymin = std::min(y1, y2);
    double ymax = std::max(y1, y2);

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

    if (xmin - (DELETE_PADDING + this->btnWidth) - BORDER_PADDING <= x &&
        x <= xmin - (DELETE_PADDING + this->btnWidth) + BORDER_PADDING && y1 - BORDER_PADDING <= y &&
        y <= y1 + BORDER_PADDING) {
        return CURSOR_SELECTION_DELETE;
    }


    if (supportRotation && xmax - BORDER_PADDING + ROTATE_PADDING + this->btnWidth <= x &&
        x <= xmax + BORDER_PADDING + ROTATE_PADDING + this->btnWidth && (y2 + y1) / 2 - 4 - BORDER_PADDING <= y &&
        (y2 + y1) / 2 + 4 + BORDER_PADDING >= y) {
        return CURSOR_SELECTION_ROTATE;
    }

    if (!this->aspectRatio) {
        if (xmin <= x && x <= xmax) {
            if (y1 - BORDER_PADDING <= y && y <= y1 + BORDER_PADDING) {
                return CURSOR_SELECTION_TOP;
            }

            if (y2 - BORDER_PADDING <= y && y <= y2 + BORDER_PADDING) {
                return CURSOR_SELECTION_BOTTOM;
            }
        }

        if (ymin <= y && y <= ymax) {
            if (x1 - BORDER_PADDING <= x && x <= x1 + BORDER_PADDING) {
                return CURSOR_SELECTION_LEFT;
            }

            if (x2 - BORDER_PADDING <= x && x <= x2 + BORDER_PADDING) {
                return CURSOR_SELECTION_RIGHT;
            }
        }
    }

    if (xmin <= x && x <= xmax && ymin <= y && y <= ymax) {
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

    cairo_rectangle(cr, std::min(x, x + width) * zoom, std::min(y, y + height) * zoom, std::abs(width) * zoom,
                    std::abs(height) * zoom);

    // for debugging
    // cairo_rectangle(cr, snappedBounds.x * zoom, snappedBounds.y * zoom, snappedBounds.width * zoom,
    // snappedBounds.height * zoom);

    cairo_stroke_preserve(cr);
    auto applied = GdkRGBA{selectionColor.red, selectionColor.green, selectionColor.blue, 0.3};
    gdk_cairo_set_source_rgba(cr, &applied);
    cairo_fill(cr);

    ToolHandler* toolHandler = view->getXournal()->getControl()->getToolHandler();
    if (toolHandler->getToolType() != TOOL_HAND) {
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
                drawAnchorRotation(cr,
                                   std::min(x, x + width) + std::abs(width) + (ROTATE_PADDING + this->btnWidth) / zoom,
                                   y + height / 2, zoom);
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

        drawDeleteRect(cr, std::min(x, x + width) - (DELETE_PADDING + this->btnWidth) / zoom, y, zoom);
    }
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

void EditSelection::serialize(ObjectOutputStream& out) const {
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

    out.writeInt(static_cast<int>(this->getElements().size()));
    for (Element* e: this->getElements()) { e->serialize(out); }
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
