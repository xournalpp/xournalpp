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

    void readData(void** data, int* len);
    template <typename T>
    void readData(std::vector<T>& data);

    /// Reads raw image data from the stream.
    const std::vector<std::byte> readImage();

private:
    void checkType(char type);

    static std::string getType(char type);

private:
    std::istringstream istream;
    size_t pos();
    size_t len = 0;
};

// This function requires that T is read from its binary representation to work (e.g. integer type)
template <typename T>
T readTypeFromSStream(std::istringstream& istream) {
    if (istream.str().size() < sizeof(T)) {
        std::ostringstream oss;
        oss << "End reached: trying to read " << sizeof(T) << " bytes while only " << istream.str().size()
            << " bytes available";
        throw InputStreamException(oss.str(), __FILE__, __LINE__);
    }
    T output;

    istream.read((char*)&output, sizeof(T));

    return output;
}

template <typename T>
void ObjectInputStream::readData(std::vector<T>& data) {
    checkType('b');

    if (istream.str().size() < 2 * sizeof(int)) {
        throw InputStreamException("End reached, but try to read data len and width", __FILE__, __LINE__);
    }

    int len = readTypeFromSStream<int>(istream);
    int width = readTypeFromSStream<int>(istream);

    if (width != sizeof(T)) {
        throw InputStreamException("Data width mismatch requested type width", __FILE__, __LINE__);
    }

    if (istream.str().size() < static_cast<size_t>(len * width)) {
        throw InputStreamException("End reached, but try to read data", __FILE__, __LINE__);
    }

    if (len) {
        data.resize(len);
        istream.read((char*)data.data(), len * width);
    }
}
