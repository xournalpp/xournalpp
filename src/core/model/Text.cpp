#include "Text.h"

#include <memory>
#include <utility>  // for move

#include <glib.h>  // for g_warning
#include <pango/pangocairo.h>

#include "model/AudioContent.h"  // for AudioContent
#include "model/Element.h"        // for ELEMENT_TEXT, Eleme...
#include "model/Font.h"           // for XojFont
#include "pdf/base/XojPdfPage.h"  // for XojPdfRectangle
#include "util/Rectangle.h"       // for Rectangle
#include "util/Stacktrace.h"      // for Stacktrace
#include "util/StringUtils.h"
#include "util/matrix/RectangleMultiply.h"
#include "util/raii/GObjectSPtr.h"
#include "util/safe_casts.h"                      // for round_cast
#include "util/serializing/ObjectInputStream.h"   // for ObjectInputStream
#include "util/serializing/ObjectOutputStream.h"  // for ObjectOutputStream

using xoj::util::Rectangle;

Text::Text(): RectangularElement(ELEMENT_TEXT) {
    this->font.setName("Sans");
    this->font.setSize(12);
}

Text::~Text() = default;

auto Text::cloneText() const -> std::unique_ptr<Text> {
    auto text = std::make_unique<Text>();
    static_cast<RectangularElement&>(*text) = *this;
    static_cast<AudioContent&>(*text) = *this;

    text->font = this->font;
    text->text = this->text;
    text->inEditing = this->inEditing;
    text->wrapWidth = this->wrapWidth;
    text->align = this->align;
    text->justify = this->justify;

    return text;
}

auto Text::clone() const -> ElementPtr { return cloneText(); }

auto Text::getFont() -> XojFont& { return font; }
auto Text::getFont() const -> const XojFont& { return font; }

void Text::setFont(const XojFont& font) {
    this->font = font;
    sizeCalculated = false;
}

auto Text::getFontSize() const -> double { return font.getSize(); }

auto Text::getFontName() const -> std::string { return font.getName(); }

auto Text::getText() const -> const std::string& { return this->text; }

void Text::setText(std::string text) {
    this->text = std::move(text);
    sizeCalculated = false;
}

void Text::setWrap(double wrap) {
    this->wrapWidth = wrap;
    sizeCalculated = false;
}

void Text::setAlignment(TextAlignment a) {
    this->align = a;
    sizeCalculated = false;
}

Text::Boxes Text::computeBoxesForLayout(PangoLayout* layout, double wrapWidth) {
    PangoRectangle box;
    pango_layout_get_extents(layout, nullptr, &box);

    xoj::util::Point<double> offset{static_cast<double>(box.x) / PANGO_SCALE, static_cast<double>(box.y) / PANGO_SCALE};

    Boxes res;

    res.effectiveBounds.width = static_cast<double>(box.width) / PANGO_SCALE;
    res.effectiveBounds.height = static_cast<double>(box.height) / PANGO_SCALE;
    res.effectiveBounds.x = offset.x;
    res.effectiveBounds.y = offset.y;

    if (wrapWidth != NO_WRAP) {
        res.theoreticalSize.width = wrapWidth;
    } else {
        res.theoreticalSize.width = res.effectiveBounds.width + offset.x;
    }
    res.theoreticalSize.height = res.effectiveBounds.height + offset.y;

    return res;
}

void Text::calcSize() const {
    auto layout = createPangoLayout();
    pango_layout_set_text(layout.get(), this->text.c_str(), static_cast<int>(this->text.length()));

    auto boxes = computeBoxesForLayout(layout.get(), this->wrapWidth);
    this->naturalSize = boxes.theoreticalSize;
    this->effectiveBounds = boxes.effectiveBounds;

    const auto& matrix = this->getTransformation();
    this->boundingBox = matrix * this->effectiveBounds;
    this->snappedBounds = matrix * xoj::util::Rectangle<double>{{0, 0}, this->naturalSize};

    this->sizeCalculated = true;
}

