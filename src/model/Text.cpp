#include "Text.h"

#include <utility>

#include "serializing/ObjectInputStream.h"
#include "serializing/ObjectOutputStream.h"
#include "view/TextView.h"  // Hack: Needed to calculate the view size

#include "Stacktrace.h"

Text::Text(): AudioElement(ELEMENT_TEXT) {
    this->font.setName("Sans");
    this->font.setSize(12);
}

Text::~Text() = default;

auto Text::clone() -> Element* {
    Text* text = new Text();
    text->font = this->font;
    text->text = this->text;
    text->setColor(this->getColor());
    text->x = this->x;
    text->y = this->y;
    text->cloneAudioData(this);
    this->updateSnapping();

    return text;
}

auto Text::getFont() -> XojFont& { return font; }

void Text::setFont(const XojFont& font) { this->font = font; }

auto Text::getFontSize() const -> double { return font.getSize(); }

auto Text::getFontName() const -> string { return font.getName(); }

auto Text::getText() const -> string { return this->text; }

void Text::setText(string text) {
    this->text = std::move(text);

    calcSize();
}

void Text::calcSize() const {
    TextView::calcSize(this, this->width, this->height);
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

auto Text::intersects(double x, double y, double halfEraserSize) -> bool {
    return intersects(x, y, halfEraserSize, nullptr);
}

auto Text::intersects(double x, double y, double halfEraserSize, double* gap) -> bool {
    double x1 = this->x - halfEraserSize;
    double x2 = this->x + this->getElementWidth() + halfEraserSize;
    double y1 = this->y - halfEraserSize;
    double y2 = this->y + this->getElementHeight() + halfEraserSize;

    return x >= x1 && x <= x2 && y >= y1 && y <= y2;
}

void Text::serialize(ObjectOutputStream& out) {
    out.writeObject("Text");

    serializeAudioElement(out);

    out.writeString(this->text);

    font.serialize(out);

    out.endObject();
}

void Text::readSerialized(ObjectInputStream& in) {
    in.readObject("Text");

    readSerializedAudioElement(in);

    this->text = in.readString();

    font.readSerialized(in);

    in.endObject();
}

void Text::updateSnapping() const {
    this->snappedBounds = Rectangle<double>(this->x, this->y, this->width, this->height);
}
