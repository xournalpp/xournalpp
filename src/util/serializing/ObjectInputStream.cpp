#include "util/serializing/ObjectInputStream.h"

#include "util/i18n.h"
#include "util/serializing/Serializable.h"

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
    } catch (InputStreamException& e) {
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

auto ObjectInputStream::readInt() -> int { //referenced https://stackoverflow.com/questions/11983311/c-4-bytes-aligned-data & https://stackoverflow.com/questions/28727914/what-does-misaligned-address-error-mean#:~:text=%22Misaligned%20address%22%20usually%20means%20that,bit%20integer%20from%20address%200x1001).
    checkType('i');
    std::istringstream obj = readTypeFromSStream<int>(istream);
    string strObj = strObj.str();
    string num =  "";
    for(int i = 0; i < obj.length(); i++){
        AlignmentInt number = strObj.at(i);
        num+=number;
    }
    int readValue = stoi(num);
    return readValue;
    
}

auto ObjectInputStream::readDouble() -> double { //referenced https://stackoverflow.com/questions/11983311/c-4-bytes-aligned-data & https://stackoverflow.com/questions/28727914/what-does-misaligned-address-error-mean#:~:text=%22Misaligned%20address%22%20usually%20means%20that,bit%20integer%20from%20address%200x1001).
    checkType('d');
    std::istringstream obj = readTypeFromSStream<double>(istream);
    string strObj = strObj.str();
    string num =  "";
    for(int i = 0; i < obj.length(); i++){
        AlignmentDouble number = strObj.at(i);
        num+=number;
    }
    double readValue = stod(num);
    return readValue;
}

auto ObjectInputStream::readSizeT() -> size_t { //referenced https://stackoverflow.com/questions/11983311/c-4-bytes-aligned-data & https://stackoverflow.com/questions/28727914/what-does-misaligned-address-error-mean#:~:text=%22Misaligned%20address%22%20usually%20means%20that,bit%20integer%20from%20address%200x1001).
    checkType('l');
    std::istringstream obj = readTypeFromSStream<size_t>(istream);
    string strObj = strObj.str();
    string num =  "";
    for(int i = 0; i < obj.length(); i++){
        char number = alignof(strObj.at(i));
        num+=number;
    }
    int readValue = (size_t)(num);
    return readValue;
    
}

auto ObjectInputStream::readString() -> std::string { //referenced https://stackoverflow.com/questions/11983311/c-4-bytes-aligned-data & https://stackoverflow.com/questions/28727914/what-does-misaligned-address-error-mean#:~:text=%22Misaligned%20address%22%20usually%20means%20that,bit%20integer%20from%20address%200x1001).

    checkType('s');

    size_t lenString = (size_t)readTypeFromSStream<int>(istream);

    if (istream.str().size() < len) {
        throw InputStreamException("End reached, but try to read an string", __FILE__, __LINE__);
    }

    std::string output;
    output.resize(lenString, 4);

    istream.read(&output[0], (long)lenString);

    return output;
}

void ObjectInputStream::readData(void** data, int* length) {
    checkType('b');

    if (istream.str().size() < 2 * sizeof(int)) {
        throw InputStreamException("End reached, but try to read data len and width", __FILE__, __LINE__);
    }

    int len = readTypeFromSStream<int>(istream);
    int width = readTypeFromSStream<int>(istream);

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

    size_t len = readTypeFromSStream<size_t>(istream);

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
