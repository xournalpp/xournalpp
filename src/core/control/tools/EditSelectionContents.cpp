#include "EditSelectionContents.h"

#include <algorithm>  // for min, max, transform
#include <cmath>      // for abs, isnan
#include <iterator>   // for back_insert_iterator
#include <limits>     // for numeric_limits
#include <memory>     // for make_unique, __shar...

#include <glib.h>  // for g_idle_add, g_sourc...

#include "control/Control.h"                      // for Control
#include "control/settings/Settings.h"            // for Settings
#include "control/tools/CursorSelectionType.h"    // for CURSOR_SELECTION_TO...
#include "gui/PageView.h"                         // for XojPageView
#include "gui/XournalView.h"                      // for XournalView
#include "model/Element.h"                        // for Element, Element::I...
#include "model/Layer.h"                          // for Layer
#include "model/LineStyle.h"                      // for LineStyle
#include "model/Stroke.h"                         // for Stroke, StrokeTool...
#include "model/Text.h"                           // for Text
#include "model/XojPage.h"                        // for XojPage
#include "undo/ColorUndoAction.h"                 // for ColorUndoAction
#include "undo/DeleteUndoAction.h"                // for DeleteUndoAction
#include "undo/FillUndoAction.h"                  // for FillUndoAction
#include "undo/FontUndoAction.h"                  // for FontUndoAction
#include "undo/InsertUndoAction.h"                // for InsertsUndoAction
#include "undo/LineStyleUndoAction.h"             // for LineStyleUndoAction
#include "undo/MoveUndoAction.h"                  // for MoveUndoAction
#include "undo/RotateUndoAction.h"                // for RotateUndoAction
#include "undo/ScaleUndoAction.h"                 // for ScaleUndoAction
#include "undo/SizeUndoAction.h"                  // for SizeUndoAction
#include "undo/UndoRedoHandler.h"                 // for UndoRedoHandler
#include "util/Assert.h"                          // for xoj_assert
#include "util/safe_casts.h"                      // for as_signed
#include "util/serializing/ObjectInputStream.h"   // for ObjectInputStream
#include "util/serializing/ObjectOutputStream.h"  // for ObjectOutputStream
#include "view/ElementContainerView.h"            // for ElementContainerView
#include "view/View.h"                            // for Context

class XojFont;

using std::vector;
using xoj::util::Rectangle;

EditSelectionContents::EditSelectionContents(Rectangle<double> bounds, Rectangle<double> snappedBounds,
                                             const PageRef& sourcePage, Layer* sourceLayer, XojPageView* sourceView):
        originalBounds(bounds),
        lastBounds(bounds),
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
void EditSelectionContents::addElement(Element* e, Element::Index order) {
    xoj_assert(this->selected.size() == this->insertOrder.size());
    this->selected.emplace_back(e);
    auto item = std::make_pair(e, order);
    this->insertOrder.insert(std::upper_bound(this->insertOrder.begin(), this->insertOrder.end(), item, insertOrderCmp),
                             item);
}

void EditSelectionContents::replaceInsertOrder(std::deque<std::pair<Element*, Element::Index>> newInsertOrder) {
    this->selected.clear();
    this->selected.reserve(newInsertOrder.size());
    std::transform(begin(newInsertOrder), end(newInsertOrder), std::back_inserter(this->selected),
                   [](auto const& e) { return e.first; });
    this->insertOrder = std::move(newInsertOrder);
}

void EditSelectionContents::addMoveUndo(UndoRedoHandler* undo, double dx, double dy) {
    undo->addUndoAction(std::make_unique<MoveUndoAction>(this->sourceLayer, this->sourcePage, &this->selected, dx, dy,
                                                         this->sourceLayer, this->sourcePage));
    this->lastBounds.x += dx;
    this->lastBounds.y += dy;
    this->lastSnappedBounds.x += dx;
    this->lastSnappedBounds.y += dy;
}

/**
 * Returns all containing elements of this selection
 */
auto EditSelectionContents::getElements() const -> const vector<Element*>& { return this->selected; }

/**
 * Returns the insert order of this selection
 */
auto EditSelectionContents::getInsertOrder() const -> std::deque<std::pair<Element*, Element::Index>> const& {
    return this->insertOrder;
}

/**
 * Sets the tool size for pen or eraser, returs an undo action
 * (or nullptr if nothing is done)
 */
