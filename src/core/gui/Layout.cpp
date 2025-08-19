#include "Layout.h"

#include <algorithm>    // for max, lower_bound, transform
#include <cmath>        // for abs
#include <iterator>     // for begin, end, distance
#include <numeric>      // for accumulate
#include <optional>     // for optional
#include <type_traits>  // for make_signed_t, remove_referen...

#include <glib-object.h>  // for G_CALLBACK, g_signal_connect

#include "control/Control.h"            // for Control
#include "control/settings/Settings.h"  // for Settings
#include "gui/LayoutMapper.h"           // for LayoutMapper, GridPosition
#include "gui/PageView.h"               // for XojPageView
#include "gui/scroll/ScrollHandling.h"  // for ScrollHandling
#include "model/Document.h"             // for Document
#include "util/Range.h"                 // for Range
#include "util/Rectangle.h"             // for Rectangle
#include "util/safe_casts.h"            // for strict_cast, as_signed, as_si...

#include "XournalView.h"  // for XournalView

/**
 * Padding outside the pages, including shadow
 */
constexpr auto const XOURNAL_PADDING = 10;

/**
 * Allowance for shadow between page pairs in paired page mode
 */
constexpr auto const XOURNAL_ROOM_FOR_SHADOW = 3;

/**
 * Padding between the pages
 */
constexpr auto const XOURNAL_PADDING_BETWEEN = 15;


Layout::Layout(XournalView* view, ScrollHandling* scrollHandling): view(view), scrollHandling(scrollHandling) {
    g_signal_connect(scrollHandling->getHorizontal(), "value-changed", G_CALLBACK(horizontalScrollChanged), this);
    g_signal_connect(scrollHandling->getVertical(), "value-changed", G_CALLBACK(verticalScrollChanged), this);


    lastScrollHorizontal = gtk_adjustment_get_value(scrollHandling->getHorizontal());
    lastScrollVertical = gtk_adjustment_get_value(scrollHandling->getVertical());
}

void Layout::horizontalScrollChanged(GtkAdjustment* adjustment, Layout* layout) {
    Layout::checkScroll(adjustment, layout->lastScrollHorizontal);
    layout->updateVisibility();
}

void Layout::verticalScrollChanged(GtkAdjustment* adjustment, Layout* layout) {
    Layout::checkScroll(adjustment, layout->lastScrollVertical);
    layout->updateVisibility();

    layout->maybeAddLastPage(layout);
}

void Layout::maybeAddLastPage(Layout* layout) {
    auto* control = this->view->getControl();
    auto* settings = control->getSettings();
    if (settings->getEmptyLastPageAppend() == EmptyLastPageAppendType::OnScrollToEndOfLastPage) {
        // If the layout is 5px away from the end of the last page
        if (std::abs((layout->getTotalPixelHeight() - layout->getVisibleRect().y) - layout->getVisibleRect().height) <
            5) {
            auto* doc = control->getDocument();
            doc->lock();
            auto pdfPageCount = doc->getPdfPageCount();
            doc->unlock();
            if (pdfPageCount == 0) {
                auto currentPage = control->getCurrentPageNo();
                doc->lock();
                auto lastPage = doc->getPageCount() - 1;
                doc->unlock();
                if (currentPage == lastPage) {
                    control->insertNewPage(currentPage + 1, false);
                }
            }
        }
    }
}

void Layout::checkScroll(GtkAdjustment* adjustment, double& lastScroll) {
    lastScroll = gtk_adjustment_get_value(adjustment);
}

/**
 * Iteratable view that computes the pixel coordinates of the delimitation between columns (makeHorizontal) or rows
 * (makeVertical) in the layout from the PreCalculated zoom-agnostic data
 *
 * The value returned by a given iterator is the pixel coordinate AFTER the corresponding column/row.
 */
