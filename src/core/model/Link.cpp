#include "Link.h"

#include "model/Element.h"                        // for ELEMENT_TEXT, Eleme...
#include "model/Font.h"                           // for XojFont
#include "util/Color.h"                           // for Colors
#include "util/serializing/ObjectInputStream.h"   // for ObjectInputStream
#include "util/serializing/ObjectOutputStream.h"  // for ObjectOutputStream

Link::Link(): Element(ELEMENT_LINK) {
    this->font.setName("Sans");
    this->font.setSize(12);

    // TODO: Remove, just for testing purpose
    this->setColor(Colors::magenta);
    this->setX(20);
    this->setY(20);
    this->setText("Hello World");
}

void Link::setText(std::string text) { this->text = text; }

std::string Link::getText() const { return this->text; }

void Link::setUrl(std::string url) { this->url = url; }

std::string Link::getUrl() const { return this->url; }

void Link::setInEditing(bool inEditing) { this->inEditing = inEditing; }

bool Link::isInEditing() const { return this->inEditing; }

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

void Link::scale(double x0, double y0, double fx, double fy, double rotation, bool restoreLineWidth){};

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
    link->inEditing = this->inEditing;
    return link;
};

void Link::calcSize() const {
    this->width = 200;
    this->height = 20;
};