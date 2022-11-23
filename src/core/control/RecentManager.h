/*
 * Xournal++
 *
 * The recent opened files
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <gtk/gtk.h>  // for GtkRecentInfo

#include "util/TinyVector.h"
#include "util/raii/CLibrariesSPtr.h"

#include "filesystem.h"  // for path

class Control;

/**
 * @brief Handles the GtkMenu displaying the recent files
 */
namespace RecentManager {
constexpr auto const* MIME = "application/x-xoj";
constexpr auto const* MIME_PDF = "application/x-pdf";
constexpr auto const* GROUP = "xournal++";
constexpr int MAX_RECENT = 10;

/**
 * Adds a file to the underlying GtkRecentManager
 */
void addRecentFileFilename(const fs::path& filename);

/**
 * Removes a file from the underlying GtkRecentManager
 */
[[maybe_unused]] void removeRecentFileFilename(const fs::path& filename);

/**
 * Remove all supported files from the recent file list
 */
void clearRecentFiles();

class GtkRecentInfoHandler {
public:
    constexpr static auto ref = gtk_recent_info_ref;
    constexpr static auto unref = gtk_recent_info_unref;
    // Todo(cpp20): replace with std:identity()
    constexpr static auto adopt = [](GtkRecentInfo* p) { return p; };
};
using GtkRecentInfoSPtr = xoj::util::CLibrariesSPtr<GtkRecentInfo, GtkRecentInfoHandler>;

/**
 * Returns the most recent xoj item from the underlying GtkRecentManager
 * or nullptr, if no recent files exist
 */
GtkRecentInfoSPtr getMostRecent();

/**
 *
 */
struct RecentFiles {
    TinyVector<GtkRecentInfoSPtr, MAX_RECENT> recentXoppFiles;
    TinyVector<GtkRecentInfoSPtr, MAX_RECENT> recentPdfFiles;
};
RecentFiles getRecentFiles();
};  // namespace RecentManager
