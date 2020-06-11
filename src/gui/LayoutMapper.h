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

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "control/settings/Settings.h"
#include "util/hashcombine.h"

#include "XournalType.h"

namespace std {
template <>
class hash<std::pair<size_t, size_t>> {
public:
    size_t operator()(std::pair<size_t, size_t> const& data) const {
        size_t seed = 0;
        boost_c::hash_combine(seed, data.first);
        boost_c::hash_combine(seed, data.second);
        return seed;
    }
};
}  // namespace std

/**
 * @brief Layout asks this mapper what page ( if any ) should be at a given column,row.
 */
class LayoutMapper {
    /**
     * @brief The Layout of the pages
     */
    enum Orientation : bool {
        Horizontal = false,
        Vertical = true,
    };

    /**
     * Horizontal read direction
     */
    enum HorizontalDirection : bool {
        LeftToRight = false,
        RightToLeft = true,
    };

    /**
     * Vertical read direction
     */
    enum VerticalDirection : bool {
        TopToBottom = false,
        BottomToTop = true,
    };

public:
    LayoutMapper() = default;
    ~LayoutMapper() = default;

    /**
     * configureFromSettings
     * Obtain user settings to determine arguments to configure().
     *
     * @param  pages  The number of pages in the document
     * @param  settings  The Settings from which users settings are obtained
     */

    void configureFromSettings(size_t numPages, Settings* settings);

    std::pair<size_t, size_t> at(size_t) const;
    std::optional<size_t> at(std::pair<size_t, size_t>) const;

    size_t getColumns() const;
    size_t getRows() const;
    int getFirstPageOffset() const;

    bool isPairedPages() const;
    bool isRightToLeft() const;
    bool isBottomToTop() const;
    bool isVertical() const;

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
    std::optional<size_t> map(size_t col, size_t row) const;

private:
    struct internal_data {
        size_t cols = 0;
        size_t rows = 0;
        size_t actualPages = 0;

        int offset = 0;

        bool showPairedPages = false;
        Orientation orientation = Vertical;
        HorizontalDirection horizontalDir = LeftToRight;
        VerticalDirection verticalDir = TopToBottom;

        bool operator==(internal_data const& other) {
            return std::tie(this->cols, this->rows, this->actualPages, this->offset, this->showPairedPages,
                            this->orientation, this->horizontalDir, this->verticalDir) ==
                   std::tie(other.cols, other.rows, other.actualPages, other.offset, other.showPairedPages,
                            other.orientation, other.horizontalDir, other.verticalDir);
        }
    } data_;

    std::vector<std::pair<size_t, size_t>> pageToRaster;
    std::unordered_map<std::pair<size_t, size_t>, size_t> rasterToPage;

    friend void calculate(LayoutMapper::internal_data& data, size_t numRows, size_t numCols, bool useRows,
                          int firstPageOffset);
    friend class std::hash<LayoutMapper::internal_data>;
};
