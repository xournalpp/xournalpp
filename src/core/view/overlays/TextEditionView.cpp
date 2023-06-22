#include "TextEditionView.h"

#include <tuple>

#include "control/tools/TextEditor.h"
#include "model/Text.h"
#include "util/Color.h"
#include "util/raii/CairoWrappers.h"
#include "view/Repaintable.h"

using namespace xoj::view;

TextEditionView::TextEditionView(const TextEditor* textEditor, Repaintable* parent):
        ToolView(parent), textEditor(textEditor) {
    this->registerToPool(textEditor->getViewPool());
    this->on(FLAG_DIRTY_REGION, textEditor->getContentBoundingBox());
}

TextEditionView::~TextEditionView() noexcept { this->unregisterFromPool(); }

void TextEditionView::draw(cairo_t* cr) const {
    xoj::util::CairoSaveGuard saveGuard(cr);

    // Draw the frame
    double zoom = parent->getZoom();
    cairo_set_line_width(cr, BORDER_WIDTH_IN_PIXELS / zoom);
    Util::cairo_set_source_argb(cr, this->textEditor->getSelectionColor());
    Range frame = textEditor->getContentBoundingBox();
    frame.addPadding(PADDING_IN_PIXELS / zoom);
    cairo_rectangle(cr, frame.getX(), frame.getY(), frame.getWidth(), frame.getHeight());
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

    // From now on, coordinates are in textElement coordinates
    cairo_translate(cr, textElement->getX(), textElement->getY());

    // The data is owned by textEditor
    PangoLayout* layout = this->textEditor->getUpToDateLayout();

    // The cairo context might have changed. Update the pango layout
    pango_cairo_update_layout(cr, layout);

    pango_context_set_matrix(pango_layout_get_context(layout), nullptr);

    Util::cairo_set_source_rgbi(cr, textElement->getColor());
    pango_cairo_show_layout(cr, layout);

    auto selection_opt = this->textEditor->getCurrentSelection();
    if (selection_opt.has_value()) {
        PangoRectangle rect;
        auto color = this->textEditor->getSelectionColor();
        auto selection = selection_opt.value();
        for (int i = std::get<0>(selection); i < std::get<1>(selection); i++) {
            pango_layout_index_to_pos(layout, i, &rect);
            Util::cairo_set_source_rgbi(cr, color, 0.6);
            cairo_rectangle(cr, double(rect.x) / PANGO_SCALE, double(rect.y) / PANGO_SCALE,
                            double(rect.width) / PANGO_SCALE, double(rect.height) / PANGO_SCALE);
            cairo_fill(cr);
        }
    }
}

bool TextEditionView::isViewOf(const OverlayBase* overlay) const { return overlay == this->textEditor; }

auto TextEditionView::toWindowCoordinates(const xoj::util::Rectangle<double>& r) const -> xoj::util::Rectangle<double> {
    auto* textElement = this->textEditor->getTextElement();
    return parent->toWindowCoordinates(r.translated(textElement->getX(), textElement->getY()));
}

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
