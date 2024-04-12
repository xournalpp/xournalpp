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
#include "util/Rectangle.h"             // for Rectangle
#include "util/safe_casts.h"            // for strict_cast, as_signed, as_si...

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

    // step through every possible page position and update using p->setIsVisible()
    // Using initial grid aprox speeds things up by a factor of 5.  See previous git check-in for specifics.
    int x1 = 0;
    int y1 = 0;

    // Data to select page based on visibility
    std::optional<size_t> mostPageNr;
    double mostPagePercent = 0;

    for (size_t row = 0; row < this->rowYStart.size(); ++row) {
        auto y2 = as_signed_strict(this->rowYStart[row]);
        for (size_t col = 0; col < this->colXStart.size(); ++col) {
            auto x2 = as_signed_strict(this->colXStart[col]);
            auto optionalPage = this->mapper.at({col, row});
            if (optionalPage)  // a page exists at this grid location
            {
                auto& pageView = this->view->viewPages[*optionalPage];

                // check if grid location is visible as an aprox for page visiblity:
                if (!(visRect.x > x2 || visRect.x + visRect.width < x1)  // visrect not outside current row/col
                    && !(visRect.y > y2 || visRect.y + visRect.height < y1)) {
                    // now use exact check of page itself:
                    // visrect not outside current page dimensions:
                    auto const& pageRect = pageView->getRect();
                    if (auto intersection = pageRect.intersects(visRect); intersection) {
                        pageView->setIsVisible(true);
                        // Set the selected page
                        double percent = intersection->area() / pageRect.area();

                        if (percent > mostPagePercent) {
                            mostPageNr = *optionalPage;
                            mostPagePercent = percent;
                        }
                    }
                } else {
                    pageView->setIsVisible(false);
                }
            }
            x1 = x2;
        }
        y1 = y2;
        x1 = 0;
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

    // get from mapper (some may have changed to accommodate paired setting etc.)
    bool const isPairedPages = this->mapper.isPairedPages();

    auto const rows = this->pc.heightRows.size();
    auto const columns = this->pc.widthCols.size();


    // add space around the entire page area to accommodate older Wacom tablets with limited sense area.
    auto v_padding = XOURNAL_PADDING;
    if (settings->getUnlimitedScrolling()) {
        v_padding += static_cast<int>(gtk_adjustment_get_page_size(scrollHandling->getVertical()));
    } else if (settings->getAddVerticalSpace()) {
        v_padding += settings->getAddVerticalSpaceAmountAbove();
    }

    auto h_padding = XOURNAL_PADDING;
    if (settings->getUnlimitedScrolling()) {
        h_padding += static_cast<int>(gtk_adjustment_get_page_size(scrollHandling->getHorizontal()));
    } else if (settings->getAddHorizontalSpace()) {
        h_padding += settings->getAddHorizontalSpaceAmountLeft();
    }

    auto const centeringXBorder = (width - as_signed(pc.minWidth)) / 2;
    auto const centeringYBorder = (height - as_signed(pc.minHeight)) / 2;

    using SBig = decltype(as_signed(h_padding * centeringXBorder));
    auto const borderX = static_cast<double>(std::max<SBig>(h_padding, centeringXBorder));
    auto const borderY = static_cast<double>(std::max<SBig>(v_padding, centeringYBorder));

    // initialize here and x again in loop below.
    auto x = borderX;
    auto y = borderY;


    // Iterate over ALL possible rows and columns.
    // We don't know which page, if any,  is to be displayed in each row, column -  ask the mapper object!
    // Then assign that page coordinates with center, left or right justify within row,column grid cell as required.
    for (size_t r = 0; r < rows; r++) {
        for (size_t c = 0; c < columns; c++) {
            auto optionalPage = this->mapper.at({c, r});

            if (optionalPage) {

                auto& v = this->view->viewPages[*optionalPage];
                v->setMappedRowCol(strict_cast<int>(r),
                                   strict_cast<int>(c));  // store row and column for e.g. proper arrow key navigation
                auto vDisplayWidth = v->getDisplayWidthDouble();
                {
                    auto paddingLeft = 0.0;
                    auto paddingRight = 0.0;
                    auto columnPadding = this->pc.widthCols[c] - vDisplayWidth;

                    if (isPairedPages && len > 1) {
                        // pair pages mode
                        if (c % 2 == 0) {
                            // align right
                            paddingLeft = XOURNAL_PADDING_BETWEEN - XOURNAL_ROOM_FOR_SHADOW + columnPadding;
                            paddingRight = XOURNAL_ROOM_FOR_SHADOW;
                        } else {  // align left
                            paddingLeft = XOURNAL_ROOM_FOR_SHADOW;
                            paddingRight = XOURNAL_PADDING_BETWEEN - XOURNAL_ROOM_FOR_SHADOW + columnPadding;
                        }
                    } else {  // not paired page mode - center
                        paddingLeft = XOURNAL_PADDING_BETWEEN / 2.0 + columnPadding / 2.0;  // center justify
                        paddingRight = XOURNAL_PADDING_BETWEEN - paddingLeft + columnPadding / 2.0;
                    }

                    x += paddingLeft;

                    v->setX(floor_cast<int>(x));  // set the page position
                    v->setY(floor_cast<int>(y));

                    x += vDisplayWidth + paddingRight;
                }
            } else {
                x += this->pc.widthCols[c] + XOURNAL_PADDING_BETWEEN;
            }
        }
        x = borderX;
        y += this->pc.heightRows[r] + XOURNAL_PADDING_BETWEEN;
    }

    this->colXStart.resize(this->pc.widthCols.size());
    this->rowYStart.resize(this->pc.heightRows.size());


    // accumulated - absolute pixel location for use by getViewAt() and updateVisibility()
    auto totalWidth = borderX;
    std::transform(
            begin(this->pc.widthCols), end(this->pc.widthCols), begin(this->colXStart), [&totalWidth](auto&& widthCol) {
                return strict_cast<std::remove_reference_t<decltype(widthCol)>>(totalWidth +=
                                                                                widthCol + XOURNAL_PADDING_BETWEEN);
            });
    auto totalHeight = borderY;
    std::transform(begin(this->pc.heightRows), end(this->pc.heightRows), begin(this->rowYStart),
                   [&totalHeight](auto&& heightRow) {
                       return strict_cast<std::remove_reference_t<decltype(heightRow)>>(
                               (totalHeight += heightRow + XOURNAL_PADDING_BETWEEN));
                   });
}


