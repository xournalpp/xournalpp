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

#include <string>
#include <vector>

#include <gtk/gtk.h>


class ObjectEncoding;
class Serializeable;

class ObjectOutputStream {
public:
    ObjectOutputStream(ObjectEncoding* encoder);
    virtual ~ObjectOutputStream();

public:
    void writeObject(const char* name);
    void endObject();

    void write(int i);
    void write(double d);
    void write(size_t st);
    void write(const char* str);
    void write(const std::string& s);

    void write(const void* data, int len, int width);
    void write(cairo_surface_t* img);

    GString* getStr();

private:
    ObjectEncoding* encoder = nullptr;
};
