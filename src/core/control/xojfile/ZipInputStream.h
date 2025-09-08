/*
 * Xournal++
 *
 * Input stream for a file inside a zip archive
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <zip.h>

#include "control/xojfile/InputStream.h"  // for InputStream

#include "filesystem.h"  // for path


class ZipInputStream final: public InputStream {
public:
    ZipInputStream();
    /**
     * Create an input stream object by calling `open()`
     */
    ZipInputStream(zip_t* archive, const fs::path& filepath);
    ~ZipInputStream() override;

    int read(char* buffer, unsigned int len) noexcept override;
    /**
     * Open a file inside a zip archive for reading
     * @param  archive  A pointer to an already opened zip archive. The archive
     *                  is not managed by this object and must remain open until
     *                  the file is closed.
     * @param  filepath The path to the file inside the archive
     * @throws std::runtime_error if the file can not be opened
     */
    void open(zip_t* archive, const fs::path& filepath);
    void close() override;

private:
    zip_file_t* file;
};
