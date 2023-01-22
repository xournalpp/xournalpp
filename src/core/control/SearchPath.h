/*
 * Xournal++
 *
 * Search path
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <iostream>  // for vector
#include <string>    // for string
#include <vector>    // for vector

#include "filesystem.h"

class SearchPath {
public:
    SearchPath(std::vector<fs::path> searchPaths);

    auto findFile(const fs::path& relativePath) const -> fs::path;
    auto getPaths() const -> std::vector<fs::path>;
private:
    std::vector<fs::path> m_paths;
};
