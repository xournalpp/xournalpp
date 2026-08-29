#include "Link.h"

#include "model/Font.h"                           // for XojFont
#include "util/Color.h"                           // for Colors
#include "util/Point.h"                           // for Point
#include "util/Rectangle.h"                       // for Rectangle
#include "util/Stacktrace.h"                      // for Stacktrace
#include "util/serializing/ObjectInputStream.h"   // for ObjectInputStream
#include "util/serializing/ObjectOutputStream.h"  // for ObjectOutputStream

#include "TextAlignment.h"

Link::Link(): RectangularElement(ELEMENT_LINK) {
    this->font.setName("Sans");
    this->font.setSize(12);
    this->setColor(Colors::magenta);
    this->text = "Hello World";
}

void Link::setText(std::string text) { this->text = text; }

std::string Link::getText() const { return this->text; }

void Link::setTextPos(double x, double y) {
    this->snappedBounds.x = x;
    this->snappedBounds.y = y;
    this->sizeCalculated = false;
}

void Link::setUrl(std::string url) { this->url = url; }

std::string Link::getUrl() const { return this->url; }

void Link::setFont(const XojFont& font) { this->font = font; }

auto Link::getFont() -> XojFont& { return this->font; }

auto Link::getFont() const -> const XojFont& { return this->font; }

void Link::serialize(ObjectOutputStream& out) const {
    out.writeObject("Link");

    RectangularElement::serialize(out);

    out.writeString(this->text);

    out.writeString(this->url);

    out.writeInt(this->alignment);

    font.serialize(out);

    out.endObject();
}

void Link::readSerialized(ObjectInputStream& in) {
    in.readObject("Link");

    RectangularElement::readSerialized(in);

    this->text = in.readString();

    this->url = in.readString();

    this->alignment = static_cast<TextAlignment::Value>(in.readInt());
    this->alignment.validate();

    font.readSerialized(in);

    in.endObject();
}

void Link::scale(double x0, double y0, double fx, double fy, double rotation, bool restoreLineWidth) {
    // only proportional scale allowed...
    if (fx != fy) {
        g_warning("rescale font with fx != fy not supported: %lf / %lf", fx, fy);
        Stacktrace::printStacktrace();
    }

    this->snappedBounds.x = (this->snappedBounds.x - x0) * fx + x0;
    this->snappedBounds.y = (this->snappedBounds.y - y0) * fy + y0;

    double size = this->font.getSize() * fx;
    this->font.setSize(size);

    sizeCalculated = false;
};

ElementPtr Link::clone() const {
    auto link = std::make_unique<Link>();
    static_cast<RectangularElement&>(*link) = *this;

    link->font = this->font;
    link->text = this->text;
    link->url = this->url;
    return link;
};

void Link::calcSize() const {
    auto layout = this->createPangoLayout();
    pango_layout_set_text(layout.get(), this->text.c_str(), static_cast<int>(this->text.length()));
    int w = 0, h = 0;
    pango_layout_get_size(layout.get(), &w, &h);
    this->snappedBounds.width = static_cast<double>(w) / PANGO_SCALE;
    this->snappedBounds.height = static_cast<double>(h) / PANGO_SCALE;
    this->boundingBox = xoj::util::Rectangle<double>(this->snappedBounds.x - PADDING, this->snappedBounds.y - PADDING,
                                                     this->snappedBounds.width + 2 * PADDING,
                                                     this->snappedBounds.height + 2 * PADDING);
    sizeCalculated = true;
};

auto Link::createPangoLayout() const -> xoj::util::GObjectSPtr<PangoLayout> {
    xoj::util::GObjectSPtr<PangoContext> context(pango_context_new(), xoj::util::adopt);
    pango_context_set_font_map(context.get(), pango_cairo_font_map_get_default());
    xoj::util::GObjectSPtr<PangoLayout> layout(pango_layout_new(context.get()), xoj::util::adopt);
    PangoAttrList* attrList = pango_attr_list_new();
    pango_attr_list_insert(attrList, pango_attr_underline_new(PANGO_UNDERLINE_SINGLE));
    pango_layout_set_attributes(layout.get(), attrList);

    PangoFontDescription* font = pango_font_description_from_string(this->font.getName().c_str());
    pango_font_description_set_absolute_size(font, this->font.getSize() * PANGO_SCALE);
    pango_layout_set_font_description(layout.get(), font);
    pango_font_description_free(font);

    pango_layout_set_alignment(layout.get(), alignment.toPango());

    return layout;
}

auto Link::rescaleOnlyAspectRatio() const -> bool { return true; }

auto Link::rescaleWithMirror() const -> bool { return true; }

void Link::setAlignment(TextAlignment alignment) { this->alignment = alignment; }

TextAlignment Link::getAlignment() const { return this->alignment; }
