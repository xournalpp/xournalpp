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

#include <cstddef>  // for size_t
#include <sstream>  // for istringstream
#include <string>   // for string
#include <vector>   // for vector

#include "InputStreamException.h"

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

    template <typename T>
    void readData(std::vector<T>& data);

    /// Reads raw image data from the stream.
    std::string readImage();

private:
    void checkType(char type);

    static std::string getType(char type);

    template <class T>
    T readType();

private:
    std::istringstream istream;
    size_t pos();
    size_t len = 0;
};

extern template int ObjectInputStream::readType<int>();

template <typename T>
void ObjectInputStream::readData(std::vector<T>& data) {
    checkType('b');

    if (istream.str().size() < 2 * sizeof(int)) {
        throw InputStreamException("End reached, but try to read data len and width", __FILE__, __LINE__);
    }

    int len = readType<int>();
    int width = readType<int>();

    if (width != sizeof(T)) {
        throw InputStreamException("Data width mismatch requested type width", __FILE__, __LINE__);
    }

    if (len < 0) {
        throw InputStreamException("Negative length of data array", __FILE__, __LINE__);
    }

    if (istream.str().size() < static_cast<size_t>(len * width)) {
        throw InputStreamException("End reached, but try to read data", __FILE__, __LINE__);
    }

    if (len) {
        data.resize(static_cast<size_t>(len));
        istream.read((char*)data.data(), len * width);
    }
}
