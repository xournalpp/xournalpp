#include "Layout.h"

#include <algorithm>    // for max, lower_bound, transform
#include <cmath>        // for abs
#include <iterator>     // for begin, end, distance
#include <numeric>      // for accumulate
#include <optional>     // for optional
#include <type_traits>  // for make_signed_t, remove_referen...

#include <glib-object.h>  // for G_CALLBACK, g_signal_connect

#include "control/Control.h"                 // for Control
#include "control/settings/Settings.h"       // for Settings
#include "control/settings/SettingsEnums.h"  // for LayoutType
#include "gui/LayoutMapper.h"                // for LayoutMapper, GridPosition
#include "gui/PageView.h"                    // for XojPageView
#include "gui/scroll/ScrollHandling.h"       // for ScrollHandling
#include "model/Document.h"                  // for Document
#include "util/Rectangle.h"                  // for Rectangle
#include "util/safe_casts.h"                 // for strict_cast, as_signed, as_si...

#include "XournalView.h"  // for XournalView

using xoj::util::Rectangle;

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
        if (std::abs((layout->getMinimalHeight() - layout->getVisibleRect().y) - layout->getVisibleRect().height) < 5) {
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

void Layout::updateVisibility() {
    Rectangle visRect = getVisibleRect();

    // Step through every possible page position and update using pageView->setIsVisible()

    // Data to select page based on visibility
    std::optional<size_t> mostPageNr;
    double mostPagePercent = 0;

    const auto beginPos = getGridPositionAt(floor_cast<int>(visRect.x), floor_cast<int>(visRect.y));
    const auto endPos =
            getGridPositionAt(ceil_cast<int>(visRect.x + visRect.width), ceil_cast<int>(visRect.y + visRect.height));

    for (size_t row = 0; row < this->mapper.getRows(); ++row) {
        for (size_t col = 0; col < this->mapper.getColumns(); ++col) {
            auto optionalPage = this->mapper.at({col, row});
            if (optionalPage)  // a page exists at this grid location
            {
                auto& pageView = this->view->viewPages[*optionalPage];

                if (row >= beginPos.row && row <= endPos.row && col >= beginPos.col && col <= endPos.col) {
                    // The grid position is within the bounds of the view. Now, use exact check of page itself.
                    auto const& pageRect = pageView->getRect();
                    if (auto intersection = pageRect.intersects(visRect); intersection) {
                        pageView->setIsVisible(true);
                        // Set the selected page
                        double percent = intersection->area() / pageRect.area();

                        if (percent > mostPagePercent) {
                            mostPageNr = *optionalPage;
                            mostPagePercent = percent;
                        }
                    } else {
                        pageView->setIsVisible(false);
                    }
                } else {
                    pageView->setIsVisible(false);
                }
            }
        }
    }

    if (mostPageNr) {
        this->view->getControl()->firePageSelected(*mostPageNr);
    }
}

auto Layout::getVisibleRect() -> Rectangle<double> {
    return Rectangle(gtk_adjustment_get_value(scrollHandling->getHorizontal()),
                     gtk_adjustment_get_value(scrollHandling->getVertical()),
                     gtk_adjustment_get_page_size(scrollHandling->getHorizontal()),
                     gtk_adjustment_get_page_size(scrollHandling->getVertical()));
}

/**
 * adds the addend to base if the predicate is true
 */

[[maybe_unused]] constexpr auto sumIf = [](auto base, auto addend, bool predicate) {
    if constexpr (std::is_signed_v<decltype(base)> || std::is_signed_v<decltype(addend)>) {
        using RT = std::make_signed_t<decltype(base + addend)>;
        if (predicate) {
            return RT(base) + RT(addend);
        }
        return RT(base);
    } else if constexpr (!(std::is_signed_v<decltype(base)> || std::is_signed_v<decltype(addend)>)) {
        using RT = decltype(base + addend);
        if (predicate) {
            return RT(base) + RT(addend);
        }
        return RT(base);
    }
};
void Layout::recalculate_int() const {
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
        pc.widthCols[c] = std::max(pc.widthCols[c], v->getDisplayWidthDouble());
        pc.heightRows[r] = std::max(pc.heightRows[r], v->getDisplayHeightDouble());
    }

    // add space around the entire page area to accommodate older Wacom tablets with limited sense area.
    auto vPadding = 2 * XOURNAL_PADDING;
    if (settings->getUnlimitedScrolling()) {
        vPadding += 2 * static_cast<int>(gtk_adjustment_get_page_size(scrollHandling->getVertical()));
    } else if (settings->getAddVerticalSpace()) {
        vPadding += settings->getAddVerticalSpaceAmountAbove();
        vPadding += settings->getAddVerticalSpaceAmountBelow();
    }

    auto hPadding = 2 * XOURNAL_PADDING;
    if (settings->getUnlimitedScrolling()) {
        hPadding += 2 * static_cast<int>(gtk_adjustment_get_page_size(scrollHandling->getHorizontal()));
    } else if (settings->getAddHorizontalSpace()) {
        hPadding += settings->getAddHorizontalSpaceAmountLeft();
        hPadding += settings->getAddHorizontalSpaceAmountRight();
    }

    pc.minWidth = as_unsigned(hPadding + as_signed_strict((pc.widthCols.size() - 1) * XOURNAL_PADDING_BETWEEN));
    pc.minHeight = as_unsigned(vPadding + as_signed_strict((pc.heightRows.size() - 1) * XOURNAL_PADDING_BETWEEN));

    pc.minWidth = floor_cast<size_t>(std::accumulate(begin(pc.widthCols), end(pc.widthCols), double(pc.minWidth)));
    pc.minHeight = floor_cast<size_t>(std::accumulate(begin(pc.heightRows), end(pc.heightRows), double(pc.minHeight)));
    pc.valid = true;
}

