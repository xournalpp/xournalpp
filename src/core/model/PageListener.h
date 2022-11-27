/*
 * Xournal++
 *
 * Page listener
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <memory>  // for shared_ptr, weak_ptr
#include <vector>

class Element;
class PageHandler;
class Range;
namespace xoj::util {
template <class T>
class Rectangle;
}  // namespace xoj::util

class PageListener {
public:
    PageListener();
    virtual ~PageListener();

public:
    void registerToHandler(std::shared_ptr<PageHandler> const& handler);
    void unregisterFromHandler();

    virtual void rectChanged(xoj::util::Rectangle<double>& rect) {}
    virtual void rangeChanged(Range& range) {}
    virtual void elementChanged(Element* elem) {}
    virtual void elementsChanged(const std::vector<Element*>& elements, const Range& range) {}
    virtual void pageChanged() {}

private:
    std::weak_ptr<PageHandler> handler;
};
