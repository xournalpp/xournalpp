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

#include <string>
#include <vector>

#include <zlib.h>

#include "XournalType.h"
#include "filesystem.h"

class OutputStream {
public:
    OutputStream();
    virtual ~OutputStream();

public:
    virtual void write(const char* str);
    virtual void write(const char* data, int len) = 0;
    virtual void write(const string& str);

    virtual void close() = 0;
};

class GzOutputStream: public OutputStream {
public:
    GzOutputStream(fs::path file);
    virtual ~GzOutputStream();

public:
    virtual void write(const char* data, int len);

    virtual void close();

    string& getLastError();

private:
    gzFile fp = nullptr;

    string error;

    string target;
    fs::path file;
};
