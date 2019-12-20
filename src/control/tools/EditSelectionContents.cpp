#include "EditSelectionContents.h"

#include <cmath>

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
#include "util/cpp14memory.h"
#include "view/DocumentView.h"

#include "Selection.h"

EditSelectionContents::EditSelectionContents(double x, double y, double width, double height, const PageRef& sourcePage,
                                             Layer* sourceLayer, XojPageView* sourceView) {
    this->crBuffer = nullptr;

    this->rescaleId = 0;

    this->lastWidth = this->originalWidth = width;
    this->lastHeight = this->originalHeight = height;
    this->relativeX = -9999999999;
    this->relativeY = -9999999999;
    this->rotation = 0;
    this->lastRotation = 0;

    this->lastX = this->originalX = x;
    this->lastY = this->originalY = y;

    this->sourcePage = sourcePage;
    this->sourceLayer = sourceLayer;
    this->sourceView = sourceView;
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
    this->selected.emplace_back(e);
    this->indexWithinLayer.emplace(e, order);
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
auto EditSelectionContents::setColor(int color) -> UndoAction* {
    auto* undo = new ColorUndoAction(this->sourcePage, this->sourceLayer);

    bool found = false;

    for (Element* e: this->selected) {
        if (e->getType() == ELEMENT_TEXT || e->getType() == ELEMENT_STROKE) {
            int lastColor = e->getColor();
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
auto EditSelectionContents::getOriginalWidth() const -> double { return this->originalWidth; }

/**
 * Gets the original height of the contents
 */
auto EditSelectionContents::getOriginalHeight() const -> double { return this->originalHeight; }

/**
 * The contents of the selection
 */
void EditSelectionContents::finalizeSelection(double x, double y, double width, double height, bool aspectRatio,
                                              Layer* layer, const PageRef& targetPage, XojPageView* targetView,
                                              UndoRedoHandler* undo) {
    double fx = width / this->originalWidth;
    double fy = height / this->originalHeight;

    if (aspectRatio) {
        double f = (fx + fy) / 2;
        fx = f;
        fy = f;
    }
    bool scale = (width != this->originalWidth || height != this->originalHeight);
    bool rotate = (std::abs(this->rotation) > __DBL_EPSILON__);

    double mx = x - this->originalX;
    double my = y - this->originalY;

    bool move = mx != 0 || my != 0;


    for (auto it = this->selected.rbegin(); it != this->selected.rend(); ++it) {
        Element* e = *it;

        if (move) {
            e->move(mx, my);
        }
        if (scale) {
            e->scale(x, y, fx, fy);
        }
        if (rotate) {
            e->rotate(x, y, this->lastWidth / 2, this->lastHeight / 2, this->rotation);
        }

        auto layerIndex = indexWithinLayer.find(e);
        if (layerIndex == indexWithinLayer.end() ||
            layerIndex->second ==
                    Layer::InvalidElementIndex) {  // if the element didn't have a source layer (e.g, clipboard)
            layer->addElement(e);
        } else {
            layer->insertElement(e, layerIndex->second);
        }
    }
}

auto EditSelectionContents::getOriginalX() const -> double { return this->originalX; }

auto EditSelectionContents::getOriginalY() const -> double { return this->originalY; }

auto EditSelectionContents::getSourceView() -> XojPageView* { return this->sourceView; }


void EditSelectionContents::updateContent(double x, double y, double rotation, double width, double height,
                                          bool aspectRatio, Layer* layer, const PageRef& targetPage,
                                          XojPageView* targetView, UndoRedoHandler* undo, CursorSelectionType type) {
    double mx = x - this->lastX;
    double my = y - this->lastY;
    this->rotation = rotation;

    double fx = width / this->lastWidth;
    double fy = height / this->lastHeight;

    if (aspectRatio) {
        double f = (fx + fy) / 2;
        fx = f;
        fy = f;
    }

    bool scale = (width != this->lastWidth || height != this->lastHeight);

    if (type == CURSOR_SELECTION_MOVE) {
        undo->addUndoAction(mem::make_unique<MoveUndoAction>(this->sourceLayer, this->sourcePage, &this->selected, mx,
                                                             my, layer, targetPage));
    } else if (type == CURSOR_SELECTION_ROTATE) {
        undo->addUndoAction(mem::make_unique<RotateUndoAction>(this->sourcePage, &this->selected, x, y, width / 2,
                                                               height / 2, rotation - this->lastRotation));
        this->rotation = 0;             // reset rotation for next usage
        this->lastRotation = rotation;  // undo one rotation at a time.
    }
    if (scale) {
        // The coordinates which are the center of the scaling
        // operation. Their coordinates depend on the scaling
        // operation performed
        double px = this->lastX;
        double py = this->lastY;

        switch (type) {
            case CURSOR_SELECTION_TOP_LEFT:
            case CURSOR_SELECTION_BOTTOM_LEFT:
            case CURSOR_SELECTION_LEFT:
                px = (this->lastWidth + this->lastX);
                break;
            default:
                break;
        }

        switch (type) {
            case CURSOR_SELECTION_TOP_LEFT:
            case CURSOR_SELECTION_TOP_RIGHT:
            case CURSOR_SELECTION_TOP:
                py = (this->lastHeight + this->lastY);
                break;
            default:
                break;
        }

        // Todo: this needs to be aware of the rotation...  this should all be rewritten to scale and rotate from
        //       center... !!!!!!!!!
        undo->addUndoAction(mem::make_unique<ScaleUndoAction>(this->sourcePage, &this->selected, px, py, fx, fy));
    }

    this->lastX = x;
    this->lastY = y;

    this->lastWidth = width;
    this->lastHeight = height;
}

/**
 * paints the selection
 */
void EditSelectionContents::paint(cairo_t* cr, double x, double y, double rotation, double width, double height,
                                  double zoom) {
    double fx = width / this->originalWidth;
    double fy = height / this->originalHeight;

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

        ec->move(x - this->originalX, y - this->originalY);

        layer->addElement(ec);
        new_elems.push_back(ec);
    }

    view->rerenderPage();

    return new InsertsUndoAction(page, layer, new_elems);
}

void EditSelectionContents::serialize(ObjectOutputStream& out) {
    out.writeObject("EditSelectionContents");

    out.writeDouble(this->originalWidth);
    out.writeDouble(this->originalHeight);

    out.writeDouble(this->originalX);
    out.writeDouble(this->originalY);

    out.writeDouble(this->relativeX);
    out.writeDouble(this->relativeY);

    out.endObject();
}

void EditSelectionContents::readSerialized(ObjectInputStream& in) {
    in.readObject("EditSelectionContents");

    this->originalWidth = in.readDouble();
    this->originalHeight = in.readDouble();

    this->originalX = in.readDouble();
    this->originalY = in.readDouble();

    this->relativeX = in.readDouble();
    this->relativeY = in.readDouble();

    in.endObject();
}
