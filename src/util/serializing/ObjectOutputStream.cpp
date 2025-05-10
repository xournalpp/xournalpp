#include "util/serializing/ObjectOutputStream.h"

#include <cairo.h>  // for CAIRO_STATUS_SUCCESS

#include "util/Assert.h"                      // for xoj_assert
#include "util/serializing/ObjectEncoding.h"  // for ObjectEncoding
#include "util/serializing/Serializable.h"    // for XML_VERSION_STR

ObjectOutputStream::ObjectOutputStream(ObjectEncoding* encoder) {
    xoj_assert(encoder != nullptr);
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

void ObjectOutputStream::writeUInt(uint32_t u) {
    this->encoder->addStr("_u");
    this->encoder->addData(&u, sizeof(uint32_t));
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
    size_t len = s.length();
    this->encoder->addData(&len, sizeof(size_t));
    this->encoder->addData(s.c_str(), len);
}

void ObjectOutputStream::writeData(const void* data, size_t len, size_t width) {
    this->encoder->addStr("_b");
    this->encoder->addData(&len, sizeof(size_t));

    // size of one element
    this->encoder->addData(&width, sizeof(size_t));
    if (data != nullptr) {
        this->encoder->addData(data, len * width);
    }
}

void ObjectOutputStream::writeImage(const std::string_view& imgData) {
    this->encoder->addStr("_m");
    size_t len = imgData.length();
    this->encoder->addData(&len, sizeof(size_t));
    this->encoder->addData(imgData.data(), len);
}

auto ObjectOutputStream::stealData() -> GString* { return this->encoder->stealData(); }
