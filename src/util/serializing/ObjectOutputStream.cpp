#include "util/serializing/ObjectOutputStream.h"

#include <cairo.h>  // for CAIRO_STATUS_SUCCESS

#include "util/serializing/ObjectEncoding.h"  // for ObjectEncoding
#include "util/serializing/Serializable.h"    // for XML_VERSION_STR

ObjectOutputStream::ObjectOutputStream(ObjectEncoding* encoder) {
    g_assert(encoder != nullptr);
    this->encoder = encoder;

    writeString(XML_VERSION_STR);
}

ObjectOutputStream::~ObjectOutputStream() {
    delete this->encoder;
    this->encoder = nullptr;
}

void ObjectOutputStream::writeObject(const char* name) {
    this->encoder->addStr("_{");

    writeString(name);
}

void ObjectOutputStream::endObject() { this->encoder->addStr("_}"); }

void ObjectOutputStream::writeInt(int i) {
    this->encoder->addStr("_i");
    this->encoder->addData(&i, sizeof(int));
}

void ObjectOutputStream::writeDouble(double d) {
    this->encoder->addStr("_d");
    this->encoder->addData(&d, sizeof(double));
}

void ObjectOutputStream::writeSizeT(size_t st) {
    this->encoder->addStr("_l");
    this->encoder->addData(&st, sizeof(size_t));
}

void ObjectOutputStream::writeString(const char* str) { writeString(std::string(str)); }

void ObjectOutputStream::writeString(const std::string& s) {
    this->encoder->addStr("_s");
    int len = s.length();
    this->encoder->addData(&len, sizeof(int));
    this->encoder->addData(s.c_str(), len);
}

void ObjectOutputStream::writeData(const void* data, int len, int width) {
    this->encoder->addStr("_b");
    this->encoder->addData(&len, sizeof(int));

    // size of one element
    this->encoder->addData(&width, sizeof(int));
    if (data != nullptr) {
        this->encoder->addData(data, len * width);
    }
}

void ObjectOutputStream::writeImage(const std::string_view& imgData) {
    this->encoder->addStr("_m");
    size_t len = imgData.length();
    this->encoder->addData(&len, sizeof(size_t));
    this->encoder->addData(imgData.data(), static_cast<int>(len));
}

auto ObjectOutputStream::getStr() -> GString* { return this->encoder->getData(); }
