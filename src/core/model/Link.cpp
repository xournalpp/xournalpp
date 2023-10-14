#include "Link.h"

#include "model/Element.h"                        // for ELEMENT_TEXT, Eleme...
#include "model/Font.h"                           // for XojFont
#include "util/Color.h"                           // for Colors
#include "util/Rectangle.h"                       // for Rectangle
#include "util/Stacktrace.h"                      // for Stacktrace
#include "util/serializing/ObjectInputStream.h"   // for ObjectInputStream
#include "util/serializing/ObjectOutputStream.h"  // for ObjectOutputStream

Link::Link(): Element(ELEMENT_LINK) {
    this->font.setName("Sans");
    this->font.setSize(12);
    this->setColor(Colors::magenta);
    this->text = "Hello World";
}

void Link::setText(std::string text) { this->text = text; }

std::string Link::getText() const { return this->text; }

void Link::setUrl(std::string url) { this->url = url; }

std::string Link::getUrl() const { return this->url; }

void Link::setHighlighted(bool highlighted) { this->highlighted = highlighted; }

bool Link::isHighlighted() const { return this->highlighted; }

void Link::setSelected(bool selected) { this->selected = selected; }

bool Link::isSelected() const { return this->selected; }

void Link::setFont(const XojFont& font) { this->font = font; }

XojFont& Link::getFont() { return this->font; }

void Link::serialize(ObjectOutputStream& out) const {
    out.writeObject("Link");

    Element::serialize(out);

    out.writeString(this->text);

    out.writeString(this->url);

    font.serialize(out);

    out.endObject();
}

void Link::readSerialized(ObjectInputStream& in) {
    in.readObject("Link");

    Element::readSerialized(in);

    this->text = in.readString();

    this->url = in.readString();

    font.readSerialized(in);

    in.endObject();
}

void Link::scale(double x0, double y0, double fx, double fy, double rotation, bool restoreLineWidth) {
    // only proportional scale allowed...
    if (fx != fy) {
        g_warning("rescale font with fx != fy not supported: %lf / %lf", fx, fy);
        Stacktrace::printStracktrace();
    }

    std::cout << x0 << ":" << y0 << ":" << fx << ":" << fy << std::endl;

    this->x -= x0;
    this->x *= fx;
    this->x += x0;
    this->y -= y0;
    this->y *= fy;
    this->y += y0;

    double size = this->font.getSize() * fx;
    this->font.setSize(size);

    sizeCalculated = false;
};

void Link::rotate(double x0, double y0, double th){};

Link* Link::clone() const {
    Link* link = new Link();
    link->font = this->font;
    link->text = this->text;
    link->url = this->url;
    link->setColor(this->getColor());
    link->x = this->x;
    link->y = this->y;
    link->width = this->width;
    link->height = this->height;
    link->snappedBounds = this->snappedBounds;
    link->sizeCalculated = this->sizeCalculated;
    link->highlighted = this->highlighted;
    return link;
};

void Link::calcSize() const {
    auto layout = this->createPangoLayout();
    pango_layout_set_text(layout.get(), this->text.c_str(), static_cast<int>(this->text.length()));
    int w = 0, h = 0;
    pango_layout_get_size(layout.get(), &w, &h);
    this->width = (static_cast<double>(w)) / PANGO_SCALE + PADDING;
    this->height = (static_cast<double>(h)) / PANGO_SCALE + PADDING;
    this->updateSnapping();
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

    pango_layout_set_alignment(layout.get(), this->alignment);

    return layout;
}

auto Link::rescaleOnlyAspectRatio() -> bool { return true; }

auto Link::rescaleWithMirror() -> bool { return true; }

void Link::setAlignment(PangoAlignment alignment) { this->alignment = alignment; }

PangoAlignment Link::getAlignment() { return this->alignment; }

void Link::updateSnapping() const {
    this->snappedBounds = xoj::util::Rectangle<double>(this->x, this->y, this->width, this->height);
}
