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
    ZipInputStream(zip_t* archive, const fs::path& filepath);
    ~ZipInputStream() override;

    int read(char* buffer, unsigned int len) noexcept override;
    void open(zip_t* archive, const fs::path& filepath);
    void close() override;

private:
    zip_file_t* file;
};
