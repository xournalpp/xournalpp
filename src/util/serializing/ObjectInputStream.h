/*
 * Xournal++
 *
 * Serialized input stream
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <sstream>

#include <gtk/gtk.h>

#include "InputStreamException.h"

class Serializeable;

class ObjectInputStream {
public:
    ObjectInputStream();
    virtual ~ObjectInputStream();

public:
    bool read(const char* data, int len);

    void readObject(const char* name);
    string readObject();
    string getNextObjectName();
    void endObject();

    int readInt();
    double readDouble();
    size_t readSizeT();
    string readString();

    void readData(void** data, int* len);
    cairo_surface_t* readImage();

private:
    void checkType(char type);

    static string getType(char type);

private:
    std::istringstream istream;
    size_t pos();
    size_t len = 0;
};