void Layout::recalculate() {
    pc.valid = false;
    gtk_widget_queue_resize(view->getWidget());
}

auto Layout::getPaddingAboveAll() const -> int {
    Settings* settings = this->view->getControl()->getSettings();

    // Minimal padding around the page area in order to accomodate older Wacom tablets
    // with limited sense area
    auto padding = XOURNAL_PADDING;

    if (settings->getUnlimitedScrolling()) {
        // add the widget's size in padding for "unlimited scrolling"
        padding += static_cast<int>(gtk_adjustment_get_page_size(scrollHandling->getVertical()));
    } else if (settings->getAddVerticalSpace()) {
        // add user-defined space above the drawing area
        padding += settings->getAddVerticalSpaceAmountAbove();
    }

    return padding;
}

auto Layout::getPaddingLeftOfAll() const -> int {
    Settings* settings = this->view->getControl()->getSettings();

    // Minimal padding around the page area in order to accomodate older Wacom tablets
    // with limited sense area
    auto padding = XOURNAL_PADDING;

    if (settings->getUnlimitedScrolling()) {
        // add the widget's size in padding for "unlimited scrolling"
        padding += static_cast<int>(gtk_adjustment_get_page_size(scrollHandling->getHorizontal()));
    } else if (settings->getAddHorizontalSpace()) {
        // add user-defined space left of the drawing area
        padding += settings->getAddHorizontalSpaceAmountLeft();
    }

    return padding;
}

