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

#include <string>
#include <vector>

#include "Element.h"
#include "XournalType.h"

template <class T>
using optional = std::optional<T>;

class Layer {
public:
    Layer();
    virtual ~Layer();

    using ElementIndex = std::ptrdiff_t;
    static constexpr auto InvalidElementIndex = static_cast<ElementIndex>(-1);

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
    void insertElement(Element* e, ElementIndex pos);

    /**
     * Returns the index of the given Element with respect to the internal list
     */
    ElementIndex indexOf(Element* e);

    /**
     * Removes an Element from the Layer and optionally deletes it
     */
    ElementIndex removeElement(Element* e, bool free);

    /**
     * Returns an iterator over the Element%s contained in this Layer
     */
    vector<Element*>* getElements();

    /**
     * Returns whether or not the Layer is empty
     */
    bool isAnnotated();

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
    Layer* clone();


    /**
     * @return true if layer has a name
     */
    bool hasName() const;

    /**
     * @return layer custom name or empty string if custom name is not set
     */
    string getName() const;

    /**
     * Sets custom name for the layer
     */
    void setName(const string& newName);

private:
    vector<Element*> elements;

    bool visible = true;

    optional<string> name;
};
