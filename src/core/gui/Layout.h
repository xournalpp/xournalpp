/*
 * Xournal++
 *
 * Handles the layout of the pages within a Xournal document
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <cstddef>     // for size_t
#include <functional>  // for function
#include <mutex>       // for mutex
#include <optional>    // for optional
#include <vector>      // for vector

#include <gtk/gtk.h>  // for GtkAdjustment

#include "LayoutMapper.h"  // for LayoutMapper

class XojPageView;
class XournalView;
class ScrollHandling;
class Range;

namespace xoj::util {
template <typename T>
struct Point;
template <typename T>
class Rectangle;
};  // namespace xoj::util

/**
 * @brief The Layout manager for the XournalWidget
 *
 * This class manages the layout of the XojPageView's contained
 * in the XournalWidget
 */
class Layout final {
public:
    Layout(XournalView* view, ScrollHandling* scrollHandling);

public:
    // Todo(Fabian): move to ScrollHandling also it must not depend on Layout
    /**
     * Increases the adjustments by the given amounts
     */
    void scrollRelative(double x, double y);

    // Todo(Fabian): move to ScrollHandling also it must not depend on Layout
    /**
     * Changes the adjustments by absolute amounts (for pinch-to-zoom)
     */
    void scrollAbs(double x, double y);

    // Todo(Fabian): move to XournalView
    /**
     * Changes the adjustments in such a way as to make sure that
     * the given Rectangle is visible
     *
     * @remark If the given Rectangle won't fit into the scrolled window
     *         then only its top left corner will be visible
     */
    void ensureRectIsVisible(int x, int y, int width, int height);

    /// Returns the height of the entire Layout - including centering padding
    int getTotalPixelHeight() const;

    /// Returns the height of the entire Layout - excluding centering padding
    int getMinimalPixelHeight() const;

    /// Returns the width of the entire Layout - including centering padding
    int getTotalPixelWidth() const;

    /// Returns the width of the entire Layout - excluding centering padding
    int getMinimalPixelWidth() const;

    // Todo(Fabian): move to XournalView this must not depend on Layout directly
    /**
     * Returns the Rectangle which is currently visible
     */
    xoj::util::Rectangle<double> getVisibleRect();


    /**
     * recalculate and resize Layout
     * If forceNow == true, the layout is computed straight away and the gtk_adjustment values are updated
     * Otherwise, the computation is differed until the values are needed
     */
    void recalculate();

    /**
     * Recompute the centering paddings (to center the content if the allocation is too big)
     * @params the size of the GtkAllocation of the GtkXournal instance - or -1 for computation from the GtkAdjustments
     */
    void recomputeCenteringPadding(int allocWidth = -1, int allocHeight = -1);

    /**
     * Return the pageview containing co-ordinates.
     */
    XojPageView* getPageViewAt(int x, int y);

    /**
     * Return the page index found (or std::nullopt if not found) when moving by the given offsets from the ref page
     * Assumes refPageNumber is a valid page index.
     */
    std::optional<size_t> getPageWithRelativePosition(size_t refPageNumber, int columnOffset, int rowOffset) const;

    /**
     * @brief Get the total padding above the page at the given index.
     *
     *  The size of this padding does not scale with pages as the user zooms in and out.
     * As such, it is often necessary to get _just_ this padding.
     *
     * @param pageIndex is the index of the XojPageView as returned by [getIndexAtGridMap]
     * @return the sum of the padding between pages above the page with the given index
     *         and any padding the user added vertically above all pages (i.e. in settings).
     */
    int getPaddingAbovePage(size_t pageIndex) const;

    /**
     * @brief Get the static padding added to the left of the current page.
     *
     *  This does not include padding added to center the page on the screen.
     *
     * @param pageIndex is the index of the XojPageView, as given by [getIndexAtGridMap]
     * @return the sum of the padding between pages left of the page at [pageIndex] and
     *         any horizontal padding the user decided to add (from settings)
     */
    int getPaddingLeftOfPage(size_t pageIndex) const;

    xoj::util::Point<int> getPixelCoordinatesOfEntry(xoj::util::Point<int> gridCoords) const;
    xoj::util::Point<int> getPixelCoordinatesOfEntry(size_t n) const;

    /**
     * Execute the given function for each entry that intersects the range. entryIndex is the entry, intersection is the
     * intersection (in widget coordinates) and pixelPosition is the entry's upper left corner in widget coordinates
     */
    void forEachEntriesIntersectingRange(
            const Range& rg,
            std::function<void(size_t entryIndex, const Range& intersection, xoj::util::Point<int> pixelPosition)> fun)
            const;

protected:
    /// Same as above but does not lock the mutex
    xoj::util::Point<int> getPixelCoordinatesOfEntryUnsafe(xoj::util::Point<int> gridCoords) const;
    /// Same as above but does not lock the mutex
    xoj::util::Point<int> getPixelCoordinatesOfEntryUnsafe(size_t n) const;

    /// Same as above but does not lock the mutex
    int getTotalPixelWidthUnsafe() const;
    /// Same as above but does not lock the mutex
    int getMinimalPixelWidthUnsafe() const;
    /// Same as above but does not lock the mutex
    int getTotalPixelHeightUnsafe() const;
    /// Same as above but does not lock the mutex
    int getMinimalPixelHeightUnsafe() const;
    /// Same as above but does not lock the mutex
    void recomputeCenteringPaddingUnsafe(int allocWidth, int allocHeight);

    // Todo(Fabian): move to ScrollHandling also it must not depend on Layout
    static void horizontalScrollChanged(GtkAdjustment* adjustment, Layout* layout);
    static void verticalScrollChanged(GtkAdjustment* adjustment, Layout* layout);

private:
    void computePrecalculated();

    void maybeAddLastPage(Layout* layout);

    // Todo(Fabian): move to ScrollHandling also it must not depend on Layout
    static void checkScroll(GtkAdjustment* adjustment, double& lastScroll);

public:
    struct PreCalculated {
        mutable std::mutex m;

        LayoutMapper mapper;

        std::vector<double> widthCols;   ///< In page coordinates - multiply by zoom to get pixels
        std::vector<double> heightRows;  ///< In page coordinates - multiply by zoom to get pixels

        std::vector<double> stretchableHorizontalPixelsAfterColumn;  ///< Stretchable - multiply by zoom to get pixels
        std::vector<double> stretchableVerticalPixelsAfterRow;       ///< Stretchable - multiply by zoom to get pixels
        int paddingLeft;                                             ///< in pixels
        int paddingRight;                                            ///< in pixels
        int paddingTop;                                              ///< in pixels
        int paddingBottom;                                           ///< in pixels

        int horizontalCenteringPadding;  ///< Added before and after if the allocation is too big
        int verticalCenteringPadding;    ///< Added before and after if the allocation is too big
    };

private:
    struct PixelCounter;  ///< Used to get the pixel coordinates of entries

    XournalView* view = nullptr;
    ScrollHandling* scrollHandling = nullptr;

    PreCalculated pc{};

    /// Used to have only one call when zooming in/out
    bool blockScrollCallback = false;
};
