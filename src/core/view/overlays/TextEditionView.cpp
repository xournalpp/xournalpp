#include "TextEditionView.h"

#include "control/tools/TextEditor.h"
#include "model/Text.h"
#include "util/Color.h"
#include "util/Matrix.h"
#include "util/raii/CairoWrappers.h"
#include "view/Repaintable.h"

using namespace xoj::view;

static constexpr double EPSILON = 1e-4;
static constexpr double WRAP_LINE_DASH_LENGTH = 2.;

static double computeMargin(const TextEditor* te) {
    const auto& m = te->getTextElement()->getTransformation();
    const auto v = m.applyToVector(xoj::util::Point<double>(1, 1));
    const auto w = m.applyToVector(xoj::util::Point<double>(1, -1));
    return std::max({std::abs(v.x), std::abs(w.x), std::abs(v.y), std::abs(w.y)}) *
           std::max(TextEditionView::BORDER_WIDTH_IN_PIXELS + TextEditionView::PADDING_IN_PIXELS,
                    TextEditionView::INSERTION_CURSOR_WIDTH_IN_PIXELS);
}

TextEditionView::TextEditionView(const TextEditor* textEditor, Repaintable* parent):
        ToolView(parent), textEditor(textEditor), repaintMargin(computeMargin(textEditor)) {
    this->registerToPool(textEditor->getViewPool());
    textEditor->onViewCreation();
    this->on(FLAG_DIRTY_REGION, textEditor->getContentBoundingBox());
}

TextEditionView::~TextEditionView() noexcept { this->unregisterFromPool(); }

/**
 * Add the three segments {p.x, p.y} -- {q.x, p.y} -- {q.x, q.y} -- {p.x, q.y} to cairo.
 * Adding cairo_close_path() afterwards would get you a rectangle with opposite corners p and q
 */
static inline void addThreeSegments(cairo_t* cr, const xoj::util::Point<double>& p, const xoj::util::Point<double>& q) {
    cairo_move_to(cr, p.x, p.y);
    cairo_line_to(cr, q.x, p.y);
    cairo_line_to(cr, q.x, q.y);
    cairo_line_to(cr, p.x, q.y);
}

void TextEditionView::draw(cairo_t* cr) const {
    xoj::util::CairoSaveGuard saveGuard(cr);

    const double zoom = parent->getZoom();
    cairo_set_line_width(cr, BORDER_WIDTH_IN_PIXELS / zoom);
    Util::cairo_set_source_argb(cr, this->textEditor->getSelectionColor());
    const auto& boxes = textEditor->getBoxes();


    // We will apply the matrix be hand to the points when drawing the box(es), so that the line width is not deformed
    const auto& m = this->textEditor->getTextElement()->getTransformation();

    // Compute the padding so it does not scale with the text
    const double padding = PADDING_IN_PIXELS / zoom;
    const double horizontalScaling = m.applyToVector(xoj::util::Point<double>(1, 0)).distance({0, 0});
    xoj::util::Point<double> relativePadding(
            padding / horizontalScaling, padding / m.applyToVector(xoj::util::Point<double>(0, 1)).distance({0, 0}));

    cairo_matrix_t originalMatrix;
    cairo_get_matrix(cr, &originalMatrix);

    if (auto w = this->textEditor->getCurrentWrapWidth();
        w != Text::NO_WRAP && w < boxes.effectiveBounds.width - EPSILON) {
        xoj::util::CairoSaveGuard guard(cr);
        // The text goes beyond the wrap limit: draw a dashed effective box
        cairo_matrix_t originalMatrix;
        cairo_get_matrix(cr, &originalMatrix);
        m.transformCairo(cr);
        auto al = this->textEditor->getTextElement()->getAlign();
        if (al != TextAlignment::RIGHT) {
            addThreeSegments(cr, {boxes.theoreticalSize.width + relativePadding.x, boxes.effectiveBounds.y},
                             {boxes.effectiveBounds.x + boxes.effectiveBounds.width,
                              boxes.effectiveBounds.y + boxes.effectiveBounds.height});
        }
        if (al != TextAlignment::LEFT) {
            addThreeSegments(cr, {-relativePadding.x, boxes.effectiveBounds.y},
                             {boxes.effectiveBounds.x, boxes.effectiveBounds.y + boxes.effectiveBounds.height});
        }
        cairo_set_matrix(cr, &originalMatrix);  // Reset the matrix before stroking to have the right line width/dashes
        cairo_set_dash(cr, &WRAP_LINE_DASH_LENGTH, 1, 0.);
        cairo_stroke(cr);
    }

    // Hard box
    m.transformCairo(cr);
    cairo_rectangle(cr, -relativePadding.x, -relativePadding.y, boxes.theoreticalSize.width + 2 * relativePadding.x,
                    boxes.theoreticalSize.height + 2 * relativePadding.y);
    cairo_set_matrix(cr, &originalMatrix);  // Reset the matrix before stroking to have the right line width
    cairo_stroke(cr);

    // Draw the text itself
    this->drawWithoutDrawingAids(cr);

    // Draw the cursor
    if (this->textEditor->isCursorVisible()) {
        auto cursorBox = this->textEditor->getCursorBox();
        if (cursorBox.getWidth() == 0.0) {
            const double shift = 0.5 * INSERTION_CURSOR_WIDTH_IN_PIXELS / zoom / horizontalScaling;
            cursorBox.minX -= shift;
            cursorBox.maxX += shift;
        }
        cairo_set_operator(cr, CAIRO_OPERATOR_DIFFERENCE);
        cairo_set_source_rgb(cr, 1, 1, 1);
        m.transformCairo(cr);
        cairo_rectangle(cr, cursorBox.minX, cursorBox.minY, cursorBox.getWidth(), cursorBox.getHeight());
        cairo_fill(cr);
    }
}

void TextEditionView::drawWithoutDrawingAids(cairo_t* cr) const {
    xoj::util::CairoSaveGuard saveGuard(cr);

    const Text* textElement = this->textEditor->getTextElement();
    Util::cairo_set_source_rgbi(cr, textElement->getColor());

    // From now on, coordinates are in textElement coordinates
    textElement->getTransformation().transformCairo(cr);

    // The data is owned by textEditor
    PangoLayout* layout = this->textEditor->getUpToDateLayout();

    // The cairo context might have changed. Update the pango layout
    pango_cairo_update_layout(cr, layout);

    pango_context_set_matrix(pango_layout_get_context(layout), nullptr);

    pango_cairo_show_layout(cr, layout);
}

bool TextEditionView::isViewOf(const OverlayBase* overlay) const { return overlay == this->textEditor; }

auto TextEditionView::toWidgetCoordinates(const xoj::util::Point<double>& r) const -> xoj::util::Point<double> {
    return parent->toWidgetCoordinates(r);
}
auto TextEditionView::toWidgetCoordinates(const xoj::util::Rectangle<double>& r) const -> xoj::util::Rectangle<double> {
    return parent->toWidgetCoordinates(r);
}

auto TextEditionView::getZoom() const -> double { return parent->getZoom(); }

void TextEditionView::on(TextEditionView::FlagDirtyRegionRequest, Range rg) {
    rg.addPadding(this->repaintMargin / this->parent->getZoom());
    this->parent->flagDirtyRegion(rg);
}

void TextEditionView::deleteOn(TextEditionView::FinalizationRequest, Range rg) {
    rg.addPadding(this->repaintMargin / this->parent->getZoom());
    this->parent->drawAndDeleteToolView(this, rg);
}
