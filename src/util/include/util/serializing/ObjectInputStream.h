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
    std::string readImage();

private:
    void checkType(char type);

    static std::string getType(char type);

private:
    std::istringstream istream;
    size_t pos();
    size_t len = 0;
};

extern template void ObjectInputStream::readData(std::vector<double>& data);
