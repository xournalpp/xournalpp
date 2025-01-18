/*
 * Xournal++
 *
 * A layer on a page
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <cstddef>   // for size_t
#include <memory>    // for unique_ptr
#include <optional>  // for optional
#include <string>    // for string
#include <vector>    // for vector

#include "Element.h"  // for Element, Element::Index
#include "ElementInsertionPosition.h"  // for InsertionOrder

template <class T>
using optional = std::optional<T>;

class Layer {
public:
    Layer();
    virtual ~Layer();

    using Index = size_t;

public:
    /**
     * Appends an Element to this Layer
     *
     * @note Performs a check to determine whether the element is already contained in the Layer
     */
    void addElement(ElementPtr e);

    /**
     * Inserts an Element in the specified position of the Layer%s internal list
     *
     * @note Performs a check to determine whether the element is already contained in the Layer
     */
    void insertElement(ElementPtr e, Element::Index pos);

    /**
     * Returns the index of the given Element with respect to the internal list
     */
    auto indexOf(Element* e) const -> Element::Index;

    /**
     * Removes an Element from the Layer and optionally deletes it
     * @return the position the element occupied
     */
    auto removeElement(Element* e) -> InsertionPosition;

    /**
     * Removes the Element. If e is not at index pos, tries to find it elsewhere (this could happen is the layer was
     * modified between now and when pos was computed)
     * @return The actual position of the removed element
     */
    auto removeElementAt(Element* e, Element::Index pos) -> InsertionPosition;

    /**
     * Removes the Elements. If an element cannot be found at its designated position, it is search through the layer
     */
    auto removeElementsAt(InsertionOrderRef const& elts) -> InsertionOrder;

    /**
     * Removes all Elements from the Layer *without freeing them*. Returns the elements.
     */
    auto clearNoFree() -> std::vector<ElementPtr>;

    /**
     * Returns an iterator over the Element%s contained in this Layer
     */
    auto getElements() const -> std::vector<ElementPtr> const&;

    /**
     * Returns whether or not the Layer is empty
     */
    auto isAnnotated() const -> bool;

    /**
     * @return true if the layer is visible
     */
    auto isVisible() const -> bool;

    /**
     * @param visible true if the layer is visible
     */
    void setVisible(bool visible);

    /**
     * Creates a deep copy of this Layer by copying all of the Element%s contained in it
     */
    auto clone() const -> Layer*;

    /**
     * @return true if layer has a name
     */
    auto hasName() const -> bool;

    /**
     * @return layer custom name or empty string if custom name is not set
     */
    auto getName() const -> std::string;

    /**
     * Sets custom name for the layer
     */
    void setName(const std::string& newName);

private:
    std::vector<ElementPtr> elements;

    bool visible = true;

    std::optional<std::string> name;
};
