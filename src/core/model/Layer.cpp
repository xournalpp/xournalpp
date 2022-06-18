#include "Layer.h"

#include <glib.h>  // for g_warning

#include "model/Element.h"    // for Element, Element::Index, Element::Inval...
#include "util/Stacktrace.h"  // for Stacktrace

Layer::Layer() = default;

Layer::~Layer() {
    for (Element* e: this->elements) { delete e; }
    this->elements.clear();
}

auto Layer::clone() const -> Layer* {
    auto* layer = new Layer();

    if (hasName()) {
        layer->setName(getName());
    }

    for (Element* e: this->elements) { layer->addElement(e->clone()); }

    return layer;
}

void Layer::addElement(Element* e) {
    if (e == nullptr) {
        g_warning("addElement(nullptr)!");
        Stacktrace::printStracktrace();
        return;
    }

    for (Element* elem2: this->elements) {
        if (e == elem2) {
            g_warning("Layer::addElement: Element is already on this layer!");
            return;
        }
    }

    this->elements.push_back(e);
}

void Layer::insertElement(Element* e, Element::Index pos) {
    if (e == nullptr) {
        g_warning("insertElement(nullptr)!");
        Stacktrace::printStracktrace();
        return;
    }

    for (Element* elem2: this->elements) {
        if (e == elem2) {
            g_warning("Layer::insertElement() try to add an element twice!");
            Stacktrace::printStracktrace();
            return;
        }
    }

    // prevent crash, even if this never should happen,
    // but there was a bug before which cause this error
    if (pos < 0) {
        pos = 0;
    }

    // If the element should be inserted at the top
    if (pos >= static_cast<int>(this->elements.size())) {
        this->elements.push_back(e);
    } else {
        this->elements.insert(this->elements.begin() + pos, e);
    }
}

auto Layer::indexOf(Element* e) const -> Element::Index {
    for (unsigned int i = 0; i < this->elements.size(); i++) {
        if (this->elements[i] == e) {
            return i;
        }
    }

    return Element::InvalidIndex;
}

auto Layer::removeElement(Element* e, bool free) -> Element::Index {
    for (unsigned int i = 0; i < this->elements.size(); i++) {
        if (e == this->elements[i]) {
            this->elements.erase(this->elements.begin() + i);

            if (free) {
                delete e;
            }
            return i;
        }
    }

    g_warning("Could not remove element from layer, it's not on the layer!");
    Stacktrace::printStracktrace();
    return Element::InvalidIndex;
}

void Layer::clearNoFree() { this->elements.clear(); }

auto Layer::isAnnotated() const -> bool { return !this->elements.empty(); }

/**
 * @return true if the layer is visible
 */
auto Layer::isVisible() const -> bool { return visible; }

/**
 * @return true if the layer is visible
 */
void Layer::setVisible(bool visible) { this->visible = visible; }

auto Layer::getElements() const -> const std::vector<Element*>& { return this->elements; }

auto Layer::hasName() const -> bool { return name.has_value(); }

auto Layer::getName() const -> std::string { return name.value_or(""); }

void Layer::setName(const std::string& newName) { this->name = newName; }
