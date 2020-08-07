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

#include <string>
#include <vector>

#include "XournalType.h"
#include "filesystem.h"

class GladeSearchpath {
public:
    GladeSearchpath();
    virtual ~GladeSearchpath();

public:
    void addSearchDirectory(fs::path const& directory);

    /**
     * Searches for a path, return the path, an empty string if not found
     */
    fs::path findFile(fs::path const& subdir, fs::path const& file);

    /**
     * @return The first search path
     */
    fs::path getFirstSearchPath() const;

private:
    /**
     * Search directory for icons and Glade files
     */
    vector<fs::path> directories;
};
