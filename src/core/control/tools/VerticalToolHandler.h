/*
 * Xournal++
 *
 * Vertical Space tool
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <memory>    // for unique_ptr
#include <vector>    // for vector

#include <cairo.h>  // for cairo_surface_t, cairo_t

#include "model/ElementContainer.h"  // for ElementContainer
#include "model/OverlayBase.h"
#include "model/PageRef.h"  // for PageRef
#include "util/Range.h"

#include "SnapToGridInputHandler.h"  // for SnapToGridInputHandler

class Element;
class Layer;
class MoveUndoAction;
class Settings;
class ZoomControl;
struct KeyEvent;

using ElementPtr = std::unique_ptr<Element>;

namespace xoj::view {
class OverlayView;
class Repaintable;
class VerticalToolView;
};  // namespace xoj::view

namespace xoj::util {
template <class T>
class DispatchPool;
};  // namespace xoj::util

/**
 * Handler class for the Vertical Spacing tool.
 */
class VerticalToolHandler: public ElementContainer, public OverlayBase {
public:
    /**
     * @param initiallyReverse Set this to true if the user has the reverse mode
     * button (e.g., Ctrl) held down when a vertical selection is started.
     */
    VerticalToolHandler(const PageRef& page, Settings* settings, double y, bool initiallyReverse);
    ~VerticalToolHandler() override;
    VerticalToolHandler(VerticalToolHandler&) = delete;
    VerticalToolHandler& operator=(VerticalToolHandler&) = delete;
    VerticalToolHandler(VerticalToolHandler&&) = delete;
    VerticalToolHandler&& operator=(VerticalToolHandler&&) = delete;

    /** Update the tool state with the new spacing position */
    void currentPos(double x, double y);

    bool onKeyPressEvent(const KeyEvent& event);
    bool onKeyReleaseEvent(const KeyEvent& event);

    std::unique_ptr<MoveUndoAction> finalize();

    void forEachElement(std::function<void(Element*)> f) const override;

    auto createView(xoj::view::Repaintable* parent, ZoomControl* zoomControl, const Settings* settings) const
            -> std::unique_ptr<xoj::view::OverlayView>;

    enum class Side {
        /** elements above the reference line */
        Above = -1,
        /** elements below the reference line */
        Below = 1,
    };

    inline double getStartY() const { return startY; }
    inline double getEndY() const { return endY; }
    inline Side getSide() const { return spacingSide; }
    double getPageWidth() const;

    inline auto getViewPool() const -> std::shared_ptr<xoj::util::DispatchPool<xoj::view::VerticalToolView>> {
        return viewPool;
    }

private:
    /**
     * Returns a vector of pointers to the elements we have adopted
     * This function copies the element ptrs into a new vector
     */
    auto refElements() const -> std::vector<Element*>;
    /**
     * Clear the currently moved elements, and then select all elements
     * above/below startY (depending on the side) to use for the spacing.
     * Lastly, redraw the elements to the buffer.
     */
    void adoptElements(Side side);

    /**
     * @brief Get the bounding range of the collection of elements we have adopted
     * @return The returned range may be empty if no elements have been adopted
     */
    Range computeElementsBoundingBox() const;


    PageRef page;
    Layer* layer;
    std::vector<ElementPtr> elements;
    /**
     * @brief Stores the smallest box containing all the adopted elements.
     *     Used to only refresh the part of the screen that needs refreshing.
     */
    Range ownedElementsOriginalBoundingBox;

    double startY;
    double endY;

    /**
     * Indicates whether to move elements above or below the anchor line.
     */
    Side spacingSide;

    /**
     * The handler for snapping points
     */
    SnapToGridInputHandler snappingHandler;

    std::shared_ptr<xoj::util::DispatchPool<xoj::view::VerticalToolView>> viewPool;
};
