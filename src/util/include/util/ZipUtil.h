/*
 * Xournal++
 *
 * Zip Helper
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <zip.h>  // for zip_t

#include "filesystem.h"  // for path

class ZipUtil {
private:
    ZipUtil();
    virtual ~ZipUtil();

public:
    /**
     * Opens a zip file with proper Unicode support on Windows
     * @param path The filesystem path to the zip file
     * @param flags The flags to pass to zip_open (e.g., ZIP_RDONLY, ZIP_CREATE)
     * @param error Pointer to receive error code
     * @return The opened zip file pointer or nullptr on error
     */
    static zip_t* openPath(const fs::path& path, int flags, int* error);
};