auto EditSelectionContents::setSize(ToolSize size, const double* thicknessPen, const double* thicknessHighlighter,
                                    const double* thicknessEraser) -> UndoActionPtr {
    auto undo = std::make_unique<SizeUndoAction>(this->sourcePage, this->sourceLayer);

    bool found = false;

    for (Element* e: this->selected) {
        if (e->getType() == ELEMENT_STROKE) {
            auto* s = dynamic_cast<Stroke*>(e);
            StrokeTool tool = s->getToolType();

            double originalWidth = s->getWidth();

            if (tool == StrokeTool::PEN) {
                s->setWidth(thicknessPen[size]);
            } else if (tool == StrokeTool::HIGHLIGHTER) {
                s->setWidth(thicknessHighlighter[size]);
            } else if (tool == StrokeTool::ERASER) {
                s->setWidth(thicknessEraser[size]);
            }

            if (s->hasPressure()) {
                vector<double> originalPressure = s->getPressureValues();
                vector<double> newPressure;
                newPressure.reserve(originalPressure.size());

                double factor = s->getWidth() / originalWidth;
                std::transform(originalPressure.cbegin(), originalPressure.cend(), std::back_inserter(newPressure),
                               [factor](double p) { return p * factor; });

                s->setPressureValues(newPressure);

                undo->addStroke(s, originalWidth, s->getWidth(), originalPressure, newPressure);
            } else {
                undo->addStroke(s, originalWidth, s->getWidth(), {}, {});
            }

            found = true;
        }
    }

    if (found) {
        this->deleteViewBuffer();
        this->sourceView->getXournal()->repaintSelection();

        return undo;
    }

    return nullptr;
}

/**
 * Fills the stroke, return an undo action
 * (Or nullptr if nothing done, e.g. because there is only an image)
 */