void Layout::layoutPages(int width, int height) {
    std::lock_guard g{pc.m};
    if (!pc.valid) {
        recalculate_int();
    }
    // Todo: remove, just a hack-hotfix
    scrollHandling->setLayoutSize(std::max(width, strict_cast<int>(this->pc.minWidth)),
                                  std::max(height, strict_cast<int>(this->pc.minHeight)));

    size_t const len = this->view->viewPages.size();
    Settings* settings = this->view->getControl()->getSettings();

    // Get layout settings from mapper
    auto const rows = this->mapper.getRows();
    auto const columns = this->mapper.getColumns();
    auto const orientation = this->mapper.getOrientation();
    bool const isPairedPages = this->mapper.isPairedPages() && len > 1;
    xoj_assert(!isPairedPages || this->mapper.getColumns() % 2 == 0);

    // Retrieve and store layout type
    this->layoutType = settings->getViewLayoutType();

    // Start with the padding around the page area
    auto v_padding = getPaddingAboveAll();
    auto h_padding = getPaddingLeftOfAll();

    auto const centeringXBorder = (width - as_signed(pc.minWidth)) / 2;
    auto const centeringYBorder = (height - as_signed(pc.minHeight)) / 2;

    using SBig = decltype(as_signed(h_padding * centeringXBorder));
    auto const borderX = static_cast<double>(std::max<SBig>(h_padding, centeringXBorder));
    auto const borderY = static_cast<double>(std::max<SBig>(v_padding, centeringYBorder));

    struct PageLayoutDescription {
        double paddingLeft = 0.0;
        double paddingTop = 0.0;
        double width = 0.0;
        double height = 0.0;
        std::optional<size_t> optionalPage = std::nullopt;
    };

    std::vector<PageLayoutDescription> pages;  // access with r * rows + c
    pages.resize(rows * columns);


    // Iterate over ALL possible rows and columns 3 times:
    // We don't know which page, if any, is to be displayed in each row, column -  ask the mapper object!
    // Once we know the dimensions of all pages, define the paddings above and left of each page as required.
    // Finally, assign page coordinates by propagating page paddings and sizes from the border.

    // first loop: set mapped position in view object and retrieve page dimensions
    for (size_t r = 0; r < rows; r++) {
        for (size_t c = 0; c < columns; c++) {
            const auto index = c * rows + r;
            pages[index].optionalPage = this->mapper.at({c, r});

            if (pages[index].optionalPage) {
                auto& v = this->view->viewPages[*pages[index].optionalPage];

                v->setMappedRowCol(strict_cast<int>(r),
                                   strict_cast<int>(c));  // store row and column for e.g. proper arrow key navigation

                pages[index].width = v->getDisplayWidthDouble();
                pages[index].height = v->getDisplayHeightDouble();
            }
        }
    }

    // second loop: set paddings, width and height according to the layout style and orientation
    if (this->layoutType == LAYOUT_TYPE_GRID) {
        for (size_t r = 0; r < rows; r++) {
            for (size_t c = 0; c < columns; c++) {
                const auto index = c * rows + r;

                const auto columnPadding = this->pc.widthCols[c] - pages[index].width;

                if (isPairedPages) {
                    // pair pages mode
                    if (c % 2 == 0) {
                        // align right
                        pages[index].paddingLeft = XOURNAL_PADDING_BETWEEN - XOURNAL_ROOM_FOR_SHADOW + columnPadding;
                    } else {  // align left
                        pages[index].paddingLeft = XOURNAL_ROOM_FOR_SHADOW;
                    }
                } else {  // not paired page mode - center
                    pages[index].paddingLeft = XOURNAL_PADDING_BETWEEN / 2.0 + columnPadding / 2.0;
                }

                // center page vertically
                pages[index].paddingTop = (this->pc.heightRows[r] - pages[index].height) / 2.0;

                pages[index].width = pc.widthCols[c];
                pages[index].height = pc.heightRows[r];
            }
        }
    } else {  // this->layoutType == LAYOUT_TYPE_CONST_PADDING
        if (orientation == LayoutSettings::Orientation::Horizontal) {
            for (size_t r = 0; r < rows; r++) {
                for (size_t c = 0; c < columns; c++) {
                    const auto index = c * rows + r;

                    if (isPairedPages) {
                        // pair pages mode
                        if (c % 2 == 0) {
                            // align right
                            pages[index].paddingLeft = XOURNAL_PADDING_BETWEEN - XOURNAL_ROOM_FOR_SHADOW;
                        } else {  // align left
                            pages[index].paddingLeft = XOURNAL_ROOM_FOR_SHADOW;
                        }
                    } else {  // not paired page mode - center
                        pages[index].paddingLeft = XOURNAL_PADDING_BETWEEN / 2.0;
                    }

                    // center page vertically
                    pages[index].paddingTop = (this->pc.heightRows[r] - pages[index].height) / 2.0;
                }
            }
        } else {  // orientation == LayoutSettings::Orientation::Vertical
            for (size_t c = 0; c < columns; c++) {
                for (size_t r = 0; r < rows; r++) {
                    const auto index = c * rows + r;

                    auto columnPadding = this->pc.widthCols[c] - pages[index].width;

                    if (isPairedPages) {
                        // pair pages mode
                        if (c % 2 == 0) {
                            // align right
                            pages[index].paddingLeft =
                                    XOURNAL_PADDING_BETWEEN - XOURNAL_ROOM_FOR_SHADOW + columnPadding;
                            if (pages[index + rows].height > pages[index].height) {
                                pages[index].paddingTop = (pages[index + rows].height - pages[index].height) / 2;
                                pages[index].height = pages[index + rows].height;
                            }
                        } else {  // align left
                            pages[index].paddingLeft = XOURNAL_ROOM_FOR_SHADOW;
                            if (pages[index - rows].height > pages[index].height) {
                                pages[index].paddingTop = (pages[index - rows].height - pages[index].height) / 2;
                                pages[index].height = pages[index - rows].height;
                            }
                        }
                    } else {  // not paired page mode - center
                        pages[index].paddingLeft = XOURNAL_PADDING_BETWEEN / 2.0 + columnPadding / 2.0;
                    }
                }
            }
        }
    }

    // initialize x and y here and again in loop below.
    auto x = borderX;
    auto y = borderY;

    // Fill in the row and column positions either in the last loop or using std::transform
    if (this->layoutType == LAYOUT_TYPE_CONST_PADDING && orientation == LayoutSettings::Orientation::Horizontal) {
        this->colXEnd.resize(rows * columns);
    } else {
        this->colXEnd.resize(rows);
    }
    if (this->layoutType == LAYOUT_TYPE_CONST_PADDING && orientation == LayoutSettings::Orientation::Vertical) {
        this->rowYEnd.resize(rows * columns);
    } else {
        this->rowYEnd.resize(columns);
    }

    // third loop: set the position of each page
    if (orientation == LayoutSettings::Orientation::Horizontal) {
        for (size_t r = 0; r < rows; r++) {
            for (size_t c = 0; c < columns; c++) {
                const auto index = c * rows + r;

                if (pages[index].optionalPage) {
                    auto& v = this->view->viewPages[*pages[index].optionalPage];
                    // set the page position
                    v->setX(round_cast<int>(x + pages[index].paddingLeft));
                    v->setY(round_cast<int>(y + pages[index].paddingTop));
                }

                x += pages[index].width + XOURNAL_PADDING_BETWEEN;
                if (this->layoutType == LAYOUT_TYPE_CONST_PADDING) {
                    this->colXEnd[r * columns + c] = floor_cast<unsigned int>(x);
                }
            }
            x = borderX;
            y += this->pc.heightRows[r] + XOURNAL_PADDING_BETWEEN;
        }
    } else {  // orientation == LayoutSettings::Orientation::Vertical
        for (size_t c = 0; c < columns; c++) {
            for (size_t r = 0; r < rows; r++) {
                const auto index = c * rows + r;
                auto optionalPage = this->mapper.at({c, r});

                if (optionalPage) {
                    auto& v = this->view->viewPages[*optionalPage];
                    // set the page position
                    v->setX(round_cast<int>(x + pages[index].paddingLeft));
                    v->setY(round_cast<int>(y + pages[index].paddingTop));
                }

                y += pages[index].height + XOURNAL_PADDING_BETWEEN;
                if (this->layoutType == LAYOUT_TYPE_CONST_PADDING) {
                    this->rowYEnd[index] = floor_cast<unsigned int>(y);
                }
            }
            x += this->pc.widthCols[c] + XOURNAL_PADDING_BETWEEN;
            y = borderY;
        }
    }

    // accumulated - absolute pixel location for use by getGridPositionAt() and updateVisibility()

    if (this->layoutType == LAYOUT_TYPE_GRID || orientation == LayoutSettings::Orientation::Vertical) {
        this->colXEnd.resize(columns);
        auto totalWidth = borderX;
        std::transform(this->pc.widthCols.begin(), this->pc.widthCols.end(), this->colXEnd.begin(),
                       [&totalWidth](auto&& widthCol) {
                           return strict_cast<std::remove_reference_t<decltype(widthCol)>>(
                                   totalWidth += widthCol + XOURNAL_PADDING_BETWEEN);
                       });
    }
    if (this->layoutType == LAYOUT_TYPE_GRID || orientation == LayoutSettings::Orientation::Horizontal) {
        this->rowYEnd.resize(rows);
        auto totalHeight = borderY;
        std::transform(this->pc.heightRows.begin(), this->pc.heightRows.end(), this->rowYEnd.begin(),
                       [&totalHeight](auto&& heightRow) {
                           return strict_cast<std::remove_reference_t<decltype(heightRow)>>(
                                   (totalHeight += heightRow + XOURNAL_PADDING_BETWEEN));
                       });
    }
}


