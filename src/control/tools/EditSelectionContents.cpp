#include "EditSelectionContents.h"

#include <cmath>
#include <memory>

#include "control/Control.h"
#include "gui/PageView.h"
#include "gui/XournalView.h"
#include "model/Document.h"
#include "model/Element.h"
#include "model/Layer.h"
#include "model/Stroke.h"
#include "model/Text.h"
#include "serializing/ObjectInputStream.h"
#include "serializing/ObjectOutputStream.h"
#include "undo/ColorUndoAction.h"
#include "undo/DeleteUndoAction.h"
#include "undo/FillUndoAction.h"
#include "undo/FontUndoAction.h"
#include "undo/InsertUndoAction.h"
#include "undo/MoveUndoAction.h"
#include "undo/RotateUndoAction.h"
#include "undo/ScaleUndoAction.h"
#include "undo/SizeUndoAction.h"
#include "undo/UndoRedoHandler.h"
#include "view/DocumentView.h"

#include "Selection.h"

EditSelectionContents::EditSelectionContents(Rectangle<double> bounds, Rectangle<double> snappedBounds,
                                             const PageRef& sourcePage, Layer* sourceLayer, XojPageView* sourceView):
        lastBounds(bounds),
        originalBounds(bounds),
        lastSnappedBounds(snappedBounds),
        sourcePage(sourcePage),
        sourceLayer(sourceLayer),
        sourceView(sourceView) {

    this->restoreLineWidth =
            this->getSourceView()->getXournal()->getControl()->getSettings()->getRestoreLineWidthEnabled();
}

EditSelectionContents::~EditSelectionContents() {
    if (this->rescaleId) {
        g_source_remove(this->rescaleId);
        this->rescaleId = 0;
    }

    deleteViewBuffer();
}

/**
 * Add an element to the this selection
 */
void EditSelectionContents::addElement(Element* e, Layer::ElementIndex order) {
    g_assert(this->selected.size() == this->insertOrder.size());
    this->selected.emplace_back(e);
    if (order == Layer::InvalidElementIndex) {
        this->insertOrder.emplace_back(e, order);
    } else {
        this->insertOrder.emplace_front(e, order);
    }
}

/**
 * Returns all containig elements of this selections
 */
auto EditSelectionContents::getElements() -> vector<Element*>* { return &this->selected; }

/**
 * Sets the tool size for pen or eraser, returs an undo action
 * (or nullptr if nothing is done)
 */
auto EditSelectionContents::setSize(ToolSize size, const double* thicknessPen, const double* thicknessHilighter,
                                    const double* thicknessEraser) -> UndoAction* {
    auto* undo = new SizeUndoAction(this->sourcePage, this->sourceLayer);

    bool found = false;

    for (Element* e: this->selected) {
        if (e->getType() == ELEMENT_STROKE) {
            auto* s = dynamic_cast<Stroke*>(e);
            StrokeTool tool = s->getToolType();

            double originalWidth = s->getWidth();

            int pointCount = s->getPointCount();
            vector<double> originalPressure = SizeUndoAction::getPressure(s);

            if (tool == STROKE_TOOL_PEN) {
                s->setWidth(thicknessPen[size]);
            } else if (tool == STROKE_TOOL_HIGHLIGHTER) {
                s->setWidth(thicknessHilighter[size]);
            } else if (tool == STROKE_TOOL_ERASER) {
                s->setWidth(thicknessEraser[size]);
            }

            // scale the stroke
            double factor = s->getWidth() / originalWidth;
            s->scalePressure(factor);

            // save the new pressure
            vector<double> newPressure = SizeUndoAction::getPressure(s);

            undo->addStroke(s, originalWidth, s->getWidth(), originalPressure, newPressure, pointCount);
            found = true;
        }
    }

    if (found) {
        this->deleteViewBuffer();
        this->sourceView->getXournal()->repaintSelection();

        return undo;
    }


    delete undo;
    return nullptr;
}

/**
 * Fills the stroke, return an undo action
 * (Or nullptr if nothing done, e.g. because there is only an image)
 */
