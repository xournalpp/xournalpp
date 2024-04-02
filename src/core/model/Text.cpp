#include "Text.h"

#include <memory>
#include <string_view>
#include <utility>  // for move

#include <glib.h>  // for g_warning
#include <pango/pangocairo.h>

#include "model/AudioElement.h"   // for AudioElement
#include "model/Element.h"        // for ELEMENT_TEXT, Eleme...
#include "model/Font.h"           // for XojFont
#include "pdf/base/XojPdfPage.h"  // for XojPdfRectangle
#include "util/Point.h"
#include "util/Rectangle.h"       // for Rectangle
#include "util/Stacktrace.h"      // for Stacktrace
#include "util/StringUtils.h"
#include "util/raii/GObjectSPtr.h"
#include "util/serializing/ObjectInputStream.h"   // for ObjectInputStream
#include "util/serializing/ObjectOutputStream.h"  // for ObjectOutputStream

using xoj::util::Rectangle;

static void updatePangoFont(PangoLayout* layout, std::string const& fontName, double fontSize) {
    PangoFontDescription* desc = pango_font_description_from_string(fontName.c_str());
    pango_font_description_set_absolute_size(desc, fontSize * PANGO_SCALE);

    pango_layout_set_font_description(layout, desc);
    pango_font_description_free(desc);
}

static auto createPangoLayoutInternal() -> xoj::util::GObjectSPtr<PangoLayout> {
    xoj::util::GObjectSPtr<PangoContext> c(pango_font_map_create_context(pango_cairo_font_map_get_default()),
                                           xoj::util::adopt);
    xoj::util::GObjectSPtr<PangoLayout> layout(pango_layout_new(c.get()), xoj::util::adopt);

#if PANGO_VERSION_CHECK(1, 48, 5)  // see https://gitlab.gnome.org/GNOME/pango/-/issues/499
    pango_layout_set_line_spacing(layout.get(), 1.0);
#endif
    return layout;
}

auto static calculateSize(PangoLayout* layout) -> std::pair<double, double> {
    int w = 0;
    int h = 0;
    pango_layout_get_size(layout, &w, &h);
    return {static_cast<double>(w) / PANGO_SCALE, static_cast<double>(h) / PANGO_SCALE};
}

Text::Text(): AudioElement(ELEMENT_TEXT) {}

Text::~Text() = default;

auto Text::cloneText() const -> std::unique_ptr<Text> {
    auto text = std::make_unique<Text>();
    text->font = this->font;
    text->text = this->text;
    text->setColor(this->getColor());
    text->sizes = this->sizes;
    text->cloneAudioData(this);
    text->inEditing = this->inEditing;

    return text;
}

auto Text::clone() const -> ElementPtr { return cloneText(); }

auto Text::getFont() -> XojFont& { return font; }

void Text::setFont(const XojFont& font) { this->font = font; }

auto Text::getFontSize() const -> double { return font.getSize(); }

auto Text::getFontName() const -> std::string { return font.getName(); }

auto Text::getText() const -> const std::string& { return this->text; }

void Text::setText(std::string text) {
    this->text = std::move(text);
    pango_layout_set_text(layout.get(), this->text.c_str(), static_cast<int>(this->text.length()));
    calculateSize(layout.get());
}

auto Text::internalUpdateBounds() const -> std::pair<Rectangle<double>, Rectangle<double>> {
    auto bounds = Rectangle<double>(0, 0, sizes.first, sizes.first);
    return {bounds, bounds};
}

void Text::setWidth(double width) {
    auto factor = width / this->getElementWidth();
    this->scale({0, 0}, factor, 1, false);
}

void Text::setHeight(double height) {
    auto factor = height / this->getElementHeight();
    this->scale({0, 0}, 1, factor, false);
}

void Text::setInEditing(bool inEditing) { this->inEditing = inEditing; }

void Text::scale(xoj::util::Point<double> base, double fx, double fy, bool) {  // line width scaling option is not used
    // only proportional scale allowed...
    if (fx != fy) {
        g_warning("rescale font with fx != fy not supported: %lf / %lf", fx, fy);
        Stacktrace::printStracktrace();
    }

    double size = this->font.getSize() * fx;
    this->font.setSize(size);
    ::updatePangoFont(layout.get(), getFontName(), getFontSize());
}

auto Text::isInEditing() const -> bool { return this->inEditing; }

auto Text::rescaleOnlyAspectRatio() -> bool { return true; }

auto Text::intersects(xoj::util::Point<double> pos, double halfEraserSize) const -> bool {
    return intersects(pos, halfEraserSize, nullptr);
}

auto Text::intersects(xoj::util::Point<double> pos, double halfEraserSize, double* gap) const -> bool {
    return boundingRect()
            .intersects({pos.x - halfEraserSize, pos.y - halfEraserSize, 2 * halfEraserSize, 2 * halfEraserSize})
            .has_value();
}

void Text::serialize(ObjectOutputStream& out) const {
    out.writeObject("Text");

    this->AudioElement::serialize(out);

    out.writeString(this->text);

    font.serialize(out);

    out.endObject();
}

void Text::readSerialized(ObjectInputStream& in) {
    in.readObject("Text");

    this->AudioElement::readSerialized(in);

    this->text = in.readString();

    font.readSerialized(in);

    in.endObject();
    this->layout = createPangoLayoutInternal();
}

auto Text::findText(const std::string& search) const -> std::vector<XojPdfRectangle> {
    size_t patternLength = search.length();
    if (patternLength == 0) {
        return {};
    }

    pango_layout_set_text(this->layout.get(), this->text.c_str(), static_cast<int>(this->text.length()));

    std::string text = StringUtils::toLowerCase(this->text);

    std::string pattern = StringUtils::toLowerCase(search);

    std::vector<XojPdfRectangle> list;

    for (size_t pos = text.find(pattern); pos != std::string::npos; pos = text.find(pattern, pos + 1)) {
        XojPdfRectangle mark;
        PangoRectangle rect = {0};
        pango_layout_index_to_pos(layout.get(), static_cast<int>(pos), &rect);
        mark.x1 = (static_cast<double>(rect.x)) / PANGO_SCALE + this->getX();
        mark.y1 = (static_cast<double>(rect.y)) / PANGO_SCALE + this->getY();

        pango_layout_index_to_pos(layout.get(), static_cast<int>(pos + patternLength - 1), &rect);
        mark.x2 = (static_cast<double>(rect.x) + rect.width) / PANGO_SCALE + this->getX();
        mark.y2 = (static_cast<double>(rect.y) + rect.height) / PANGO_SCALE + this->getY();

        list.push_back(mark);
    }

    return list;
}
auto Text::getPangoLayout() const -> xoj::util::GObjectSPtr<PangoLayout> { return this->layout; }