struct Layout::PixelCounter {
    static PixelCounter makeVertical(const PreCalculated& pc, double zoom) {
        return {pc.stretchableVerticalPixelsAfterRow, pc.paddingTop, zoom};
    }
    static PixelCounter makeHorizontal(const PreCalculated& pc, double zoom) {
        return {pc.stretchableHorizontalPixelsAfterColumn, pc.paddingLeft, zoom};
    }

    static_assert(std::is_same<decltype(PreCalculated::stretchableHorizontalPixelsAfterColumn),
                               decltype(PreCalculated::stretchableVerticalPixelsAfterRow)>());
    using base_container = decltype(PreCalculated::stretchableHorizontalPixelsAfterColumn);
    using base_iterator = base_container::const_iterator;

private:
    PixelCounter(const base_container& c, int padding, double zoom):
            b(c.begin()), e(c.end()), padding(padding), zoom(zoom) {}

    base_iterator b;
    base_iterator e;
    int padding;
    double zoom;

public:
    struct iterator {
        iterator() = default;
        iterator(const iterator& o) = default;
        iterator(iterator&& o) = default;
        iterator& operator=(const iterator& o) = default;
        iterator& operator=(iterator&& o) = default;

        using difference_type = base_iterator::difference_type;
        using iterator_category = std::random_access_iterator_tag;
        using value_type = double;
        using reference = double;
        using pointer = void;

        iterator(base_iterator base, const PixelCounter* parent): it(base), parent(parent) {}


        /**
         * This is the only function with non-trivial content of this iterator class. Computes the pixel position based
         * on the precalculated zoom-agnostic data
         */
        value_type operator*() const {
            return *it * parent->zoom + parent->padding +
                   XOURNAL_PADDING_BETWEEN * static_cast<double>(std::distance(parent->b, it)) +
                   .5 * XOURNAL_PADDING_BETWEEN;
        }

        // input iterator
        bool operator==(const iterator& o) const { return it == o.it; }
        bool operator!=(const iterator& o) const { return it != o.it; }
        iterator& operator++() {
            ++it;
            return *this;
        }

        // forward iterator
        iterator operator++(int) {
            auto c = *this;
            this->operator++();
            return c;
        }

        // bidirectional operator
        iterator& operator--() {
            --it;
            return *this;
        }
        iterator operator--(int) {
            auto c = *this;
            this->operator--();
            return c;
        }

        // random access iterator
        iterator& operator+=(difference_type n) {
            it += n;
            return *this;
        }
        iterator operator+(difference_type n) const { return iterator(it + n, parent); }
        friend iterator operator+(iterator::difference_type n, const iterator& self) { return self + n; }
        iterator& operator-=(difference_type n) {
            it -= n;
            return *this;
        }
        iterator operator-(difference_type n) const { return iterator(it - n, parent); }
        difference_type operator-(const iterator& o) const { return it - o.it; }
        value_type operator[](difference_type n) const { return *(*this + n); }
        bool operator<(const iterator& o) const { return it < o.it; }
        bool operator<=(const iterator& o) const { return it <= o.it; }
        bool operator>(const iterator& o) const { return it > o.it; }
        bool operator>=(const iterator& o) const { return it >= o.it; }

    private:
        base_iterator it;
        const PixelCounter* parent;
    };
    iterator begin() const { return iterator(b, this); }
    iterator end() const { return iterator(e, this); }
};

