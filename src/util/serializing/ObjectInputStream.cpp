#include "ObjectInputStream.h"

#include "Serializeable.h"
#include "i18n.h"

template <typename T>
T readTypeFromSStream(std::istringstream& istream, std::string typeName) {
    if (istream.str().size() < sizeof(T)) {
        throw InputStreamException("End reached, but try to read a" + typeName, __FILE__, __LINE__);
    }
    T output;

    istream.read((char*)&output, sizeof(T));

    return output;
}


ObjectInputStream::ObjectInputStream() = default;

ObjectInputStream::~ObjectInputStream() {}

size_t ObjectInputStream::pos() { return len - istream.str().size(); }

auto ObjectInputStream::read(const char* data, int data_len) -> bool {
    istream.clear();
    len = (size_t)data_len;
    string dataStr = string(data, len);
    istream.str(dataStr);

    try {
        string version = readString();
        if (version != XML_VERSION_STR) {
            g_warning("ObjectInputStream version mismatch... two different Xournal versions running? (%s / %s)",
                      version.c_str(), XML_VERSION_STR);
            return false;
        }
    } catch (InputStreamException& e) {
        g_warning("InputStreamException: %s", e.what());
        return false;
    }
    return true;
}

void ObjectInputStream::readObject(const char* name) {
    string type = readObject();
    if (type != name) {
        throw InputStreamException(FS(FORMAT_STR("Try to read object type {1} but read object type {2}") % name % type),
                                   __FILE__, __LINE__);
    }
}

auto ObjectInputStream::readObject() -> string {
    checkType('{');
    return readString();
}

auto ObjectInputStream::getNextObjectName() -> string {
    checkType('{');
    string name = readString();
    return name;
}

void ObjectInputStream::endObject() { checkType('}'); }

auto ObjectInputStream::readInt() -> int {
    checkType('i');
    return readTypeFromSStream<int>(istream, "int");
}

auto ObjectInputStream::readDouble() -> double {
    checkType('d');
    return readTypeFromSStream<double>(istream, "double");
}

auto ObjectInputStream::readSizeT() -> size_t {
    checkType('l');
    return readTypeFromSStream<size_t>(istream, "size_t");
}

auto ObjectInputStream::readString() -> string {
    checkType('s');

    size_t lenString = (size_t)readTypeFromSStream<int>(istream, "int");

    if (istream.str().size() < len) {
        throw InputStreamException("End reached, but try to read an string", __FILE__, __LINE__);
    }

    string output;
    output.resize(lenString);

    istream.read(&output[0], (long)lenString);

    return output;
}

void ObjectInputStream::readData(void** data, int* length) {
    checkType('b');

    if (istream.str().size() < 2 * sizeof(int)) {
        throw InputStreamException("End reached, but try to read data len and width", __FILE__, __LINE__);
    }

    int len = readTypeFromSStream<int>(istream, "int");
    int width = readTypeFromSStream<int>(istream, "int");

    if (istream.str().size() < (len * width)) {
        throw InputStreamException("End reached, but try to read data", __FILE__, __LINE__);
    }

    if (len == 0) {
        *length = 0;
        *data = nullptr;
    } else {
        *data = (void*)new char[(size_t)(len * width)];
        *length = len;

        istream.read((char*)*data, len * width);
    }
}

cairo_status_t cairoReadFunction(std::istringstream* iss, unsigned char* data, unsigned int length) {
    if (iss->str().size() < length) {
        return CAIRO_STATUS_READ_ERROR;
    }
    iss->read((char*)data, length);
    return CAIRO_STATUS_SUCCESS;
}

auto ObjectInputStream::readImage() -> cairo_surface_t* {
    checkType('m');

    if (istream.str().size() < sizeof(size_t)) {
        throw InputStreamException("End reached, but try to read an image's data's length", __FILE__, __LINE__);
    }

    size_t len = readTypeFromSStream<size_t>(istream, "gsize");

    if (istream.str().size() < len) {
        throw InputStreamException("End reached, but try to read an image", __FILE__, __LINE__);
    }

    return cairo_image_surface_create_from_png_stream((cairo_read_func_t)(&cairoReadFunction), &istream);
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

auto ObjectInputStream::getType(char type) -> string {
    string ret;
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
        char* str = g_strdup_printf("Unknown type: %02x (%c)", type, type);
        ret = str;
        g_free(str);
    }

    return ret;
}
