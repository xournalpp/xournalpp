#include "Text.h"

#include <utility>  // for move

#include <glib.h>  // for g_warning

#include "model/AudioElement.h"                   // for AudioElement
#include "model/Element.h"                        // for ELEMENT_TEXT, Eleme...
#include "model/Font.h"                           // for XojFont
#include "util/Rectangle.h"                       // for Rectangle
#include "util/Stacktrace.h"                      // for Stacktrace
#include "util/serializing/ObjectInputStream.h"   // for ObjectInputStream
#include "util/serializing/ObjectOutputStream.h"  // for ObjectOutputStream
#include "view/TextView.h"                        // for TextView

using xoj::util::Rectangle;

Text::Text(): AudioElement(ELEMENT_TEXT) {
    this->font.setName("Sans");
    this->font.setSize(12);
}

Text::~Text() = default;

auto Text::clone() const -> Element* {
    Text* text = new Text();
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

    return text;
}

auto Text::getFont() -> XojFont& { return font; }

void Text::setFont(const XojFont& font) { this->font = font; }

auto Text::getFontSize() const -> double { return font.getSize(); }

auto Text::getFontName() const -> std::string { return font.getName(); }

auto Text::getText() const -> std::string { return this->text; }

void Text::setText(std::string text) {
    this->text = std::move(text);

    calcSize();
}

void Text::calcSize() const {
    xoj::view::TextView::calcSize(this, this->width, this->height);
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

    calcSize();
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
