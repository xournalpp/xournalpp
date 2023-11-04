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
    void addElement(Element* e);

    /**
     * Inserts an Element in the specified position of the Layer%s internal list
     *
     * @note Performs a check to determine whether the element is already contained in the Layer
     */
    void insertElement(Element* e, Element::Index pos);

    /**
     * Returns the index of the given Element with respect to the internal list
     */
    Element::Index indexOf(Element* e) const;

    /**
     * Removes an Element from the Layer and optionally deletes it
     * @return the position the element occupied
     */
    Element::Index removeElement(Element* e, bool free);

    /**
     * Removes the Element. If e is not at index pos, tries to find it elsewhere (this could happen is the layer was
     * modified between now and when pos was computed)
     * @return The actual position of the removed element
     */
    Element::Index removeElementAt(Element* e, Element::Index pos, bool free);

    /**
     * Removes the Elements. If an element cannot be found at its designated position, it is search through the layer
     */
    void removeElementsAt(const InsertionOrder& elts, bool free);

    /**
     * Removes all Elements from the Layer *without freeing them*. Returns the elements.
     */
    std::vector<Element*> clearNoFree();

    /**
     * Returns an iterator over the Element%s contained in this Layer
     */
    const std::vector<Element*>& getElements() const;

    /**
     * Returns whether or not the Layer is empty
     */
    bool isAnnotated() const;

    /**
     * @return true if the layer is visible
     */
    bool isVisible() const;

    /**
     * @param visible true if the layer is visible
     */
    void setVisible(bool visible);

    /**
     * Creates a deep copy of this Layer by copying all of the Element%s contained in it
     */
    Layer* clone() const;

    /**
     * @return true if layer has a name
     */
    bool hasName() const;

    /**
     * @return layer custom name or empty string if custom name is not set
     */
    std::string getName() const;

    /**
     * Sets custom name for the layer
     */
    void setName(const std::string& newName);

private:
    std::vector<Element*> elements;

    bool visible = true;

    optional<std::string> name;
};
