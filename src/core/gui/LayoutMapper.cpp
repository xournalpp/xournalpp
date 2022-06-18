#include "LayoutMapper.h"

#include <algorithm>  // for max
#include <map>        // for __alloc_traits<>::value_type

#include "control/settings/Settings.h"  // for Settings

constexpr auto calc_default_grid_pos = [](size_t page_nr, GridPosition grid_size,
                                          LayoutSettings::Orientation orientation) {
    return orientation == LayoutSettings::Orientation::Vertical ?
                   GridPosition{page_nr / grid_size.row, page_nr % grid_size.row} :
                   GridPosition{page_nr % grid_size.col, page_nr / grid_size.col};
};

constexpr auto map1 = [](LayoutSettings const& data, size_t page_nr) {
    GridPosition grid_pos{};
    auto new_nr = page_nr + static_cast<size_t>(data.offset);
    if (data.showPairedPages) {
        auto grid_size = GridPosition{data.cols / 2, data.rows};
        grid_pos = calc_default_grid_pos(new_nr / 2, grid_size, data.orientation);
        (grid_pos.col *= 2) += (new_nr % 2);
    } else {
        grid_pos = calc_default_grid_pos(new_nr, {data.cols, data.rows}, data.orientation);
    }
    decltype(grid_pos) grid_pos2 = {
            (data.horizontalDir == LayoutSettings::HorizontalDirection::RightToLeft ? data.cols - 1 - grid_pos.col :
                                                                                      grid_pos.col),
            (data.verticalDir == LayoutSettings::VerticalDirection::BottomToTop ? data.rows - 1 - grid_pos.row :
                                                                                  grid_pos.row)};
    return grid_pos2;
};

constexpr auto map2 = [](LayoutSettings const& data, GridPosition pos) -> std::optional<size_t> {
    auto isRightToLeft = [&]() { return data.horizontalDir == LayoutSettings::RightToLeft; };
    auto isBottomToTop = [&]() { return data.verticalDir == LayoutSettings::BottomToTop; };
    auto isVertical = [&]() { return data.orientation == LayoutSettings::Vertical; };

    if (isRightToLeft()) {  // reverse x
        pos.col = data.cols - 1 - pos.col;
    }
    if (isBottomToTop()) {  // reverse y
        pos.row = data.rows - 1 - pos.row;
    }
    size_t res{};
    if (isVertical()) {
        if (data.showPairedPages) {
            res = ((pos.row + data.rows * (pos.col / 2)) * 2) + pos.col % 2;
        } else {
            res = pos.row + data.rows * pos.col;
        }
    } else {  // Horizontal
        res = pos.col + data.cols * pos.row;
    }
    res -= data.offset;
    if (res >= data.actualPages) {
        return {};
    }
    return res;
};

// Testers

constexpr LayoutSettings my_data0{1, 91, 92, 1};
constexpr LayoutSettings my_data1{91, 1, 90, 1};
constexpr LayoutSettings my_data2{
        6, 6, 35, 1, true, LayoutSettings::Vertical, LayoutSettings::RightToLeft, LayoutSettings::BottomToTop};
constexpr GridPosition test_pos0{0, 3};
constexpr GridPosition test_pos1{3, 0};
constexpr GridPosition test_pos2{3, 3};

static_assert(map1(my_data0, *map2(my_data0, test_pos0)) == test_pos0);
static_assert(map1(my_data1, *map2(my_data1, test_pos1)) == test_pos1);
static_assert(map1(my_data2, *map2(my_data2, test_pos2)) == test_pos2);

template <typename T>
constexpr auto hash(T&& data_, GridPosition const& pos) {
    return pos.col + pos.row * data_.cols;
}


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

void calculate(LayoutSettings& data, size_t numRows, size_t numCols, bool useRows, uint32_t firstPageOffset) {
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


    if (data.orientation == LayoutSettings::Vertical) {
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
    LayoutSettings data;
    // get from user settings:
    data.actualPages = numPages;
    data.showPairedPages = settings->isShowPairedPages();
    auto const& pairsOffset = data.showPairedPages ? static_cast<uint32_t>(settings->getPairsOffset()) : 0U;

    auto const& isPresentationMode = settings->isPresentationMode();
    auto const& fixRows = !isPresentationMode && settings->isViewFixedRows();
    auto const& numCols = size_t(isPresentationMode ? 1 : settings->getViewColumns());
    auto const& numRows = size_t(isPresentationMode ? 1 : settings->getViewRows());

    // assemble bitflags for LayoutType
    data.orientation = (isPresentationMode || settings->getViewLayoutVert()) ? LayoutSettings::Vertical :
                                                                               LayoutSettings::Horizontal;
    data.horizontalDir = settings->getViewLayoutR2L() ? LayoutSettings::RightToLeft : LayoutSettings::LeftToRight;
    data.verticalDir = settings->getViewLayoutB2T() ? LayoutSettings::BottomToTop : LayoutSettings::TopToBottom;

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
    this->rasterToPage.resize(data_.rows * data_.cols, size_t(-1));

    for (size_t i = 0; i != data_.actualPages; ++i) {
        auto&& val = map1(data_, i);
        this->pageToRaster[i] = val;
        this->rasterToPage[hash(data_, val)] = i;
    }
}

auto LayoutMapper::getColumns() const -> size_t { return data_.cols; }

auto LayoutMapper::getRows() const -> size_t { return data_.rows; }

auto LayoutMapper::isPairedPages() const -> bool { return data_.showPairedPages; }

auto LayoutMapper::at(size_t page) const -> GridPosition { return pageToRaster.at(page); }

auto LayoutMapper::at(GridPosition const& rasterXY) const -> std::optional<size_t> {
    auto index = hash(data_, rasterXY);
    if (index >= rasterToPage.size()) {
        return std::nullopt;
    }
    auto val = rasterToPage[index];
    return val >= data_.actualPages ? std::nullopt : std::optional<size_t>(val);  // Maybe optimize this
}
