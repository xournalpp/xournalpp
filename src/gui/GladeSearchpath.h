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

class GladeSearchpath {
public:
    GladeSearchpath();
    virtual ~GladeSearchpath();

public:
    void addSearchDirectory(const string& directory);

    /**
     * Searches for a path, return the path, an empty string if not found
     */
    string findFile(const string& subdir, const string& file);

    /**
     * @return The first search path
     */
    string getFirstSearchPath();

private:
    /**
     * Search directory for icons and Glade files
     */
    vector<string> directories;
};
