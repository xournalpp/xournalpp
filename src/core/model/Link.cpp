#include "Link.h"

#include "model/AudioElement.h"  // for AudioElement
#include "model/Element.h"       // for ELEMENT_TEXT, Eleme...
#include "model/Font.h"          // for XojFont

Link::Link(): AudioElement(ELEMENT_LINK) {
    this->font.setName("Sans");
    this->font.setSize(12);
}

void Link::setText(std::string text) { this->text = text; }

std::string Link::getText() const { return this->text; }

void Link::setUrl(std::string url) { this->url = url; }

std::string Link::getUrl() const { return this->url; }

void Link::setInEditing(bool inEditing) { this->inEditing = inEditing; }

bool Link::isInEditing() const { return this->inEditing; }

void Link::setFont(const XojFont& font) { this->font = font; }

XojFont& Link::getFont() { return this->font; }
