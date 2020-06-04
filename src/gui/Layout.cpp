#include "Layout.h"

#include <algorithm>
#include <cstdlib>
#include <numeric>
#include <optional>

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
    this->hadjHandler = g_signal_connect(scrollHandling->getHorizontal(), "value-changed",
                                         G_CALLBACK(horizontalScrollChanged), this);

    this->vadjHandler =
            g_signal_connect(scrollHandling->getVertical(), "value-changed", G_CALLBACK(verticalScrollChanged), this);


    lastScrollHorizontal = gtk_adjustment_get_value(scrollHandling->getHorizontal());
    lastScrollVertical = gtk_adjustment_get_value(scrollHandling->getVertical());
}

void Layout::horizontalScrollChanged(GtkAdjustment* adjustment, Layout* layout) {
    g_signal_handler_block(layout->scrollHandling->getHorizontal(), layout->hadjHandler);
    Layout::checkScroll(adjustment, layout->lastScrollHorizontal);
    layout->updateVisibility();
    layout->scrollHandling->scrollChanged();
    g_signal_handler_unblock(layout->scrollHandling->getHorizontal(), layout->hadjHandler);
}

void Layout::verticalScrollChanged(GtkAdjustment* adjustment, Layout* layout) {
    g_signal_handler_block(layout->scrollHandling->getVertical(), layout->vadjHandler);
    Layout::checkScroll(adjustment, layout->lastScrollVertical);
    layout->updateVisibility();
    layout->scrollHandling->scrollChanged();
    g_signal_handler_unblock(layout->scrollHandling->getVertical(), layout->vadjHandler);
}

Layout::~Layout() = default;

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

    for (size_t row = 0; row < this->heightRows.size(); ++row) {
        int y2 = this->heightRows[row];
        for (size_t col = 0; col < this->widthCols.size(); ++col) {
            int x2 = this->widthCols[col];
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


void Layout::recalculate() {
    auto* settings = view->getControl()->getSettings();
    size_t len = view->viewPages.size();
    mapper.configureFromSettings(len, settings);
    size_t colCount = mapper.getColumns();
    size_t rowCount = mapper.getRows();

    widthCols.assign(colCount, 0);
    heightRows.assign(rowCount, 0);

    for (size_t pageIdx{}; pageIdx < len; ++pageIdx) {
        auto const& raster_p = mapper.at(pageIdx);  // auto [c, r] raster = mapper.at();
        auto const& c = raster_p.first;
        auto const& r = raster_p.second;
        XojPageView* v = view->viewPages[pageIdx];
        widthCols[c] = std::max<unsigned>(widthCols[c], v->getDisplayWidth());
        heightRows[r] = std::max<unsigned>(heightRows[r], v->getDisplayHeight());
    }

    // add space around the entire page area to accomodate older Wacom tablets with limited sense area.
    size_t const vPadding =
            sumIf(XOURNAL_PADDING, settings->getAddVerticalSpaceAmount(), settings->getAddVerticalSpace());
    size_t const hPadding =
            sumIf(XOURNAL_PADDING, settings->getAddHorizontalSpaceAmount(), settings->getAddHorizontalSpace());

    minWidth = 2 * hPadding + (widthCols.size() - 1) * XOURNAL_PADDING_BETWEEN;
    minHeight = 2 * vPadding + (heightRows.size() - 1) * XOURNAL_PADDING_BETWEEN;

    minWidth = std::accumulate(begin(widthCols), end(widthCols), minWidth);
    minHeight = std::accumulate(begin(heightRows), end(heightRows), minHeight);

    setLayoutSize(minWidth, minHeight);
    valid = true;
}

void Layout::layoutPages(int width, int height) {
    if (!valid) {
        recalculate();
    }
    valid = false;

    size_t const len = this->view->viewPages.size();
    Settings* settings = this->view->getControl()->getSettings();

    // get from mapper (some may have changed to accomodate paired setting etc.)
    bool const isPairedPages = this->mapper.isPairedPages();

    auto const rows = this->heightRows.size();
    auto const columns = this->widthCols.size();


    // add space around the entire page area to accomodate older Wacom tablets with limited sense area.
    int64_t const v_padding =
            sumIf(XOURNAL_PADDING, settings->getAddVerticalSpaceAmount(), settings->getAddVerticalSpace());
    int64_t const h_padding =
            sumIf(XOURNAL_PADDING, settings->getAddHorizontalSpaceAmount(), settings->getAddHorizontalSpace());

    int64_t const centeringXBorder = static_cast<int64_t>(width - minWidth) / 2;
    int64_t const centeringYBorder = static_cast<int64_t>(height - minHeight) / 2;

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
                    auto columnPadding = static_cast<int64_t>(this->widthCols[c] - vDisplayWidth);

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
                x += this->widthCols[c] + XOURNAL_PADDING_BETWEEN;
            }
        }
        x = borderX;
        y += this->heightRows[r] + XOURNAL_PADDING_BETWEEN;
    }

    int64_t totalWidth = borderX;
    for (auto&& widthCol: this->widthCols) {
        // accumulated - absolute pixel location for use by getViewAt() and updateVisibility()
        totalWidth += widthCol + XOURNAL_PADDING_BETWEEN;
        widthCol = totalWidth;
    }

    int64_t totalHeight = borderY;
    for (auto&& heightRow: this->heightRows) {
        totalHeight += heightRow + XOURNAL_PADDING_BETWEEN;
        heightRow = totalHeight;
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


auto Layout::getViewAt(int x, int y) -> XojPageView* {
    // Binary Search:
    auto rit = std::lower_bound(this->heightRows.begin(), this->heightRows.end(), y);
    int const foundRow = std::distance(this->heightRows.begin(), rit);
    auto cit = std::lower_bound(this->widthCols.begin(), this->widthCols.end(), x);
    int const foundCol = std::distance(this->widthCols.begin(), cit);

    auto optionalPage = this->mapper.at({foundCol, foundRow});

    if (optionalPage && this->view->viewPages[*optionalPage]->containsPoint(x, y, false)) {
        return this->view->viewPages[*optionalPage];
    }

    return nullptr;
}

// Todo replace with boost::optional<size_t> Layout::getIndexAtGridMap(size_t row, size_t col)
//                  or std::optional<size_t> Layout::getIndexAtGridMap(size_t row, size_t col)
auto Layout::getIndexAtGridMap(size_t row, size_t col) -> std::optional<size_t> {
    return this->mapper.at({col, row});  // watch out.. x,y --> c,r
}

auto Layout::getMinimalHeight() const -> int { return this->minHeight; }

auto Layout::getMinimalWidth() const -> int { return this->minWidth; }
