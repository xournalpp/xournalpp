/*
 * Xournal++
 *
 * A layout manager - map where( row,column) to which page( document index)
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <cstddef>   // for size_t
#include <cstdint>   // for uint32_t
#include <optional>  // for optional
#include <tuple>     // for operator==, tie, tuple
#include <vector>    // for vector

class Settings;

struct GridPosition {
    size_t col{};
    size_t row{};

    constexpr friend bool operator==(GridPosition const& lhs, GridPosition const& rhs) noexcept {
        return lhs.col == rhs.col && lhs.row == rhs.row;
    }
};

struct LayoutSettings {
    /// @brief The Layout of the pages
    enum Orientation : bool {
        Horizontal = false,
        Vertical = true,
    };

    /// Horizontal read direction
    enum HorizontalDirection : bool {
        LeftToRight = false,
        RightToLeft = true,
    };

    /// Vertical read direction
    enum VerticalDirection : bool {
        TopToBottom = false,
        BottomToTop = true,
    };

    size_t cols = 0;
    size_t rows = 0;
    size_t actualPages = 0;
    uint32_t offset = 0;

    bool showPairedPages = false;
    Orientation orientation = Vertical;
    HorizontalDirection horizontalDir = LeftToRight;
    VerticalDirection verticalDir = TopToBottom;

    constexpr friend auto operator==(LayoutSettings const& lhs, LayoutSettings const& rhs) {
        auto tier = [](auto&& other) {
            return std::tie(other.cols, other.rows, other.actualPages, other.offset, other.showPairedPages,
                            other.orientation, other.horizontalDir, other.verticalDir);
        };
        return tier(lhs) == tier(rhs);
    }
};
auto layoutSettings(Settings const*) -> LayoutSettings;

/**
 * @brief Layout asks this mapper what page ( if any ) should be at a given column,row.
 */
class LayoutMapper {
public:
    LayoutMapper() = default;

    /**
     * configureFromSettings
     * Obtain user settings to determine arguments to configure().
     *
     * @param  pages  The number of pages in the document
     * @param  settings  The Settings from which users settings are obtained
     */

    void configureFromSettings(size_t numPages, Settings* settings);

    auto at(size_t) const -> GridPosition;
    auto at(GridPosition const&) const -> std::optional<size_t>;

    size_t getColumns() const;
    size_t getRows() const;

    bool isPairedPages() const;

private:
    void precalculateMappers();

    /**
     * Map page location to document index
     *
     * @param  x Row we are interested in
     * @param  y Column we are interested in
     *
     * @return Page index to put at coordinates
     */

public:
    LayoutSettings data_;

    friend void calculate(LayoutSettings& data, size_t numRows, size_t numCols, bool useRows, uint32_t firstPageOffset);

    std::vector<GridPosition> pageToRaster;
    std::vector<size_t> rasterToPage;  // access with x*y_max+y+
};
