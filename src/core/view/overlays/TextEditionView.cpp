#include "TextEditionView.h"

#include "control/tools/TextEditor.h"
#include "model/Text.h"
#include "util/Color.h"
#include "util/raii/CairoWrappers.h"
#include "view/Repaintable.h"

using namespace xoj::view;

static constexpr double EPSILON = 1e-4;
static constexpr double WRAP_LINE_DASH_LENGTH = 1.;

TextEditionView::TextEditionView(const TextEditor* textEditor, Repaintable* parent):
        ToolView(parent), textEditor(textEditor) {
    this->registerToPool(textEditor->getViewPool());
    textEditor->onViewCreation();
    this->on(FLAG_DIRTY_REGION, textEditor->getContentBoundingBox());
}

TextEditionView::~TextEditionView() noexcept { this->unregisterFromPool(); }

void TextEditionView::draw(cairo_t* cr) const {
    xoj::util::CairoSaveGuard saveGuard(cr);

    double zoom = parent->getZoom();
    cairo_set_line_width(cr, BORDER_WIDTH_IN_PIXELS / zoom);
    Util::cairo_set_source_argb(cr, this->textEditor->getSelectionColor());
    Range box = textEditor->getContentBoundingBox();

    // Draw the wrap limit line if needed
    if (auto w = this->textEditor->getCurrentWrapWidth(); w != Text::NO_WRAP && w < box.getWidth() - EPSILON) {
        xoj::util::CairoSaveGuard saveGuard(cr);
        cairo_move_to(cr, box.getX() + this->textEditor->getCurrentWrapWidth(), box.getY());
        cairo_rel_line_to(cr, 0, box.getHeight());
        cairo_set_dash(cr, &WRAP_LINE_DASH_LENGTH, 1, 0.);
        cairo_stroke(cr);
    }

    // Draw the frame
    box.addPadding(PADDING_IN_PIXELS / zoom);
    cairo_rectangle(cr, box.getX(), box.getY(), box.getWidth(), box.getHeight());
    cairo_stroke(cr);


    // Draw the text itself
    this->drawWithoutDrawingAids(cr);

    // Draw the cursor
    if (this->textEditor->isCursorVisible()) {
        auto cursorBox = this->textEditor->getCursorBox();
        if (cursorBox.getWidth() == 0.0) {
            const double shift = 0.5 * INSERTION_CURSOR_WIDTH_IN_PIXELS / zoom;
            cursorBox.minX -= shift;
            cursorBox.maxX += shift;
        }
        cairo_set_operator(cr, CAIRO_OPERATOR_DIFFERENCE);
        cairo_set_source_rgb(cr, 1, 1, 1);
        const Text* textElement = this->textEditor->getTextElement();
        cairo_rectangle(cr, cursorBox.minX + textElement->getX(), cursorBox.minY + textElement->getY(),
                        cursorBox.getWidth(), cursorBox.getHeight());
        cairo_fill(cr);
    }
}

void TextEditionView::drawWithoutDrawingAids(cairo_t* cr) const {
    xoj::util::CairoSaveGuard saveGuard(cr);

    const Text* textElement = this->textEditor->getTextElement();
    Util::cairo_set_source_rgbi(cr, textElement->getColor());

    // From now on, coordinates are in textElement coordinates
    cairo_translate(cr, textElement->getX(), textElement->getY());

    // The data is owned by textEditor
    PangoLayout* layout = this->textEditor->getUpToDateLayout();

    // The cairo context might have changed. Update the pango layout
    pango_cairo_update_layout(cr, layout);

    pango_context_set_matrix(pango_layout_get_context(layout), nullptr);

    pango_cairo_show_layout(cr, layout);
}

bool TextEditionView::isViewOf(const OverlayBase* overlay) const { return overlay == this->textEditor; }

auto TextEditionView::toWidgetCoordinates(const xoj::util::Rectangle<double>& r) const -> xoj::util::Rectangle<double> {
    return parent->toWidgetCoordinates(r);
}

auto TextEditionView::getZoom() const -> double { return parent->getZoom(); }

void TextEditionView::on(TextEditionView::FlagDirtyRegionRequest, Range rg) {
    const double padding = std::max(BORDER_WIDTH_IN_PIXELS + PADDING_IN_PIXELS, INSERTION_CURSOR_WIDTH_IN_PIXELS) /
                           this->parent->getZoom();
    rg.addPadding(padding);
    this->parent->flagDirtyRegion(rg);
}

void TextEditionView::deleteOn(TextEditionView::FinalizationRequest, Range rg) {
    const double padding = std::max(BORDER_WIDTH_IN_PIXELS + PADDING_IN_PIXELS, INSERTION_CURSOR_WIDTH_IN_PIXELS) /
                           this->parent->getZoom();
    rg.addPadding(padding);
    this->parent->drawAndDeleteToolView(this, rg);
}
