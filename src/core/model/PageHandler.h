/*
 * Xournal++
 *
 * Page handler
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <list>  // for list
#include <vector>

#include "util/Range.h"  // for Range

class Element;
class PageListener;
class Range;
namespace xoj::util {
template <class T>
class Rectangle;
}  // namespace xoj::util

class PageHandler {
public:
    PageHandler();
    virtual ~PageHandler();

public:
    void fireRectChanged(xoj::util::Rectangle<double>& rect);
    void fireRangeChanged(Range& range);
    void fireElementChanged(Element* elem);
    /**
     * @brief The listed elements have been changed
     * @param range (optional) if provided, the Range must contain the bounding boxes of the changed elements, both
     * before and after they were changed
     */
    void fireElementsChanged(const std::vector<Element*>& elements, Range range = Range());
    void firePageChanged();

private:
    void addListener(PageListener* l);
    void removeListener(PageListener* l);

private:
    std::list<PageListener*> listeners;

    friend class PageListener;
};
