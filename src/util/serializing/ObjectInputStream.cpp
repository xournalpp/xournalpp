#include "util/serializing/ObjectInputStream.h"

#include <cstdint>  // for uint32_t

#include <glib.h>  // for g_free, g_strdup_...

#include "util/PlaceholderString.h"                 // for PlaceholderString
#include "util/i18n.h"                              // for FORMAT_STR, FS
#include "util/safe_casts.h"                        // for as_signed
#include "util/serializing/InputStreamException.h"  // for InputStreamException
#include "util/serializing/Serializable.h"          // for XML_VERSION_STR

static size_t getSize(std::stringstream& s) {
    auto pos = s.tellg();
    s.seekg(0, std::ios::end);
    auto size = s.tellg() - pos;
    s.seekg(pos);
    return as_unsigned(size);
}

template size_t ObjectInputStream::readType<size_t>();

// This function requires that T is read from its binary representation to work (e.g. integer type)
template <typename T>
T ObjectInputStream::readType() {
    if (getSize(istream) < sizeof(T)) {
        std::ostringstream oss;
        oss << "End reached: trying to read " << sizeof(T) << " bytes while only " << getSize(istream)
            << " bytes available";
        throw InputStreamException(oss.str(), __FILE__, __LINE__);
    }
    T output;

    istream.read((char*)&output, as_signed(sizeof(T)));

    return output;
}

size_t ObjectInputStream::pos() { return static_cast<size_t>(istream.tellg()); }

auto ObjectInputStream::read(const char* data, size_t data_len) -> bool {
    return read(std::stringstream(std::string(data, data_len)), data_len);
}

auto ObjectInputStream::read(std::stringstream stream, size_t length) -> bool {
    this->istream = std::move(stream);
    this->len = length;

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
    return readType<int>();
}

auto ObjectInputStream::readUInt() -> uint32_t {
    checkType('u');
    return readType<uint32_t>();
}

auto ObjectInputStream::readDouble() -> double {
    checkType('d');
    return readType<double>();
}

auto ObjectInputStream::readSizeT() -> size_t {
    checkType('l');
    return readType<size_t>();
}

auto ObjectInputStream::readString() -> std::string {
    checkType('s');

    size_t lenString = readType<size_t>();

    if (getSize(istream) < lenString) {
        throw InputStreamException("End reached, but try to read an string", __FILE__, __LINE__);
    }

    std::string output;
    output.resize(lenString);

    istream.read(&output[0], as_signed(lenString));

    return output;
}

auto ObjectInputStream::readImage() -> std::string {
    checkType('m');

    if (getSize(istream) < sizeof(size_t)) {
        throw InputStreamException("End reached, but try to read an image's data's length", __FILE__, __LINE__);
    }

    const size_t len = readType<size_t>();
    if (getSize(istream) < len) {
        throw InputStreamException("End reached, but try to read an image", __FILE__, __LINE__);
    }
    std::string data;
    data.resize(len);
    istream.read(data.data(), as_signed(len));

    return data;
}

void ObjectInputStream::checkType(char type) {
    if (getSize(istream) < 2) {
        throw InputStreamException(
                FS(FORMAT_STR("End reached, but try to read {1}, index {2} of {3}") % getType(type) % pos() % len),
                __FILE__, __LINE__);
    }
    char t = 0, underscore = 0;
    istream >> underscore >> t;

    if (underscore != '_') {
        throw InputStreamException(FS(FORMAT_STR("Expected type signature of {1}, index {2} of {3}, but read '{4}'") %
                                      getType(type) % (pos() + 1) % len % underscore),
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
    } else if (type == 'u') {
        ret = "Unsigned Number";
    } else if (type == 'd') {
        ret = "Floating point";
    } else if (type == 'l') {
        ret = "Size";
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