auto Layout::getPaddingAbovePage(size_t pageIndex) const -> int {
    const Settings* settings = this->view->getControl()->getSettings();

    // User-configured padding above all pages.
    auto paddingAbove = XOURNAL_PADDING;
    if (settings->getUnlimitedScrolling()) {
        paddingAbove += static_cast<int>(gtk_adjustment_get_page_size(scrollHandling->getVertical()));
    } else if (settings->getAddVerticalSpace()) {
        paddingAbove += settings->getAddVerticalSpaceAmountAbove();
    }

    // (x, y) coordinate pair gives grid indicies. This handles paired pages
    // and different page layouts for us.
    auto pageYLocation = this->mapper.at(pageIndex).row;
    return strict_cast<int>(as_signed(pageYLocation) * XOURNAL_PADDING_BETWEEN + as_signed(paddingAbove));
}


auto Layout::getPaddingLeftOfPage(size_t pageIndex) const -> int {
    bool isPairedPages = this->mapper.isPairedPages();
    const Settings* settings = this->view->getControl()->getSettings();

    auto paddingBefore = XOURNAL_PADDING;
    if (settings->getUnlimitedScrolling()) {
        paddingBefore += static_cast<int>(gtk_adjustment_get_page_size(scrollHandling->getHorizontal()));
    } else if (settings->getAddHorizontalSpace()) {
        paddingBefore += settings->getAddHorizontalSpaceAmountLeft();
    }

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
    // Binary Search:
    auto rit = std::lower_bound(this->rowYStart.begin(), this->rowYStart.end(), y);
    auto const foundRow = size_t(std::distance(this->rowYStart.begin(), rit));
    auto cit = std::lower_bound(this->colXStart.begin(), this->colXStart.end(), x);
    auto const foundCol = size_t(std::distance(this->colXStart.begin(), cit));

    auto optionalPage = this->mapper.at({foundCol, foundRow});

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
