#include "Layer.h"

#include <cstddef>
#include <memory>
#include <utility>
#include <vector>

#include <glib.h>  // for g_warning

#include "model/Element.h"  // for Element, Element::Index, Element::Inval...
#include "model/ElementInsertionPosition.h"
#include "util/Assert.h"      // for xoj_assert
#include "util/Stacktrace.h"  // for Stacktrace
#include "util/safe_casts.h"

Layer::Layer() = default;

Layer::~Layer() = default;

auto Layer::clone() const -> Layer* {
    auto* layer = new Layer();

    if (hasName()) {
        layer->setName(getName());
    }

    for (auto const& e: this->elements) {
        layer->addElement(e->clone());
    }

    return layer;
}

void Layer::addElement(ElementPtr e) {
    if (e == nullptr) {
        g_warning("addElement(nullptr)!");
        Stacktrace::printStacktrace();
        return;
    }

    this->elements.emplace_back(std::move(e));
}

void Layer::insertElement(ElementPtr e, Element::Index pos) {
    if (e == nullptr) {
        g_warning("insertElement(nullptr)!");
        Stacktrace::printStacktrace();
        return;
    }

    // prevent crash, even if this never should happen,
    // but there was a bug before which cause this error
    if (pos < 0) {
        pos = 0;
    }

    // If the element should be inserted at the top
    if (pos >= static_cast<int>(this->elements.size())) {
        this->elements.push_back(std::move(e));
    } else {
        this->elements.insert(this->elements.begin() + pos, std::move(e));
    }
}

auto Layer::indexOf(const Element* e) const -> Element::Index {
    for (unsigned int i = 0; i < this->elements.size(); i++) {
        if (this->elements[i].get() == e) {
            return i;
        }
    }

    return Element::InvalidIndex;
}

auto Layer::removeElement(const Element* e) -> InsertionPosition {
    for (unsigned int i = 0; i < this->elements.size(); i++) {
        if (e == this->elements[i].get()) {
            auto res = std::move(this->elements[i]);
            this->elements.erase(this->elements.begin() + i);
            return InsertionPosition{std::move(res), i};
        }
    }

    g_warning("Could not remove element %p from layer %p, it's not on the layer!", e, this);
    Stacktrace::printStacktrace();
    return InsertionPosition{nullptr, Element::InvalidIndex};
}

auto Layer::removeElementAt(const Element* e, Element::Index pos) -> InsertionPosition {
    if (pos >= 0 && as_unsigned(pos) < elements.size() && this->elements[as_unsigned(pos)].get() == e) {
        auto iter = std::next(this->elements.begin(), pos);
        auto res = std::move(*iter);
        this->elements.erase(iter);
        return InsertionPosition{std::move(res), pos};
    }
    return removeElement(e);
}

auto Layer::removeElementsAt(InsertionOrderRef const& elts) -> InsertionOrder {
    InsertionOrder res;
    res.reserve(elts.size());
    auto endIndex = static_cast<Element::Index>(elements.size());
    for (auto&& [e, p]: elts) {
        xoj_assert(e);
        auto pos = p;
        if (pos < 0 || pos > endIndex || elements[static_cast<size_t>(pos)].get() != e) {
            pos = indexOf(e);
            if (pos == Element::InvalidIndex) {
                g_warning("Could not remove element from layer, it's not on the layer!");
                Stacktrace::printStacktrace();
                continue;
            }
        }
        res.emplace_back(std::move(elements[static_cast<size_t>(pos)]), pos);
    }
    this->elements.erase(std::remove(this->elements.begin(), this->elements.end(), nullptr), this->elements.end());
    return res;
}

auto Layer::clearNoFree() -> std::vector<ElementPtr> { return std::move(this->elements); }

auto Layer::isAnnotated() const -> bool { return !this->elements.empty(); }

/**
 * @return true if the layer is visible
 */
auto Layer::isVisible() const -> bool { return visible; }

/**
 * @return true if the layer is visible
 */
void Layer::setVisible(bool visible) { this->visible = visible; }

auto Layer::getElements() -> std::vector<ElementPtr>& { return this->elements; }

auto Layer::getElementsView() const -> xoj::util::PointerContainerView<std::vector<ElementPtr>> {
    return this->elements;
}


auto Layer::hasName() const -> bool { return name.has_value(); }

auto Layer::getName() const -> std::string { return name.value_or(""); }

void Layer::setName(const std::string& newName) { this->name = newName; }
