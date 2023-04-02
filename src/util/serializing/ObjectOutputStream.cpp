#include "util/serializing/ObjectOutputStream.h"

#include <cassert>

#include <cairo.h>  // for CAIRO_STATUS_SUCCESS

#include "util/serializing/Serializable.h"  // for XML_VERSION_STR

ObjectOutputStream::ObjectOutputStream(ObjectEncoding* encoder) {
    assert(encoder != nullptr);
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
    int len = static_cast<int>(s.length());
    this->encoder->addData(&len, sizeof(int));
    this->encoder->addData(s.c_str(), len);
}

void ObjectOutputStream::writeImage(const std::vector<std::byte>& imgData) {
    this->encoder->addStr("_m");
    size_t len = imgData.size();
    this->encoder->addData(&len, sizeof(size_t));
    this->encoder->addData(imgData.data(), static_cast<int>(len));
}

auto ObjectOutputStream::stealData() -> std::vector<std::byte> { return std::move(this->encoder->stealData()); }
