#include "GladeSearchpath.h"

GladeSearchpath::GladeSearchpath() = default;

GladeSearchpath::~GladeSearchpath() { directories.clear(); }

auto GladeSearchpath::findFile(fs::path const& subdir, fs::path const& file) -> fs::path {
    fs::path filepath;
    if (subdir.empty()) {
        filepath = file;
    } else {
        filepath = subdir / file;
    }

    // We step through each directory to find it.
    for (const auto& dir: directories) {
        auto pathname = dir / G_DIR_SEPARATOR_S / file;

        if (g_file_test(pathname.string().c_str(), G_FILE_TEST_EXISTS)) {
            return pathname;
        }
    }

    return fs::path{};
}

/**
 * @return The first search path
 */
auto GladeSearchpath::getFirstSearchPath() const -> fs::path {
    if (this->directories.empty()) {
        return {};
    }

    return this->directories[0];
}

/**
 * Use this function to set the directory containing installed pixmaps and Glade XML files.
 */
void GladeSearchpath::addSearchDirectory(fs::path const& directory) { this->directories.push_back(directory); }