auto EditSelectionContents::setFill(int alphaPen, int alphaHighligther) -> UndoAction* {
    auto* undo = new FillUndoAction(this->sourcePage, this->sourceLayer);

    bool found = false;

    for (Element* e: this->selected) {
        if (e->getType() == ELEMENT_STROKE) {
            auto* s = dynamic_cast<Stroke*>(e);
            StrokeTool tool = s->getToolType();
            int newFill = 128;

            if (tool == STROKE_TOOL_PEN) {
                newFill = alphaPen;
            } else if (tool == STROKE_TOOL_HIGHLIGHTER) {
                newFill = alphaHighligther;
            } else {
                continue;
            }

            if (newFill == s->getFill()) {
                continue;
            }

            bool originalFill = s->getFill();
            s->setFill(newFill);

            undo->addStroke(s, originalFill, newFill);
            found = true;
        }
    }

    if (found) {
        this->deleteViewBuffer();
        this->sourceView->getXournal()->repaintSelection();

        return undo;
    }


    delete undo;
    return nullptr;
}

/**
 * Sets the font of all containing text elements, return an undo action
 * (or nullptr if there are no Text elements)
 */
auto EditSelectionContents::setFont(XojFont& font) -> UndoAction* {
    double x1 = 0.0 / 0.0;
    double x2 = 0.0 / 0.0;
    double y1 = 0.0 / 0.0;
    double y2 = 0.0 / 0.0;

    auto* undo = new FontUndoAction(this->sourcePage, this->sourceLayer);

    for (Element* e: this->selected) {
        if (e->getType() == ELEMENT_TEXT) {
            Text* t = dynamic_cast<Text*>(e);
            undo->addStroke(t, t->getFont(), font);

            if (std::isnan(x1)) {
                x1 = t->getX();
                y1 = t->getY();
                x2 = t->getX() + t->getElementWidth();
                y2 = t->getY() + t->getElementHeight();
            } else {
                // size with old font
                x1 = std::min(x1, t->getX());
                y1 = std::min(y1, t->getY());

                x2 = std::max(x2, t->getX() + t->getElementWidth());
                y2 = std::max(y2, t->getY() + t->getElementHeight());
            }

            t->setFont(font);

            // size with new font
            x1 = std::min(x1, t->getX());
            y1 = std::min(y1, t->getY());

            x2 = std::max(x2, t->getX() + t->getElementWidth());
            y2 = std::max(y2, t->getY() + t->getElementHeight());
        }
    }

    if (!std::isnan(x1)) {
        this->deleteViewBuffer();
        this->sourceView->getXournal()->repaintSelection();
        return undo;
    }
    delete undo;
    return nullptr;
}

/**
 * Set the color of all elements, return an undo action
 * (Or nullptr if nothing done, e.g. because there is only an image)
 */
auto EditSelectionContents::setColor(Color color) -> UndoAction* {
    auto* undo = new ColorUndoAction(this->sourcePage, this->sourceLayer);

    bool found = false;

    for (Element* e: this->selected) {
        if (e->getType() == ELEMENT_TEXT || e->getType() == ELEMENT_STROKE) {
            auto lastColor = e->getColor();
            e->setColor(color);
            undo->addStroke(e, lastColor, e->getColor());

            found = true;
        }
    }

    if (found) {
        this->deleteViewBuffer();
        this->sourceView->getXournal()->repaintSelection();

        return undo;
    }


    delete undo;
    return nullptr;
}

/**
 * Fills the undo item if the selection is deleted
 * the selection is cleared after
 */
void EditSelectionContents::fillUndoItem(DeleteUndoAction* undo) {
    Layer* layer = this->sourceLayer;

    // Always insert the elements on top
    // Because the elements are already removed
    // and owned by the selection, therefore the layer
    // doesn't know the index anymore
    int index = layer->getElements()->size();
    for (Element* e: this->selected) {
        undo->addElement(layer, e, index);
    }

    this->selected.clear();
    this->insertOrder.clear();
}

/**
 * Callback to redrawing the buffer asynchron
 */
auto EditSelectionContents::repaintSelection(EditSelectionContents* selection) -> bool {
    // delete the selection buffer, force a redraw
    selection->deleteViewBuffer();
    selection->sourceView->getXournal()->repaintSelection();
    selection->rescaleId = 0;

    return false;
}

/**
 * Delete our internal View buffer,
 * it will be recreated when the selection is painted next time
 */
void EditSelectionContents::deleteViewBuffer() {
    if (this->crBuffer) {
        cairo_surface_destroy(this->crBuffer);
        this->crBuffer = nullptr;
    }
}

/**
 * Gets the original width of the contents
 */
auto EditSelectionContents::getOriginalWidth() const -> double { return this->originalBounds.width; }

