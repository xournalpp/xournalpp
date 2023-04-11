#include "util/serializing/ObjectInputStream.h"

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
    checkType('{');
    return readString();
}

auto ObjectInputStream::getNextObjectName() -> std::string {
    auto position = istream.tellg();

    checkType('{');
    std::string name = readString();

    istream.seekg(position);
    return name;
}

void ObjectInputStream::endObject() { checkType('}'); }

auto ObjectInputStream::readInt() -> int {
    checkType('i');
    return readTypeFromSStream<int>(istream);
}

auto ObjectInputStream::readDouble() -> double {
    checkType('d');
    return readTypeFromSStream<double>(istream);
}

auto ObjectInputStream::readSizeT() -> size_t {
    checkType('l');
    return readTypeFromSStream<size_t>(istream);
}

auto ObjectInputStream::readString() -> std::string {
    checkType('s');

    size_t lenString = (size_t)readTypeFromSStream<int>(istream);

    if (istream.str().size() < len) {
        throw InputStreamException("End reached, but try to read an string", __FILE__, __LINE__);
    }

    std::string output;
    output.resize(lenString);

    istream.read(&output[0], (long)lenString);

    return output;
}

auto ObjectInputStream::readImage() -> const std::vector<std::byte> {
    checkType('m');

    if (istream.str().size() < sizeof(size_t)) {
        throw InputStreamException("End reached, but try to read an image's data's length", __FILE__, __LINE__);
    }

    const size_t len = readTypeFromSStream<size_t>(istream);
    if (istream.str().size() < len) {
        throw InputStreamException("End reached, but try to read an image", __FILE__, __LINE__);
    }
    std::vector<std::byte> data;
    data.resize(len);
    istream.read(reinterpret_cast<char*>(data.data()), static_cast<int>(len));

    return data;
}

void ObjectInputStream::checkType(char type) {
    if (istream.str().size() < 2) {
        throw InputStreamException(FS(FORMAT_STR("End reached, but try to read {1}, index {2} of {3}") % getType(type) %
                                      (uint32_t)pos() % (uint32_t)len),
                                   __FILE__, __LINE__);
    }
    char t = 0, underscore = 0;
    istream >> underscore >> t;

    if (underscore != '_') {
        throw InputStreamException(FS(FORMAT_STR("Expected type signature of {1}, index {2} of {3}, but read '{4}'") %
                                      getType(type) % ((uint32_t)pos() + 1) % (uint32_t)len % underscore),
                                   __FILE__, __LINE__);
    }

    if (t != type) {
        throw InputStreamException(FS(FORMAT_STR("Expected {1} but read {2}") % getType(type) % getType(t)), __FILE__,
                                   __LINE__);
    }
}

auto ObjectInputStream::getType(char type) -> std::string {
    std::string ret;
    if (type == '{') {
        ret = "Object begin";
    } else if (type == '}') {
        ret = "Object end";
    } else if (type == 'i') {
        ret = "Number";
    } else if (type == 'd') {
        ret = "Floating point";
    } else if (type == 's') {
        ret = "String";
    } else if (type == 'b') {
        ret = "Binary";
    } else if (type == 'm') {
        ret = "Image";
    } else {
        char* str = g_strdup_printf("Unknown type: %02hhx (%c)", type, type);
        ret = str;
        g_free(str);
    }

    return ret;
}