void Layout::forEachEntriesIntersectingRange(
        const Range& rg, std::function<void(size_t, const Range&, xoj::util::Point<int>)> fun) const {
    if (rg.empty()) {
        return;
    }
    xoj_assert(rg.isValid());
    const double zoom = this->view->getZoom();
    std::lock_guard g{pc.m};
    if (!pc.valid) {
        computePrecalculated();
    }
    auto verticalPixels = PixelCounter::makeVertical(this->pc, zoom);
    auto horizontalPixels = PixelCounter::makeHorizontal(this->pc, zoom);

    // Use binary search to find the possibly visible slots
    auto xEnd = [&]() {
        // To get an actual "end", take the next one
        auto it = std::upper_bound(horizontalPixels.begin(), horizontalPixels.end(), rg.maxX);
        return it == horizontalPixels.end() ? horizontalPixels.end() : std::next(it);
    }();
    auto xBegin = std::lower_bound(horizontalPixels.begin(), xEnd, rg.minX);

    auto yEnd = [&]() {
        // To get an actual "end", take the next one
        auto it = std::upper_bound(verticalPixels.begin(), verticalPixels.end(), rg.maxY);
        return it == verticalPixels.end() ? verticalPixels.end() : std::next(it);
    }();
    auto yBegin = std::lower_bound(verticalPixels.begin(), yEnd, rg.minY);

    for (auto col = as_unsigned(std::distance(horizontalPixels.begin(), xBegin)),
              endCol = as_unsigned(std::distance(horizontalPixels.begin(), xEnd));
         col != endCol; col++) {
        for (auto row = as_unsigned(std::distance(verticalPixels.begin(), yBegin)),
                  endRow = as_unsigned(std::distance(verticalPixels.begin(), yEnd));
             row != endRow; row++) {
            auto optionalPage = this->mapper.at({as_unsigned(col), as_unsigned(row)});
            if (optionalPage) {
                auto pos = this->getPixelCoordinatesOfEntryUnsafe(optionalPage.value());
                auto& pageView = this->view->viewPages[optionalPage.value()];
                double w = pageView->getWidth();
                double h = pageView->getHeight();
                Range pageRg(pos.x, pos.y, pos.x + w * zoom, pos.y + h * zoom);
                if (auto intersection = pageRg.intersect(rg); !intersection.empty()) {
                    fun(optionalPage.value(), intersection, pos);
                }
            }
        }
    }
}

void Layout::updateVisibility() {
    auto visibleRg = Range(getVisibleRect());
    xoj::util::Point<int> center(round_cast<int>(.5 * (visibleRg.minX + visibleRg.maxX)),
                                 round_cast<int>(.5 * (visibleRg.minY + visibleRg.maxY)));

    // Data to select page based on visibility
    std::optional<size_t> mostPageNr;
    double mostPagePercent = 0;

    std::vector<size_t> visiblePages;
    forEachEntriesIntersectingRange(visibleRg, [&](size_t index, const Range& intersection, xoj::util::Point<int> pos) {
        auto& pageView = this->view->viewPages[index];
        pageView->setIsVisible(true);
        pageView->setCenterOfVisibleArea(center, pos);
        visiblePages.emplace_back(index);

        // Set the selected page
        double percent =
                intersection.getWidth() * intersection.getHeight() / (visibleRg.getWidth() * visibleRg.getHeight());

        if (percent > mostPagePercent) {
            mostPageNr = index;
            mostPagePercent = percent;
        }
    });

    std::sort(visiblePages.begin(), visiblePages.end());
    xoj_assert(std::is_sorted(this->previouslyVisiblePages.begin(), this->previouslyVisiblePages.end()));
    printf("visible pages:");
    for (auto&& p: visiblePages) {
        printf(" %zu", p);
    }
    printf("\n");
    auto it = visiblePages.begin();
    for (auto&& s: this->previouslyVisiblePages) {
        while (it != visiblePages.end() && *it < s) {
            it++;
        }
        if (it == visiblePages.end() || *it != s) {
            // This page is no longer visible
            this->view->viewPages[s]->setIsVisible(false);
        }
    }
    this->previouslyVisiblePages = std::move(visiblePages);
    if (mostPageNr) {
        this->view->getControl()->firePageSelected(*mostPageNr);
    }
}

auto Layout::getVisiblePages() const -> std::vector<size_t> {
    // We make a copy in case previouslyVisiblePages's iterators get invalidated. The vector is typically very small.
    return previouslyVisiblePages;
}