/**
 * Gets the original height of the contents
 */
auto EditSelectionContents::getOriginalHeight() const -> double { return this->originalBounds.height; }

/**
 * The contents of the selection
 */
void EditSelectionContents::finalizeSelection(Rectangle<double> bounds, Rectangle<double> snappedBounds,
                                              bool aspectRatio, Layer* layer, const PageRef& targetPage,
                                              XojPageView* targetView, UndoRedoHandler* undo) {
    double fx = bounds.width / this->originalBounds.width;
    double fy = bounds.height / this->originalBounds.height;

    if (aspectRatio) {
        double f = (fx + fy) / 2;
        fx = f;
        fy = f;
    }
    bool scale = (bounds.width != this->originalBounds.width || bounds.height != this->originalBounds.height);
    bool rotate = (std::abs(this->rotation) > __DBL_EPSILON__);

    double mx = bounds.x - this->originalBounds.x;
    double my = bounds.y - this->originalBounds.y;

    bool move = mx != 0 || my != 0;

    g_assert(this->selected.size() == this->insertOrder.size());
    for (auto&& [e, index]: this->insertOrder) {
        if (move) {
            e->move(mx, my);
        }
        if (scale) {
            e->scale(bounds.x, bounds.y, fx, fy, 0, this->restoreLineWidth);
        }
        if (rotate) {
            e->rotate(snappedBounds.x + this->lastSnappedBounds.width / 2,
                      snappedBounds.y + this->lastSnappedBounds.height / 2, this->rotation);
        }
        if (index == Layer::InvalidElementIndex) {
            // if the element didn't have a source layer (e.g, clipboard)
            layer->addElement(e);
        } else {
            layer->insertElement(e, index);
        }
    }
}

auto EditSelectionContents::getOriginalX() const -> double { return this->originalBounds.x; }

auto EditSelectionContents::getOriginalY() const -> double { return this->originalBounds.y; }

auto EditSelectionContents::getSourceView() -> XojPageView* { return this->sourceView; }


void EditSelectionContents::updateContent(Rectangle<double> bounds, Rectangle<double> snappedBounds, double rotation,
                                          bool aspectRatio, Layer* layer, const PageRef& targetPage,
                                          XojPageView* targetView, UndoRedoHandler* undo, CursorSelectionType type) {
    double mx = snappedBounds.x - this->lastSnappedBounds.x;
    double my = snappedBounds.y - this->lastSnappedBounds.y;
    this->rotation = rotation;

    double fx = snappedBounds.width / this->lastSnappedBounds.width;
    double fy = snappedBounds.height / this->lastSnappedBounds.height;

    if (aspectRatio) {
        double f = (fx + fy) / 2;
        fx = f;
        fy = f;
    }

    bool scale = (snappedBounds.width != this->lastSnappedBounds.width ||
                  snappedBounds.height != this->lastSnappedBounds.height);

    if (type == CURSOR_SELECTION_MOVE) {
        undo->addUndoAction(std::make_unique<MoveUndoAction>(this->sourceLayer, this->sourcePage, &this->selected, mx,
                                                             my, layer, targetPage));
    } else if (type == CURSOR_SELECTION_ROTATE) {
        undo->addUndoAction(std::make_unique<RotateUndoAction>(
                this->sourcePage, &this->selected, snappedBounds.x + snappedBounds.width / 2,
                snappedBounds.y + snappedBounds.height / 2, rotation - this->lastRotation));
        this->rotation = 0;             // reset rotation for next usage
        this->lastRotation = rotation;  // undo one rotation at a time.
    }
    if (scale) {
        // The coordinates of the center of the scaling operation. They depend on the scaling operation performed
        double px = this->lastSnappedBounds.x;
        double py = this->lastSnappedBounds.y;

        switch (type) {
            case CURSOR_SELECTION_TOP_LEFT:
                [[fallthrough]];
            case CURSOR_SELECTION_BOTTOM_LEFT:
                [[fallthrough]];
            case CURSOR_SELECTION_LEFT:
                px += this->lastSnappedBounds.width;
                break;
            default:
                break;
        }

        switch (type) {
            case CURSOR_SELECTION_TOP_LEFT:
                [[fallthrough]];
            case CURSOR_SELECTION_TOP_RIGHT:
                [[fallthrough]];
            case CURSOR_SELECTION_TOP:
                py += this->lastSnappedBounds.height;
                break;
            default:
                break;
        }
        // now px and py are the center of the scaling in the rotated coordinate system. We need to rotate them
        double cx = this->lastSnappedBounds.x + this->lastSnappedBounds.width / 2;
        double cy = this->lastSnappedBounds.y + this->lastSnappedBounds.height / 2;
        cairo_matrix_t rotMatrix;
        cairo_matrix_init_identity(&rotMatrix);
        cairo_matrix_translate(&rotMatrix, cx, cy);
        cairo_matrix_rotate(&rotMatrix, this->lastRotation);
        cairo_matrix_translate(&rotMatrix, -cx, -cy);
        cairo_matrix_transform_point(&rotMatrix, &px, &py);

        undo->addUndoAction(std::make_unique<ScaleUndoAction>(this->sourcePage, &this->selected, px, py, fx, fy,
                                                              this->lastRotation, restoreLineWidth));
    }

    this->lastBounds = bounds;
    this->lastSnappedBounds = snappedBounds;
}

