#include "Layout.h"

#include <algorithm>
#include <cstdlib>
#include <numeric>
#include <optional>
#include <utility>

#include "control/Control.h"
#include "gui/scroll/ScrollHandling.h"
#include "widgets/XournalWidget.h"

#include "XournalView.h"
/**
 * Padding outside the pages, including shadow
 */
constexpr size_t const XOURNAL_PADDING = 10;

/**
 * Allowance for shadow between page pairs in paired page mode
 */
constexpr size_t const XOURNAL_ROOM_FOR_SHADOW = 3;

/**
 * Padding between the pages
 */
constexpr size_t const XOURNAL_PADDING_BETWEEN = 15;


Layout::Layout(XournalView* view, ScrollHandling* scrollHandling): view(view), scrollHandling(scrollHandling) {
    g_signal_connect(scrollHandling->getHorizontal(), "value-changed", G_CALLBACK(horizontalScrollChanged), this);
    g_signal_connect(scrollHandling->getVertical(), "value-changed", G_CALLBACK(verticalScrollChanged), this);


    lastScrollHorizontal = gtk_adjustment_get_value(scrollHandling->getHorizontal());
    lastScrollVertical = gtk_adjustment_get_value(scrollHandling->getVertical());
}

void Layout::horizontalScrollChanged(GtkAdjustment* adjustment, Layout* layout) {
    Layout::checkScroll(adjustment, layout->lastScrollHorizontal);
    layout->updateVisibility();
    layout->scrollHandling->scrollChanged();
}

