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

class Serializable;

class ObjectInputStream {
public:
    ObjectInputStream() = default;
    virtual ~ObjectInputStream() = default;

public:
    bool read(const char* data, int len);

    void readObject(const char* name);
    std::string readObject();
    std::string getNextObjectName();
    void endObject();

    int readInt();
    double readDouble();
    size_t readSizeT();
    std::string readString();

    void readData(void** data, int* len);
    cairo_surface_t* readImage();
    
    struct AlignmentInt{ //referenced https://stackoverflow.com/questions/11983311/c-4-bytes-aligned-data & https://stackoverflow.com/questions/28727914/what-does-misaligned-address-error-mean#:~:text=%22Misaligned%20address%22%20usually%20means%20that,bit%20integer%20from%20address%200x1001).
        char c;
        __attribute__((__aligned__(4))) int32_t member;
    }
    struct AlignmentDouble{ //referenced https://stackoverflow.com/questions/11983311/c-4-bytes-aligned-data & https://stackoverflow.com/questions/28727914/what-does-misaligned-address-error-mean#:~:text=%22Misaligned%20address%22%20usually%20means%20that,bit%20integer%20from%20address%200x1001).
        char c;
        __attribute__((__aligned__(8))) double32_t member;
    }


private:
    void checkType(char type);

    static std::string getType(char type);

private:
    std::istringstream istream;
    size_t pos();
    size_t len = 0;
};