auto EditSelectionContents::setFill(int alphaPen, int alphaHighligther) -> UndoActionPtr {
    auto undo = std::make_unique<FillUndoAction>(this->sourcePage, this->sourceLayer);

    bool found = false;

    for (Element* e: this->selected) {
        if (e->getType() == ELEMENT_STROKE) {
            auto* s = dynamic_cast<Stroke*>(e);
            StrokeTool tool = s->getToolType();
            int newFill = 128;

            if (tool == StrokeTool::PEN) {
                newFill = alphaPen;
            } else if (tool == StrokeTool::HIGHLIGHTER) {
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

    return nullptr;
}

/**
 * Sets the font of all containing text elements, return an undo action
 * (or nullptr if there are no Text elements)
 */
auto EditSelectionContents::setFont(XojFont& font) -> UndoActionPtr {
    double x1 = std::numeric_limits<double>::quiet_NaN();
    double x2 = std::numeric_limits<double>::quiet_NaN();
    double y1 = std::numeric_limits<double>::quiet_NaN();
    double y2 = std::numeric_limits<double>::quiet_NaN();

    auto undo = std::make_unique<FontUndoAction>(this->sourcePage, this->sourceLayer);

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

    return nullptr;
}

/**
 * Set the line style of all strokes, return an undo action
 * (Or nullptr if nothing done)
 */
auto EditSelectionContents::setLineStyle(LineStyle style) -> UndoActionPtr {
    auto undo = std::make_unique<LineStyleUndoAction>(this->sourcePage, this->sourceLayer);

    bool found = false;

    for (Element* e: this->selected) {
        if (e->getType() == ELEMENT_STROKE) {
            auto s = static_cast<Stroke*>(e);
            auto lastLineStyle = s->getLineStyle();
            s->setLineStyle(style);
            undo->addStroke(s, lastLineStyle, s->getLineStyle());

            found = true;
        }
    }

    if (found) {
        this->deleteViewBuffer();
        this->sourceView->getXournal()->repaintSelection();

        return undo;
    }

    return nullptr;
}

/**
 * Set the color of all elements, return an undo action
 * (Or nullptr if nothing done, e.g. because there is only an image)
 */
auto EditSelectionContents::setColor(Color color) -> UndoActionPtr {
    auto undo = std::make_unique<ColorUndoAction>(this->sourcePage, this->sourceLayer);

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
    Element::Index index = as_signed(layer->getElements().size());
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
    bool rotate = (std::abs(this->rotation) > std::numeric_limits<double>::epsilon());

    double mx = bounds.x - this->originalBounds.x;
    double my = bounds.y - this->originalBounds.y;

    bool move = mx != 0 || my != 0;

    xoj_assert(this->selected.size() == this->insertOrder.size());
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
        if (index == Element::InvalidIndex) {
            // if the element didn't have a source layer (e.g, clipboard)
            layer->addElement(e);
        } else {
            layer->insertElement(e, index);
        }
    }
}

auto EditSelectionContents::getOriginalX() const -> double { return this->originalBounds.x; }

auto EditSelectionContents::getOriginalY() const -> double { return this->originalBounds.y; }

auto EditSelectionContents::getOriginalBounds() const -> Rectangle<double> {
    return Rectangle<double>{this->originalBounds};
}

auto EditSelectionContents::getSourceView() -> XojPageView* { return this->sourceView; }


void EditSelectionContents::updateContent(Rectangle<double> bounds, Rectangle<double> snappedBounds, double rotation,
                                          bool aspectRatio, Layer* layer, const PageRef& targetPage,
                                          XojPageView* targetView, UndoRedoHandler* undo, CursorSelectionType type) {
    double mx = snappedBounds.x - this->lastSnappedBounds.x;
    double my = snappedBounds.y - this->lastSnappedBounds.y;
    bool move = mx != 0 || my != 0;

    this->rotation = rotation;

    double fx = snappedBounds.width / this->lastSnappedBounds.width;
    double fy = snappedBounds.height / this->lastSnappedBounds.height;

    if (aspectRatio) {
        double f = (fx + fy) / 2;
        fx = f;
        fy = f;
    }

    bool rotate = (std::abs(this->rotation - this->lastRotation) > std::numeric_limits<double>::epsilon());
    bool scale = (snappedBounds.width != this->lastSnappedBounds.width ||
                  snappedBounds.height != this->lastSnappedBounds.height);

    if (type == CURSOR_SELECTION_MOVE && move) {
        undo->addUndoAction(std::make_unique<MoveUndoAction>(this->sourceLayer, this->sourcePage, &this->selected, mx,
                                                             my, layer, targetPage));
    } else if (type == CURSOR_SELECTION_ROTATE && rotate) {
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

    this->sourceLayer = layer;
    this->sourcePage = targetPage;
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

    if (std::abs(rotation) > std::numeric_limits<double>::epsilon()) {
        this->rotation = rotation;
    }

    if (this->crBuffer == nullptr) {
        this->crBuffer = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, static_cast<int>(std::abs(width) * zoom),
                                                    static_cast<int>(std::abs(height) * zoom));
        cairo_t* cr2 = cairo_create(this->crBuffer);

        int dx = static_cast<int>(this->relativeX * zoom);
        int dy = static_cast<int>(this->relativeY * zoom);

        cairo_translate(cr2, fx < 0 ? -width * zoom : 0, fy < 0 ? -height * zoom : 0);
        cairo_scale(cr2, fx, fy);
        cairo_translate(cr2, -dx, -dy);
        cairo_scale(cr2, zoom, zoom);

        xoj::view::ElementContainerView view(this);
        view.draw(xoj::view::Context::createDefault(cr2));

        cairo_destroy(cr2);
    }

    cairo_save(cr);

    int wImg = cairo_image_surface_get_width(this->crBuffer);
    int hImg = cairo_image_surface_get_height(this->crBuffer);

    int wTarget = static_cast<int>(std::abs(width) * zoom);
    int hTarget = static_cast<int>(std::abs(height) * zoom);

    double sx = static_cast<double>(wTarget) / wImg;
    double sy = static_cast<double>(hTarget) / hImg;

    if (wTarget != wImg || hTarget != hImg || std::abs(rotation) > std::numeric_limits<double>::epsilon()) {
        if (!this->rescaleId) {
            this->rescaleId = g_idle_add(reinterpret_cast<GSourceFunc>(repaintSelection), this);
        }
        cairo_scale(cr, sx, sy);
    }

    double dx = static_cast<int>(std::min(x, x + width) * zoom / sx);
    double dy = static_cast<int>(std::min(y, y + height) * zoom / sy);

    cairo_set_source_surface(cr, this->crBuffer, dx, dy);
    cairo_paint(cr);

    cairo_restore(cr);
}

void EditSelectionContents::serialize(ObjectOutputStream& out) const {
    out.writeObject("EditSelectionContents");

    out.writeDouble(this->originalBounds.x);
    out.writeDouble(this->originalBounds.y);
    out.writeDouble(this->originalBounds.width);
    out.writeDouble(this->originalBounds.height);

    out.writeDouble(this->lastSnappedBounds.x);
    out.writeDouble(this->lastSnappedBounds.y);
    out.writeDouble(this->lastSnappedBounds.width);
    out.writeDouble(this->lastSnappedBounds.height);

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

    double snappedX = in.readDouble();
    double snappedY = in.readDouble();
    double snappedW = in.readDouble();
    double snappedH = in.readDouble();
    this->lastSnappedBounds = Rectangle<double>{snappedX, snappedY, snappedW, snappedH};

    this->relativeX = in.readDouble();
    this->relativeY = in.readDouble();

    in.endObject();
}
