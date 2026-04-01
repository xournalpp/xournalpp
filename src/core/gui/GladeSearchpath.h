/*
 * Xournal++
 *
 * Search directory for icons and Glade files
 *
 * @author andreas
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <vector>  // for vector

#include "filesystem.h"  // for path

class GladeSearchpath {
public:
    GladeSearchpath();
    virtual ~GladeSearchpath();

public:
    void addSearchDirectory(fs::path const& directory);

    /**
     * Searches for a path, return the path, an empty string if not found
     */
    fs::path findFile(fs::path const& subdir, fs::path const& file) const;

    /**
     * @return The first search path
     */
    fs::path getFirstSearchPath() const;

private:
    /**
     * Search directory for icons and Glade files
     */
    std::vector<fs::path> directories;
};
