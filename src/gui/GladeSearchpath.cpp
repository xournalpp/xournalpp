#include "GladeSearchpath.h"

GladeSearchpath::GladeSearchpath() = default;

GladeSearchpath::~GladeSearchpath() { directories.clear(); }

auto GladeSearchpath::findFile(const string& subdir, const string& file) -> string {
    string filename;
    if (subdir.empty()) {
        filename = file;
    } else {
        filename = subdir + G_DIR_SEPARATOR_S + file;
    }

    // We step through each directory to find it.
    for (const string& dir: directories) {
        string pathname = dir + G_DIR_SEPARATOR_S + filename;

        if (g_file_test(pathname.c_str(), G_FILE_TEST_EXISTS)) {
            return pathname;
        }
    }

    return "";
}

/**
 * @return The first search path
 */
auto GladeSearchpath::getFirstSearchPath() -> string {
    if (this->directories.empty()) {
        return "";
    }

    return this->directories[0];
}

/**
 * Use this function to set the directory containing installed pixmaps and Glade XML files.
 */
void GladeSearchpath::addSearchDirectory(const string& directory) { this->directories.push_back(directory); }