auto Layout::getPaddingAbovePage(size_t pageIndex) const -> int {
    // User-configured padding above all pages.
    const auto paddingAbove = getPaddingAboveAll();

    // (x, y) coordinate pair gives grid indicies. This handles paired pages
    // and different page layouts for us.
    auto pageYLocation = this->mapper.at(pageIndex).row;
    return strict_cast<int>(as_signed(pageYLocation) * XOURNAL_PADDING_BETWEEN + as_signed(paddingAbove));
}


auto Layout::getPaddingLeftOfPage(size_t pageIndex) const -> int {
    bool isPairedPages = this->mapper.isPairedPages();

    // User-configured padding left of all pages.
    const auto paddingBefore = getPaddingLeftOfAll();

    auto const pageXLocation = as_signed(this->mapper.at(pageIndex).col);

    // No page pairing or we haven't rendered enough pages in the row for
    // page pairing to have an effect,
    if (!isPairedPages) {
        return strict_cast<int>(pageXLocation * XOURNAL_PADDING_BETWEEN + XOURNAL_PADDING_BETWEEN / 2 +
                                as_signed(paddingBefore));
    } else {
        auto columnPadding =
                XOURNAL_PADDING_BETWEEN + strict_cast<int>(pageXLocation / 2) * (XOURNAL_PADDING_BETWEEN * 2);
        if (pageXLocation % 2 == 0) {
            return strict_cast<int>(columnPadding - XOURNAL_ROOM_FOR_SHADOW + paddingBefore);
        } else {
            return strict_cast<int>(columnPadding + XOURNAL_ROOM_FOR_SHADOW + paddingBefore);
        }
    }
}

