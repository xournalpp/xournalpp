/*
 * Xournal++
 *
 * Serialized output stream
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <cstddef>      // for size_t
#include <string>       // for string
#include <string_view>  // for string_view

#include <glib.h>  // for GString

class ObjectEncoding;

class ObjectOutputStream {
public:
    ObjectOutputStream(ObjectEncoding* encoder);
    virtual ~ObjectOutputStream();

public:
    void writeObject(const char* name);
    void endObject();

    void writeInt(int i);
    void writeDouble(double d);
    void writeSizeT(size_t st);
    void writeString(const char* str);
    void writeString(const std::string& s);

    void writeData(const void* data, int len, int width);

    /// Writes the raw image data to the output stream.
    void writeImage(const std::string_view& imgData);

    GString* getStr();

private:
    ObjectEncoding* encoder = nullptr;
};