auto Layout::getVisibleRect() -> xoj::util::Rectangle<double> {
    return xoj::util::Rectangle<double>(gtk_adjustment_get_value(scrollHandling->getHorizontal()),
                                        gtk_adjustment_get_value(scrollHandling->getVertical()),
                                        gtk_adjustment_get_page_size(scrollHandling->getHorizontal()),
                                        gtk_adjustment_get_page_size(scrollHandling->getVertical()));
}

void Layout::computePrecalculated() const {
    auto* settings = view->getControl()->getSettings();
    auto len = view->viewPages.size();
    mapper.configureFromSettings(len, settings);
    auto colCount = mapper.getColumns();
    auto rowCount = mapper.getRows();

    pc.widthCols.assign(colCount, 0);
    pc.heightRows.assign(rowCount, 0);

    for (size_t pageIdx{}; pageIdx < len; ++pageIdx) {
        auto const& raster_p = mapper.at(pageIdx);  // auto [c, r] raster = mapper.at();
        auto const& c = raster_p.col;
        auto const& r = raster_p.row;
        auto& v = view->viewPages[pageIdx];
        pc.widthCols[c] = std::max(pc.widthCols[c], v->getWidth());
        pc.heightRows[r] = std::max(pc.heightRows[r], v->getHeight());
        v->setGridCoordinates({strict_cast<int>(c), strict_cast<int>(r)});
    }

    pc.stretchableHorizontalPixelsAfterColumn.clear();
    pc.stretchableHorizontalPixelsAfterColumn.reserve(colCount);
    pc.stretchableVerticalPixelsAfterRow.clear();
    pc.stretchableVerticalPixelsAfterRow.reserve(rowCount);

    std::partial_sum(pc.widthCols.begin(), pc.widthCols.end(),
                     std::back_inserter(pc.stretchableHorizontalPixelsAfterColumn));
    std::partial_sum(pc.heightRows.begin(), pc.heightRows.end(),
                     std::back_inserter(pc.stretchableVerticalPixelsAfterRow));

    pc.paddingTop = XOURNAL_PADDING;
    pc.paddingBottom = XOURNAL_PADDING;
    if (settings->getUnlimitedScrolling()) {
        pc.paddingTop += static_cast<int>(gtk_adjustment_get_page_size(scrollHandling->getVertical()));
        pc.paddingBottom = pc.paddingTop;
    } else if (settings->getAddVerticalSpace()) {
        pc.paddingTop += settings->getAddVerticalSpaceAmountAbove();
        pc.paddingBottom += settings->getAddVerticalSpaceAmountBelow();
    }

    pc.paddingLeft = XOURNAL_PADDING;
    pc.paddingRight = XOURNAL_PADDING;
    if (settings->getUnlimitedScrolling()) {
        pc.paddingLeft += static_cast<int>(gtk_adjustment_get_page_size(scrollHandling->getHorizontal()));
        pc.paddingRight = pc.paddingLeft;
    } else if (settings->getAddHorizontalSpace()) {
        pc.paddingLeft += settings->getAddHorizontalSpaceAmountLeft();
        pc.paddingRight += settings->getAddHorizontalSpaceAmountRight();
    }

    printf("pc: %d %d %d %d  layout %zu x %zu\n", pc.paddingTop, pc.paddingBottom, pc.paddingLeft, pc.paddingRight,
           colCount, rowCount);

    pc.valid = true;
}

void Layout::recalculate(bool forceNow) {
    pc.valid = false;
    if (forceNow) {
        gtk_adjustment_set_upper(scrollHandling->getHorizontal(), getTotalPixelWidth());
        gtk_adjustment_set_upper(scrollHandling->getVertical(), getTotalPixelHeight());
    }
    gtk_widget_queue_resize(view->getWidget());
}

