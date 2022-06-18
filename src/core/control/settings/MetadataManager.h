/*
 * Xournal++
 *
 * Last opened files with all settings
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <mutex>   // for mutex
#include <vector>  // for vector

#include <glib.h>  // for gint64

#include "filesystem.h"  // for path


class MetadataEntry {
public:
    MetadataEntry();

public:
    fs::path metadataFile;
    bool valid;
    fs::path path;
    double zoom;
    int page;
    gint64 time;
};

class MetadataManager {
public:
    MetadataManager();
    virtual ~MetadataManager();

public:
    /**
     * Get the metadata for a file
     */
    static MetadataEntry getForFile(fs::path const& file);

    /**
     * Store the current data into metadata
     */
    void storeMetadata(fs::path const& file, int page, double zoom);

    /**
     * Document was closed, a new document was opened etc.
     */
    void documentChanged();

private:
    /**
     * Delete an old metadata file
     */
    static void deleteMetadataFile(fs::path const& path);

    /**
     * Parse a single metadata file
     */
    static MetadataEntry loadMetadataFile(fs::path const& path, fs::path const& file);

    /**
     * Store metadata to file
     */
    static void storeMetadata(MetadataEntry* m);

private:
    /**
     * Load the metadata list (sorted)
     */
    static std::vector<MetadataEntry> loadList();


private:
    std::mutex mutex;
    MetadataEntry* metadata;
};
