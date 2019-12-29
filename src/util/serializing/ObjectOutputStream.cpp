#include "ObjectOutputStream.h"

#include "ObjectEncoding.h"
#include "Serializeable.h"

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

void ObjectOutputStream::writeString(const char* str) { writeString(string(str)); }

void ObjectOutputStream::writeString(const string& s) {
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

static auto cairoWriteFunction(GString* string, const unsigned char* data, unsigned int length) -> cairo_status_t {
    g_string_append_len(string, reinterpret_cast<const gchar*>(data), length);
    return CAIRO_STATUS_SUCCESS;
}

void ObjectOutputStream::writeImage(cairo_surface_t* img) {
    GString* imgStr = g_string_sized_new(102400);

    cairo_surface_write_to_png_stream(img, reinterpret_cast<cairo_write_func_t>(&cairoWriteFunction), imgStr);

    this->encoder->addStr("_m");
    this->encoder->addData(&imgStr->len, sizeof(gsize));

    this->encoder->addData(imgStr->str, imgStr->len);

    g_string_free(imgStr, true);
}

auto ObjectOutputStream::getStr() -> GString* { return this->encoder->getData(); }
