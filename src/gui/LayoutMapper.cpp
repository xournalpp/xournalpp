#include "gui/LayoutMapper.h"

#include <algorithm>

/**
 * calculate
 *
 * Set mapper to LayoutType with number of pages and of fixed rows or columns
 * @param data the data to be configured
 * @param numRows Number of rows ( used if useRows )
 * @param numCols  Number of columns ( used if !useRows )
 * @param useRows  use pages/rows to recalculate cols else recalculate rows
 * @param firstPageOffset  Pages to offset - usually one or zero in order to pair up properly
 */

void calculate(LayoutMapper::internal_data& data, size_t numRows, size_t numCols, bool useRows, int firstPageOffset) {
    if (useRows) {
        data.rows = std::max<size_t>(1UL, numRows);

        // using  + ( rows-1) to round up (int)pages/rows
        data.cols = std::max<size_t>(1UL, (data.actualPages + firstPageOffset + (data.rows - 1)) / data.rows);
        if (data.showPairedPages) {
            data.cols += data.cols % 2;  // make even
        }
    } else {
        data.cols = std::max<size_t>(1UL, numCols);
        if (data.showPairedPages) {
            data.cols += data.cols % 2;  // make even
        }
        data.rows = std::max<size_t>(1UL, (data.actualPages + firstPageOffset + (data.cols - 1)) / data.cols);
    }


    if (data.orientation == LayoutMapper::Vertical) {
        // Vertical Layout
        if (data.showPairedPages) {
            data.offset = firstPageOffset % (2 * data.rows);
        } else {
            data.offset = firstPageOffset % data.rows;
        }
    } else {
        // Horizontal Layout
        data.offset = firstPageOffset % data.cols;
    }
}

void LayoutMapper::configureFromSettings(size_t numPages, Settings* settings) {
    internal_data data;
    // get from user settings:
    data.actualPages = numPages;
    data.showPairedPages = settings->isShowPairedPages();
    const int pairsOffset = data.showPairedPages ? settings->getPairsOffset() : 0;

    const bool fixRows = settings->isPresentationMode() ? false : settings->isViewFixedRows();
    const size_t numCols = settings->isPresentationMode() ? 1 : settings->getViewColumns();
    const size_t numRows = settings->isPresentationMode() ? 1 : settings->getViewRows();

    // assemble bitflags for LayoutType
    data.orientation = (settings->isPresentationMode() || settings->getViewLayoutVert()) ? Vertical : Horizontal;
    data.horizontalDir = settings->getViewLayoutR2L() ? RightToLeft : LeftToRight;
    data.verticalDir = settings->getViewLayoutB2T() ? BottomToTop : TopToBottom;

    calculate(data, numRows, numCols, fixRows, pairsOffset);
    if (data == data_) {
        return;
    }
    data_ = data;
    precalculateMappers();
}

void LayoutMapper::precalculateMappers() {
    this->pageToRaster.clear();
    this->rasterToPage.clear();
    this->pageToRaster.resize(data_.actualPages);
    this->rasterToPage.reserve(data_.rows * data_.cols);

    for (size_t col{}; col < data_.cols; ++col) {
        for (size_t row{}; row < data_.rows; ++row) {
            auto maybe_index = map(col, row);
            if (maybe_index) {
                this->pageToRaster[*maybe_index] = std::pair<size_t, size_t>{col, row};
                this->rasterToPage.emplace(std::pair<size_t, size_t>{col, row}, *maybe_index);
            }
        }
    }
}

auto LayoutMapper::getColumns() const -> size_t { return data_.cols; }

auto LayoutMapper::getRows() const -> size_t { return data_.rows; }

auto LayoutMapper::getFirstPageOffset() const -> int { return data_.offset; }

auto LayoutMapper::isPairedPages() const -> bool { return data_.showPairedPages; }


// Todo: replace with map<pair(x,y)> -> index and vector<index> -> pair(x,y)
//       precalculate it in configure
auto LayoutMapper::map(size_t col, size_t row) const -> std::optional<size_t> {
    if (isRightToLeft()) {
        // reverse x
        col = data_.cols - 1 - col;
    }

    if (isBottomToTop()) {
        // reverse y
        row = data_.rows - 1 - row;
    }

    size_t res = 0;
    if (isVertical()) {
        if (data_.showPairedPages) {
            res = ((row + data_.rows * (col / 2)) * 2) + col % 2;
        } else {
            res = row + data_.rows * col;
        }
    } else  // Horizontal
    {
        res = col + data_.cols * row;
    }

    res -= data_.offset;

    if (res >= data_.actualPages) {
        return {};
    }

    return res;
}

auto LayoutMapper::isVertical() const -> bool { return data_.orientation == Vertical; }

auto LayoutMapper::isBottomToTop() const -> bool { return data_.verticalDir == BottomToTop; }

auto LayoutMapper::isRightToLeft() const -> bool { return data_.horizontalDir == RightToLeft; }

auto LayoutMapper::at(size_t page) const -> std::pair<size_t, size_t> { return pageToRaster.at(page); }

auto LayoutMapper::at(std::pair<size_t, size_t> rasterXY) const -> std::optional<size_t> {
    if (auto iter = rasterToPage.find(rasterXY); iter != end(rasterToPage)) {
        return iter->second;
    }
    return std::nullopt;
}
