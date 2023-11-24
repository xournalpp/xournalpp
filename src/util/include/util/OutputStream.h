/*
 * Xournal++
 *
 * Output streams for writing
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>  // for string

#include <zlib.h>  // for gzFile

#include "filesystem.h"  // for path

class OutputStream {
public:
    OutputStream();
    virtual ~OutputStream();

public:
    void write(const char* str);
    void write(const std::string& str);

    virtual void write(const char* data, size_t len) = 0;
    virtual void close() = 0;
};

class GzOutputStream: public OutputStream {
public:
    GzOutputStream(fs::path file);
    ~GzOutputStream() override;

public:
    void write(const char* data, size_t len) override;

    void close() override;

    const std::string& getLastError() const;

private:
    gzFile fp = nullptr;

    std::string error;
    fs::path file;
};
