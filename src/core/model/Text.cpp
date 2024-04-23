#include "Text.h"

#include <memory>
#include <utility>  // for move

#include <glib.h>  // for g_warning
#include <pango/pangocairo.h>

#include "model/AudioElement.h"   // for AudioElement
#include "model/Element.h"        // for ELEMENT_TEXT, Eleme...
#include "model/Font.h"           // for XojFont
#include "pdf/base/XojPdfPage.h"  // for XojPdfRectangle
#include "util/Rectangle.h"       // for Rectangle
#include "util/Stacktrace.h"      // for Stacktrace
#include "util/StringUtils.h"
#include "util/raii/GObjectSPtr.h"
#include "util/raii/PangoSPtr.h"                  // for PangoAttrListSPtr
#include "util/serializing/ObjectInputStream.h"   // for ObjectInputStream
#include "util/serializing/ObjectOutputStream.h"  // for ObjectOutputStream

using xoj::util::Rectangle;

Text::Text(): AudioElement(ELEMENT_TEXT) {
    this->font.setName("Sans");
    this->font.setSize(12);
    this->attributes = xoj::util::PangoAttrListSPtr(pango_attr_list_new(), xoj::util::adopt);
}

Text::~Text() = default;

auto Text::cloneText() const -> std::unique_ptr<Text> {
    auto text = std::make_unique<Text>();
    text->font = this->font;
    text->text = this->text;
    text->setColor(this->getColor());
    text->x = this->x;
    text->y = this->y;
    text->width = this->width;
    text->height = this->height;
    text->cloneAudioData(this);
    text->snappedBounds = this->snappedBounds;
    text->sizeCalculated = this->sizeCalculated;
    text->inEditing = this->inEditing;
    text->alignment = this->alignment;
    text->attributes = this->attributes;

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
    sizeCalculated = false;
}

void Text::calcSize() const {
    auto layout = createPangoLayout();
    pango_layout_set_text(layout.get(), this->text.c_str(), static_cast<int>(this->text.length()));
    int w = 0;
    int h = 0;
    pango_layout_get_size(layout.get(), &w, &h);
    this->width = (static_cast<double>(w)) / PANGO_SCALE;
    this->height = (static_cast<double>(h)) / PANGO_SCALE;
    this->updateSnapping();
}

void Text::setWidth(double width) {
    this->width = width;
    this->updateSnapping();
}

void Text::setHeight(double height) {
    this->height = height;
    this->updateSnapping();
}

void Text::setInEditing(bool inEditing) { this->inEditing = inEditing; }

auto Text::createPangoLayout() const -> xoj::util::GObjectSPtr<PangoLayout> {
    xoj::util::GObjectSPtr<PangoContext> c(pango_font_map_create_context(pango_cairo_font_map_get_default()),
                                           xoj::util::adopt);
    xoj::util::GObjectSPtr<PangoLayout> layout(pango_layout_new(c.get()), xoj::util::adopt);

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

    pango_layout_set_attributes(layout, this->attributes.get());

    PangoAlignment alignment = static_cast<PangoAlignment>(this->alignment);
    pango_layout_set_alignment(layout, alignment);
}

void Text::scale(double x0, double y0, double fx, double fy, double rotation,
                 bool) {  // line width scaling option is not used
    // only proportional scale allowed...
    if (fx != fy) {
        g_warning("rescale font with fx != fy not supported: %lf / %lf", fx, fy);
        Stacktrace::printStracktrace();
    }

    this->x -= x0;
    this->x *= fx;
    this->x += x0;
    this->y -= y0;
    this->y *= fy;
    this->y += y0;

    double size = this->font.getSize() * fx;
    this->font.setSize(size);

    sizeCalculated = false;
}

void Text::rotate(double x0, double y0, double th) {}

auto Text::isInEditing() const -> bool { return this->inEditing; }

auto Text::rescaleOnlyAspectRatio() -> bool { return true; }

auto Text::intersects(double x, double y, double halfEraserSize) const -> bool {
    return intersects(x, y, halfEraserSize, nullptr);
}

auto Text::intersects(double x, double y, double halfEraserSize, double* gap) const -> bool {
    double x1 = this->x - halfEraserSize;
    double x2 = this->x + this->getElementWidth() + halfEraserSize;
    double y1 = this->y - halfEraserSize;
    double y2 = this->y + this->getElementHeight() + halfEraserSize;

    return x >= x1 && x <= x2 && y >= y1 && y <= y2;
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
}

void Text::updateSnapping() const {
    this->snappedBounds = Rectangle<double>(this->x, this->y, this->width, this->height);
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

void Text::setAlignment(TextAlignment align) { this->alignment = align; }

TextAlignment Text::getAlignment() const { return this->alignment; }

xoj::util::PangoAttrListSPtr Text::getAttributeList() const { return this->attributes; };

void Text::addAttribute(PangoAttribute* attrib) {
    pango_attr_list_change(this->attributes.get(), attrib);
    this->calcSize();
}

void Text::clearAttributes() {
    this->attributes = xoj::util::PangoAttrListSPtr(pango_attr_list_new(), xoj::util::adopt);
    this->calcSize();
}

void Text::updateTextAttributesPosition(int pos, int del, int add) {
    pango_attr_list_update(this->attributes.get(), pos, del, add);
    this->calcSize();
}

void Text::replaceAttributes(xoj::util::PangoAttrListSPtr newAttributes) {
    this->attributes = newAttributes;
    this->calcSize();
}
