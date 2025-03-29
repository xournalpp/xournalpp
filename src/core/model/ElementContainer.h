/*
 * Xournal++
 *
 * Interface for element containers like selection
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <functional>

class Element;

class ElementContainer {
public:
    virtual void forEachElement(std::function<void(const Element*)> f) const = 0;

protected:
    // interface -> protected, non virtual
    ~ElementContainer() = default;
};
