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
#include <cstdint>  // for uint32_t
#include <sstream>  // for istringstream
#include <string>   // for string
#include <vector>   // for vector

#include "util/safe_casts.h"  // for as_signed

#include "InputStreamException.h"

class ObjectInputStream {
public:
    ObjectInputStream() = default;
    virtual ~ObjectInputStream() = default;

public:
    bool read(const char* data, size_t len);
    /// @param length of the stream
    bool read(std::stringstream istream, size_t length);

    void readObject(const char* name);
    std::string readObject();
    std::string getNextObjectName();
    void endObject();

    int readInt();
    uint32_t readUInt();
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
    std::stringstream istream;
    size_t pos();
    size_t len = 0;
};

extern template size_t ObjectInputStream::readType<size_t>();

template <typename T>
void ObjectInputStream::readData(std::vector<T>& data) {
    checkType('b');

    if (istream.str().size() < 2 * sizeof(size_t)) {
        throw InputStreamException("End reached, but try to read data len and width", __FILE__, __LINE__);
    }

    size_t len = readType<size_t>();
    size_t width = readType<size_t>();

    if (width != sizeof(T)) {
        throw InputStreamException("Data width mismatch requested type width", __FILE__, __LINE__);
    }

    if (istream.str().size() < len * width) {
        throw InputStreamException("End reached, but try to read data", __FILE__, __LINE__);
    }

    if (len) {
        data.resize(len);
        istream.read((char*)data.data(), as_signed(len * width));
    }
}