void Text::setInEditing(bool inEditing) { this->inEditing = inEditing; }

auto Text::createPangoLayout() const -> xoj::util::GObjectSPtr<PangoLayout> {
    xoj::util::GObjectSPtr<PangoContext> c(pango_font_map_create_context(pango_cairo_font_map_get_default()),
                                           xoj::util::adopt);
    pango_context_set_round_glyph_positions(c.get(), false);  // Avoid weird glyph positioning on small fonts
    xoj::util::GObjectSPtr<PangoLayout> layout(pango_layout_new(c.get()), xoj::util::adopt);

    pango_layout_set_width(layout.get(),
                           this->wrapWidth == NO_WRAP ? -1 : round_cast<int>(this->wrapWidth * PANGO_SCALE));

    pango_layout_set_justify(layout.get(), this->justify);
    pango_layout_set_alignment(layout.get(), this->align.toPango());

#if PANGO_VERSION_CHECK(1, 48, 5)  // see https://gitlab.gnome.org/GNOME/pango/-/issues/499
    pango_layout_set_line_spacing(layout.get(), 1.0);
#endif

    updatePangoFont(layout.get());

    return layout;
}

void Text::updatePangoFont(PangoLayout* layout) const {
    PangoFontDescription* desc = pango_font_description_from_string(this->getFontName().c_str());
    pango_font_description_set_absolute_size(desc, this->getFontSize() * PANGO_SCALE);

    pango_layout_set_font_description(layout, desc);
    pango_font_description_free(desc);
}

auto Text::isInEditing() const -> bool { return this->inEditing; }

void Text::serialize(ObjectOutputStream& out) const {
    out.writeObject("Text");

    this->RectangularElement::serialize(out);
    this->AudioContent::serialize(out);

    out.writeString(this->text);

    font.serialize(out);

    out.writeDouble(this->wrapWidth);
    out.writeInt(static_cast<int>(this->align));
    out.writeInt(this->justify);

    out.endObject();
}

void Text::readSerialized(ObjectInputStream& in) {
    in.readObject("Text");

    this->RectangularElement::readSerialized(in);
    this->AudioContent::readSerialized(in);

    this->text = in.readString();

    font.readSerialized(in);

    this->wrapWidth = in.readDouble();
    this->align = static_cast<TextAlignment::Value>(in.readInt());
    this->align.validate();
    this->justify = in.readInt() != 0;

    in.endObject();
}

auto Text::findText(const std::string& search) const -> std::vector<XojPdfRectangle> {
    size_t patternLength = search.length();
    if (patternLength == 0) {
        return {};
    }

    auto layout = this->createPangoLayout();
    pango_layout_set_text(layout.get(), this->text.c_str(), static_cast<int>(this->text.length()));


    std::string text = StringUtils::toLowerCase(this->text);
    std::string pattern = StringUtils::toLowerCase(search);

    const auto& origin = this->getOrigin();

    std::vector<XojPdfRectangle> list;

    for (size_t pos = text.find(pattern); pos != std::string::npos; pos = text.find(pattern, pos + 1)) {
        XojPdfRectangle mark;
        PangoRectangle rect = {0};
        pango_layout_index_to_pos(layout.get(), static_cast<int>(pos), &rect);
        mark.x1 = (static_cast<double>(rect.x)) / PANGO_SCALE + origin.x;
        mark.y1 = (static_cast<double>(rect.y)) / PANGO_SCALE + origin.y;

        pango_layout_index_to_pos(layout.get(), static_cast<int>(pos + patternLength - 1), &rect);
        mark.x2 = (static_cast<double>(rect.x) + rect.width) / PANGO_SCALE + origin.x;
        mark.y2 = (static_cast<double>(rect.y) + rect.height) / PANGO_SCALE + origin.y;

        list.push_back(mark);
    }

    return list;
}