auto Layout::getPaddingAbovePage(size_t pageIndex) const -> int {
    const Settings* settings = this->view->getControl()->getSettings();

    // User-configured padding above all pages.
    auto paddingAbove = XOURNAL_PADDING;
    if (settings->getUnlimitedScrolling()) {
        paddingAbove += ceil_cast<int>(gtk_adjustment_get_page_size(scrollHandling->getVertical()));
    } else if (settings->getAddVerticalSpace()) {
        paddingAbove += settings->getAddVerticalSpaceAmountAbove();
    }

    // (x, y) coordinate pair gives grid indices. This handles paired pages and different page layouts for us.
    auto pageYLocation = strict_cast<int>(this->mapper.at(pageIndex).row);
    return pageYLocation * XOURNAL_PADDING_BETWEEN + paddingAbove;
}


auto Layout::getPaddingLeftOfPage(size_t pageIndex) const -> int {
    bool isPairedPages = this->mapper.isPairedPages();
    const Settings* settings = this->view->getControl()->getSettings();

    auto paddingBefore = XOURNAL_PADDING;
    if (settings->getUnlimitedScrolling()) {
        paddingBefore += ceil_cast<int>(gtk_adjustment_get_page_size(scrollHandling->getHorizontal()));
    } else if (settings->getAddHorizontalSpace()) {
        paddingBefore += settings->getAddHorizontalSpaceAmountLeft();
    }

    auto const pageXLocation = strict_cast<int>(this->mapper.at(pageIndex).col);

    // No page pairing or we haven't rendered enough pages in the row for page pairing to have an effect,
    if (!isPairedPages) {
        return pageXLocation * XOURNAL_PADDING_BETWEEN + XOURNAL_PADDING_BETWEEN / 2 + paddingBefore;
    } else {
        auto columnPadding = XOURNAL_PADDING_BETWEEN + pageXLocation * XOURNAL_PADDING_BETWEEN;
        if (pageXLocation % 2 == 0) {
            return columnPadding - XOURNAL_ROOM_FOR_SHADOW + paddingBefore;
        } else {
            return columnPadding + XOURNAL_ROOM_FOR_SHADOW + paddingBefore;
        }
    }
}

void Layout::scrollRelative(double x, double y) {
    if (this->view->getControl()->getSettings()->isPresentationMode()) {
        return;
    }

    gtk_adjustment_set_value(scrollHandling->getHorizontal(),
                             gtk_adjustment_get_value(scrollHandling->getHorizontal()) + x);
    gtk_adjustment_set_value(scrollHandling->getVertical(),
                             gtk_adjustment_get_value(scrollHandling->getVertical()) + y);
}

void Layout::scrollAbs(double x, double y) {
    if (this->view->getControl()->getSettings()->isPresentationMode()) {
        return;
    }

    gtk_adjustment_set_value(scrollHandling->getHorizontal(), x);
    gtk_adjustment_set_value(scrollHandling->getVertical(), y);
}

void Layout::ensureRectIsVisible(int x, int y, int width, int height) {
    gtk_adjustment_clamp_page(scrollHandling->getHorizontal(), x - 5, x + width + 10);
    gtk_adjustment_clamp_page(scrollHandling->getVertical(), y - 5, y + height + 10);
}

auto Layout::getPageViewAt(int x, int y) -> XojPageView* {
    auto gridPosition = [&]() {
        std::lock_guard g{pc.m};
        if (!pc.valid) {
            computePrecalculated();
        }
        // Binary Search:
        double zoom = this->view->getZoom();
        auto verticalPixels = PixelCounter::makeVertical(this->pc, zoom);
        auto rit = std::lower_bound(verticalPixels.begin(), verticalPixels.end(), y);
        auto const foundRow = size_t(std::distance(verticalPixels.begin(), rit));

        auto horizontalPixels = PixelCounter::makeHorizontal(this->pc, zoom);
        auto cit = std::lower_bound(horizontalPixels.begin(), horizontalPixels.end(), x);
        auto const foundCol = size_t(std::distance(horizontalPixels.begin(), cit));

        return GridPosition{foundCol, foundRow};
    }();
    auto optionalPage = this->mapper.at(gridPosition);

    if (optionalPage && this->view->viewPages[*optionalPage]->containsPoint(x, y, false)) {
        return this->view->viewPages[*optionalPage].get();
    }

    return nullptr;
}