void Layout::setLayoutSize(int width, int height) { this->scrollHandling->setLayoutSize(width, height); }

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

auto Layout::getGridPositionAt(int x, int y) -> GridPosition {
    size_t row = 0;
    size_t col = 0;

    if (this->layoutType == LAYOUT_TYPE_GRID ||
        this->mapper.getOrientation() == LayoutSettings::Orientation::Horizontal) {

        // find row
        const auto rit = std::lower_bound(this->rowYEnd.begin(), this->rowYEnd.end(), y);
        row = size_t(std::distance(this->rowYEnd.begin(), rit));
    }
    if (this->layoutType == LAYOUT_TYPE_GRID ||
        this->mapper.getOrientation() == LayoutSettings::Orientation::Vertical) {

        // find column
        const auto cit = std::lower_bound(this->colXEnd.begin(), this->colXEnd.end(), x);
        col = size_t(std::distance(this->colXEnd.begin(), cit));
    }

    // For constant padding, we need to know the row first in order to look for
    // the column, or vice-versa.
    if (this->layoutType == LAYOUT_TYPE_CONST_PADDING) {
        if (this->mapper.getOrientation() == LayoutSettings::Orientation::Horizontal) {
            // row-major ordering
            const auto rowBeginIt = this->colXEnd.begin() + as_signed(this->mapper.getColumns() * row);
            const auto cit = std::lower_bound(rowBeginIt, rowBeginIt + as_signed(this->mapper.getColumns()), x);
            col = static_cast<size_t>(std::distance(rowBeginIt, cit));
        } else {  // this->mapper.getOrientation() == LayoutSettings::Orientation::Vertical
            // column-major ordering
            const auto colBeginIt = this->rowYEnd.begin() + as_signed(this->mapper.getRows() * col);
            const auto rit = std::lower_bound(colBeginIt, colBeginIt + as_signed(this->mapper.getRows()), y);
            row = static_cast<size_t>(std::distance(colBeginIt, rit));
        }
    }

    return {col, row};
}

auto Layout::getPageViewAt(int x, int y) -> XojPageView* {
    auto optionalPage = this->mapper.at(getGridPositionAt(x, y));

    if (optionalPage && this->view->viewPages[*optionalPage]->containsPoint(x, y, false)) {
        return this->view->viewPages[*optionalPage].get();
    }

    return nullptr;
}

auto Layout::getPageIndexAtGridMap(size_t row, size_t col) -> std::optional<size_t> {
    return this->mapper.at({col, row});  // watch out.. x,y --> c,r
}

auto Layout::getMinimalHeight() const -> int {
    std::lock_guard g{pc.m};
    if (!pc.valid) {
        recalculate_int();
    }
    return strict_cast<int>(this->pc.minHeight);
}

auto Layout::getMinimalWidth() const -> int {
    std::lock_guard g{pc.m};
    if (!pc.valid) {
        recalculate_int();
    }
    return strict_cast<int>(this->pc.minWidth);
}
