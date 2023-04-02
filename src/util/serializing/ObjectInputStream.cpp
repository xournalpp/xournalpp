#include "util/serializing/ObjectInputStream.h"

#include <array>      // for array
#include <cinttypes>  // for uint32_t

#include <glib.h>  // for g_free, g_strdup_...

#include "util/PlaceholderString.h"                 // for PlaceholderString
#include "util/i18n.h"                              // for FORMAT_STR, FS
#include "util/serializing/InputStreamException.h"  // for InputStreamException
#include "util/serializing/Serializable.h"          // for XML_VERSION_STR

size_t ObjectInputStream::pos() { return istream.tellg(); }

auto ObjectInputStream::read(const char* data, int data_len) -> bool {
    istream.clear();
    len = (size_t)data_len;
    std::string dataStr = std::string(data, len);
    istream.str(dataStr);

    try {
        std::string version = readString();
        if (version != XML_VERSION_STR) {
            g_warning("ObjectInputStream version mismatch... two different Xournal versions running? (%s / %s)",
                      version.c_str(), XML_VERSION_STR);
            return false;
        }
    } catch (const InputStreamException& e) {
        g_warning("InputStreamException: %s", e.what());
        return false;
    }
    return true;
}

void ObjectInputStream::readObject(const char* name) {
    std::string type = readObject();
    if (type != name) {
        throw InputStreamException(FS(FORMAT_STR("Try to read object type {1} but read object type {2}") % name % type),
                                   __FILE__, __LINE__);
    }
}

auto ObjectInputStream::readObject() -> std::string {
    checkType(std::byte('{'));
    return readString();
}

auto ObjectInputStream::getNextObjectName() -> std::string {
    auto position = istream.tellg();

    checkType(std::byte('{'));
    std::string name = readString();

    istream.seekg(position);
    return name;
}

void ObjectInputStream::endObject() { checkType(std::byte('}')); }

auto ObjectInputStream::readInt() -> int {
    checkType(std::byte('i'));
    return readTypeFromSStream<int>(istream);
}

auto ObjectInputStream::readDouble() -> double {
    checkType(std::byte('d'));
    return readTypeFromSStream<double>(istream);
}

auto ObjectInputStream::readSizeT() -> size_t {
    checkType(std::byte('l'));
    return readTypeFromSStream<size_t>(istream);
}

auto ObjectInputStream::readString() -> std::string {
    checkType(std::byte('s'));

    size_t lenString = (size_t)readTypeFromSStream<int>(istream);

    std::string output;
    output.resize(lenString);

    istream.read(&output[0], (long)lenString);
    if (istream.fail()) {
        throw InputStreamException("End reached, but try to read an string", __FILE__, __LINE__);
    }


    return output;
}

auto ObjectInputStream::readImage() -> std::vector<std::byte> {
    checkType(std::byte('m'));


    const size_t len = readTypeFromSStream<size_t>(istream);
    if (istream.fail()) {
        throw InputStreamException("End reached, but try to read an image's data's length", __FILE__, __LINE__);
    }

    std::vector<std::byte> data;
    data.resize(len);
    istream.read(reinterpret_cast<char*>(data.data()), static_cast<int>(len));
    if (istream.fail()) {
        throw InputStreamException("End reached, but try to read an image", __FILE__, __LINE__);
    }

    return data;
}

void ObjectInputStream::checkType(const std::byte type) {
    std::array<std::byte, 2> typeField = {};
    istream.read(reinterpret_cast<char*>(typeField.data()), typeField.size());

    if (istream.fail()) {
        throw InputStreamException(FS(FORMAT_STR("End reached, but try to read {1}, index {2} of {3}") % getType(type) %
                                      (uint32_t)pos() % (uint32_t)len),
                                   __FILE__, __LINE__);
    }

    if (typeField[0] != std::byte('_')) {
        throw InputStreamException(
                FS(FORMAT_STR("Expected type signature of {1}, index {2} of {3}, but read '{4}'") % getType(type) %
                   ((uint32_t)pos() + 1) % (uint32_t)len % static_cast<unsigned char>(typeField[0])),
                __FILE__, __LINE__);
    }

    if (typeField[1] != std::byte(type)) {
        throw InputStreamException(FS(FORMAT_STR("Expected {1} but read {2}") % getType(type) % getType(typeField[1])),
                                   __FILE__, __LINE__);
    }
}

auto ObjectInputStream::getType(const std::byte type) -> std::string {
    std::string ret;
    if (type == std::byte('{')) {
        ret = "Object begin";
    } else if (type == std::byte('}')) {
        ret = "Object end";
    } else if (type == std::byte('i')) {
        ret = "Number";
    } else if (type == std::byte('d')) {
        ret = "Floating point";
    } else if (type == std::byte('s')) {
        ret = "String";
    } else if (type == std::byte('b')) {
        ret = "Binary";
    } else if (type == std::byte('m')) {
        ret = "Image";
    } else {
        char* str = g_strdup_printf("Unknown type: %02hhx (%c)", static_cast<unsigned char>(type),
                                    static_cast<unsigned char>(type));
        ret = str;
        g_free(str);
    }

    return ret;
}
