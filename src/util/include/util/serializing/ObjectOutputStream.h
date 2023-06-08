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
#include <vector>       // for vector

#include "util/serializing/ObjectEncoding.h"  // for ObjectEncoding

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

    template <typename T>
    void writeData(const std::vector<T>& data);

    /// Writes the raw image data to the output stream.
    void writeImage(const std::vector<std::byte>& imgData);

    std::vector<std::byte> const& getData();

    std::vector<std::byte> stealData();

private:
    ObjectEncoding* encoder = nullptr;
};

template <typename T>
void ObjectOutputStream::writeData(const std::vector<T>& data) {
    this->encoder->addStr("_b");
    const int len = static_cast<int>(data.size());
    const int width = sizeof(T);
    this->encoder->addData(&len, sizeof(int));
    this->encoder->addData(&width, sizeof(int));
    this->encoder->addData(data.data(), static_cast<int>(data.size() * sizeof(T)));
}
