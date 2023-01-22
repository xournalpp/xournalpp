#include "control/SearchPath.h"

#include <utility>  // for move

#include "util/PathUtil.h"
#include "util/Stacktrace.h"  // for Stacktrace

SearchPath::SearchPath(std::vector<fs::path> searchPaths): m_paths{std::move(searchPaths)} {}

auto SearchPath::findFile(const fs::path& relativePath) -> fs::path {
    for (auto& path: m_paths) {
        auto p = path / relativePath;
        if (fs::exists(p)) {
            return p;
        }
    }

    return fs::path{};
};