/**
 * paints the selection
 */
void EditSelectionContents::paint(cairo_t* cr, double x, double y, double rotation, double width, double height,
                                  double zoom) {
    double fx = width / this->originalBounds.width;
    double fy = height / this->originalBounds.height;

    if (this->relativeX == -9999999999) {
        this->relativeX = x;
        this->relativeY = y;
    }

    if (std::abs(rotation) > __DBL_EPSILON__) {
        this->rotation = rotation;
    }

    if (this->crBuffer == nullptr) {
        this->crBuffer = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width * zoom, height * zoom);
        cairo_t* cr2 = cairo_create(this->crBuffer);

        int dx = static_cast<int>(this->relativeX * zoom);
        int dy = static_cast<int>(this->relativeY * zoom);

        cairo_scale(cr2, fx, fy);
        cairo_translate(cr2, -dx, -dy);
        cairo_scale(cr2, zoom, zoom);
        DocumentView view;
        view.drawSelection(cr2, this);

        cairo_destroy(cr2);
    }

    cairo_save(cr);

    int wImg = cairo_image_surface_get_width(this->crBuffer);
    int hImg = cairo_image_surface_get_height(this->crBuffer);

    int wTarget = static_cast<int>(width * zoom);
    int hTarget = static_cast<int>(height * zoom);

    double sx = static_cast<double>(wTarget) / wImg;
    double sy = static_cast<double>(hTarget) / hImg;

    if (wTarget != wImg || hTarget != hImg || std::abs(rotation) > __DBL_EPSILON__) {
        if (!this->rescaleId) {
            this->rescaleId = g_idle_add(reinterpret_cast<GSourceFunc>(repaintSelection), this);
        }
        cairo_scale(cr, sx, sy);
    }

    double dx = static_cast<int>(x * zoom / sx);
    double dy = static_cast<int>(y * zoom / sy);

    cairo_set_source_surface(cr, this->crBuffer, dx, dy);
    cairo_paint(cr);

    cairo_restore(cr);
}

auto EditSelectionContents::copySelection(PageRef page, XojPageView* view, double x, double y) -> UndoAction* {
    Layer* layer = page->getSelectedLayer();

    vector<Element*> new_elems;

    for (Element* e: *getElements()) {
        Element* ec = e->clone();

        ec->move(x - this->originalBounds.x, y - this->originalBounds.y);

        layer->addElement(ec);
        new_elems.push_back(ec);
    }

    view->rerenderPage();

    return new InsertsUndoAction(page, layer, new_elems);
}

void EditSelectionContents::serialize(ObjectOutputStream& out) {
    out.writeObject("EditSelectionContents");

    out.writeDouble(this->originalBounds.x);
    out.writeDouble(this->originalBounds.y);
    out.writeDouble(this->originalBounds.width);
    out.writeDouble(this->originalBounds.height);

    out.writeDouble(this->relativeX);
    out.writeDouble(this->relativeY);

    out.endObject();
}

void EditSelectionContents::readSerialized(ObjectInputStream& in) {
    in.readObject("EditSelectionContents");

    double originalX = in.readDouble();
    double originalY = in.readDouble();
    double originalW = in.readDouble();
    double originalH = in.readDouble();
    this->originalBounds = Rectangle<double>{originalX, originalY, originalW, originalH};

    this->relativeX = in.readDouble();
    this->relativeY = in.readDouble();

    in.endObject();
}