void Layout::verticalScrollChanged(GtkAdjustment* adjustment, Layout* layout) {
    Layout::checkScroll(adjustment, layout->lastScrollVertical);
    layout->updateVisibility();
    layout->scrollHandling->scrollChanged();
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
        int y2 = this->rowYStart[row];
        for (size_t col = 0; col < this->colXStart.size(); ++col) {
            int x2 = this->colXStart[col];
            auto optionalPage = this->mapper.at({col, row});
            if (optionalPage)  // a page exists at this grid location
            {
                XojPageView* pageView = this->view->viewPages[*optionalPage];


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
inline auto sumIf(size_t base, size_t addend, bool predicate) -> size_t {
    if (predicate) {
        return base + addend;
    }
    return base;
}

void Layout::recalculate_int() const {
    auto* settings = view->getControl()->getSettings();
    size_t len = view->viewPages.size();
    mapper.configureFromSettings(len, settings);
    size_t colCount = mapper.getColumns();
    size_t rowCount = mapper.getRows();

    pc.widthCols.assign(colCount, 0);
    pc.heightRows.assign(rowCount, 0);

    for (size_t pageIdx{}; pageIdx < len; ++pageIdx) {
        auto const& raster_p = mapper.at(pageIdx);  // auto [c, r] raster = mapper.at();
        auto const& c = raster_p.first;
        auto const& r = raster_p.second;
        XojPageView* v = view->viewPages[pageIdx];
        pc.widthCols[c] = std::max<unsigned>(pc.widthCols[c], v->getDisplayWidth());
        pc.heightRows[r] = std::max<unsigned>(pc.heightRows[r], v->getDisplayHeight());
    }

    // add space around the entire page area to accommodate older Wacom tablets with limited sense area.
    size_t const vPadding =
            sumIf(XOURNAL_PADDING, settings->getAddVerticalSpaceAmount(), settings->getAddVerticalSpace());
    size_t const hPadding =
            sumIf(XOURNAL_PADDING, settings->getAddHorizontalSpaceAmount(), settings->getAddHorizontalSpace());

    pc.minWidth = 2 * hPadding + (pc.widthCols.size() - 1) * XOURNAL_PADDING_BETWEEN;
    pc.minHeight = 2 * vPadding + (pc.heightRows.size() - 1) * XOURNAL_PADDING_BETWEEN;

    pc.minWidth = std::accumulate(begin(pc.widthCols), end(pc.widthCols), pc.minWidth);
    pc.minHeight = std::accumulate(begin(pc.heightRows), end(pc.heightRows), pc.minHeight);
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
    scrollHandling->setLayoutSize(width, height);

    size_t const len = this->view->viewPages.size();
    Settings* settings = this->view->getControl()->getSettings();

    // get from mapper (some may have changed to accommodate paired setting etc.)
    bool const isPairedPages = this->mapper.isPairedPages();

    auto const rows = this->pc.heightRows.size();
    auto const columns = this->pc.widthCols.size();


    // add space around the entire page area to accommodate older Wacom tablets with limited sense area.
    int64_t const v_padding =
            sumIf(XOURNAL_PADDING, settings->getAddVerticalSpaceAmount(), settings->getAddVerticalSpace());
    int64_t const h_padding =
            sumIf(XOURNAL_PADDING, settings->getAddHorizontalSpaceAmount(), settings->getAddHorizontalSpace());

    int64_t const centeringXBorder = static_cast<int64_t>(width - pc.minWidth) / 2;
    int64_t const centeringYBorder = static_cast<int64_t>(height - pc.minHeight) / 2;

    int64_t const borderX = std::max<int64_t>(h_padding, centeringXBorder);
    int64_t const borderY = std::max<int64_t>(v_padding, centeringYBorder);

    // initialize here and x again in loop below.
    int64_t x = borderX;
    int64_t y = borderY;


    // Iterate over ALL possible rows and columns.
    // We don't know which page, if any,  is to be displayed in each row, column -  ask the mapper object!
    // Then assign that page coordinates with center, left or right justify within row,column grid cell as required.
    for (size_t r = 0; r < rows; r++) {
        for (size_t c = 0; c < columns; c++) {
            auto optionalPage = this->mapper.at({c, r});

            if (optionalPage) {

                XojPageView* v = this->view->viewPages[*optionalPage];
                v->setMappedRowCol(r, c);  // store row and column for e.g. proper arrow key navigation
                int64_t vDisplayWidth = v->getDisplayWidth();
                {
                    int64_t paddingLeft = 0;
                    int64_t paddingRight = 0;
                    auto columnPadding = static_cast<int64_t>(this->pc.widthCols[c] - vDisplayWidth);

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
                    } else {                                                            // not paired page mode - center
                        paddingLeft = XOURNAL_PADDING_BETWEEN / 2 + columnPadding / 2;  // center justify
                        paddingRight = XOURNAL_PADDING_BETWEEN - paddingLeft + columnPadding / 2;
                    }

                    x += paddingLeft;

                    v->setX(x);  // set the page position
                    v->setY(y);

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
    int64_t totalWidth = borderX;
    std::transform(begin(this->pc.widthCols), end(this->pc.widthCols), begin(this->colXStart),
                   [&totalWidth](auto&& widthCol) { return (totalWidth += widthCol + XOURNAL_PADDING_BETWEEN); });
    int64_t totalHeight = borderY;
    std::transform(begin(this->pc.heightRows), end(this->pc.heightRows), begin(this->rowYStart),
                   [&totalHeight](auto&& heightRow) { return (totalHeight += heightRow + XOURNAL_PADDING_BETWEEN); });
}

int Layout::getPaddingAbovePage(size_t pageIndex) const {
    const Settings* settings = this->view->getControl()->getSettings();

    // User-configured padding above all pages.
    auto const paddingAbove =
            sumIf(XOURNAL_PADDING, settings->getAddHorizontalSpaceAmount(), settings->getAddVerticalSpace());

    // (x, y) coordinate pair gives grid indicies. This handles paired pages
    // and different page layouts for us.
    std::pair<size_t, size_t> pageGridLocation = this->mapper.at(pageIndex);
    size_t pageYLocation = std::get<1>(pageGridLocation);

    return pageYLocation * XOURNAL_PADDING_BETWEEN + paddingAbove;
}


int Layout::getPaddingLeftOfPage(size_t pageIndex) const {
    bool isPairedPages = this->mapper.isPairedPages();
    const Settings* settings = this->view->getControl()->getSettings();

    auto const paddingBefore =
            sumIf(XOURNAL_PADDING, settings->getAddVerticalSpaceAmount(), settings->getAddHorizontalSpace());

    std::pair<size_t, size_t> pageGridLocation = this->mapper.at(pageIndex);
    size_t pageXLocation = std::get<0>(pageGridLocation);

    // No page pairing or we haven't rendered enough pages in the row for
    // page pairing to have an effect,
    if (!isPairedPages || pageXLocation == 0) {
        return pageXLocation * XOURNAL_PADDING_BETWEEN + paddingBefore;
    } else {
        // We have a greater separation between pairs of pages. Handle this here,
        //  Note that pageXLocation - 1 >= 0 because we take the if branch above when
        // pageXLocation == 0.
        int paddingBetweenPairs = (pageXLocation - 1) / 2 * XOURNAL_PADDING_BETWEEN;

        // The two pages within each pair have a smaller separation, XOURNAL_ROOM_FOR_SHADOW.
        int shadowRoomInsidePairs = pageXLocation / 2 * XOURNAL_ROOM_FOR_SHADOW;

        return paddingBetweenPairs + shadowRoomInsidePairs + paddingBefore;
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


auto Layout::getPageViewAt(int x, int y) -> XojPageView* {
    // Binary Search:
    auto rit = std::lower_bound(this->rowYStart.begin(), this->rowYStart.end(), y);
    int const foundRow = std::distance(this->rowYStart.begin(), rit);
    auto cit = std::lower_bound(this->colXStart.begin(), this->colXStart.end(), x);
    int const foundCol = std::distance(this->colXStart.begin(), cit);

    auto optionalPage = this->mapper.at({foundCol, foundRow});

    if (optionalPage && this->view->viewPages[*optionalPage]->containsPoint(x, y, false)) {
        return this->view->viewPages[*optionalPage];
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
    return this->pc.minHeight;
}

auto Layout::getMinimalWidth() const -> int {
    std::lock_guard g{pc.m};
    if (!pc.valid) {
        recalculate_int();
    }
    return this->pc.minWidth;
}