auto Layout::getPageWithRelativePosition(size_t referencePageNumber, int columnOffset, int rowOffset) const
        -> std::optional<size_t> {
    auto pos = this->mapper.at(referencePageNumber);
    return this->mapper.at(
            {as_unsigned(as_signed(pos.col) + columnOffset), as_unsigned(as_signed(pos.row) + rowOffset)});
}

auto Layout::getTotalPixelHeight() const -> int {
    std::lock_guard g{pc.m};
    if (!pc.valid) {
        computePrecalculated();
    }
    auto rowCount = mapper.getRows();
    return pc.paddingTop + pc.paddingBottom + strict_cast<int>(rowCount - 1) * XOURNAL_PADDING_BETWEEN +
           ceil_cast<int>(pc.stretchableVerticalPixelsAfterRow.back() * view->getZoom());
}

auto Layout::getTotalPixelWidth() const -> int {
    std::lock_guard g{pc.m};
    if (!pc.valid) {
        computePrecalculated();
    }
    auto colCount = mapper.getColumns();
    return pc.paddingLeft + pc.paddingRight + strict_cast<int>(colCount - 1) * XOURNAL_PADDING_BETWEEN +
           ceil_cast<int>(pc.stretchableHorizontalPixelsAfterColumn.back() * view->getZoom());
}

auto Layout::getPixelCoordinatesOfEntry(xoj::util::Point<int> gridCoords) const -> xoj::util::Point<int> {
    std::lock_guard g{pc.m};
    if (!pc.valid) {
        computePrecalculated();
    }
    return getPixelCoordinatesOfEntryUnsafe(gridCoords);
}

auto Layout::getPixelCoordinatesOfEntry(size_t n) const -> xoj::util::Point<int> {
    std::lock_guard g{pc.m};
    if (!pc.valid) {
        computePrecalculated();
    }
    return getPixelCoordinatesOfEntryUnsafe(n);
}

static xoj::util::Point<int> getPixelCoords(const Layout::PreCalculated& pc, const GridPosition& pos, XojPageView& pv) {
    double zoom = pv.getZoom();
    return {floor_cast<int>(((pos.col == 0 ? 0. : pc.stretchableHorizontalPixelsAfterColumn[pos.col - 1]) +
                             .5 * (pc.widthCols[pos.col] - pv.getWidth())) *
                            zoom) +
                    strict_cast<int>(pos.col) * XOURNAL_PADDING_BETWEEN + pc.paddingLeft,
            floor_cast<int>(((pos.row == 0 ? 0. : pc.stretchableVerticalPixelsAfterRow[pos.row - 1]) +
                             .5 * (pc.heightRows[pos.row] - pv.getHeight())) *
                            zoom) +
                    strict_cast<int>(pos.row) * XOURNAL_PADDING_BETWEEN + pc.paddingTop};
}

auto Layout::getPixelCoordinatesOfEntryUnsafe(xoj::util::Point<int> gridCoords) const -> xoj::util::Point<int> {
    GridPosition pos{size_t(gridCoords.x), size_t(gridCoords.y)};

    auto optionalPage = this->mapper.at(pos);

    if (optionalPage) {
        return getPixelCoords(this->pc, pos, *this->view->viewPages[*optionalPage]);
    } else {
        return {0, 0};
    }
}

auto Layout::getPixelCoordinatesOfEntryUnsafe(size_t n) const -> xoj::util::Point<int> {
    return getPixelCoords(this->pc, this->mapper.at(n), *this->view->viewPages[n]);
}
