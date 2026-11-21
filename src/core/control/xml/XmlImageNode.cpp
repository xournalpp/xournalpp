#include "XmlImageNode.h"

#include <algorithm>  // for min

#include <glib.h>  // for g_base64_encode, g_free, gchar, g_e...

#include "control/xml/XmlNode.h"  // for XmlNode
#include "util/OutputStream.h"    // for OutputStream
#include "util/StringUtils.h"     // for StaticStringView

namespace {

/// Write binary data as base64 in chunks whose size is a multiple of 3.
void writeBase64(OutputStream* out, std::string_view data) {
    static constexpr size_t CHUNK_SIZE = 3 * 4096;

    for (size_t i = 0; i < data.size(); i += CHUNK_SIZE) {
        const size_t length = std::min(CHUNK_SIZE, data.size() - i);
        gchar* base64 = g_base64_encode(reinterpret_cast<const guchar*>(data.data() + i), length);
        out->write(base64);
        g_free(base64);
    }
}

}  // namespace

XmlImageNode::XmlImageNode(StringUtils::StaticStringView tag): XmlNode(tag) {
    this->img = nullptr;
    this->out = nullptr;
    this->pos = 0;
}

XmlImageNode::~XmlImageNode() {
    if (this->img) {
        cairo_surface_destroy(this->img);
    }
}

void XmlImageNode::setImage(cairo_surface_t* img) {
    if (this->img) {
        cairo_surface_destroy(this->img);
    }
    this->img = cairo_surface_reference(img);
    this->imageData.clear();
}

void XmlImageNode::setImageData(std::string_view data) { this->imageData = data; }

auto XmlImageNode::pngWriteFunction(XmlImageNode* image, const unsigned char* data, unsigned int length)
        -> cairo_status_t {
    for (unsigned int i = 0; i < length; i++, image->pos++) {
        if (image->pos == 30) {
            gchar* base64_str = g_base64_encode(image->buffer, image->pos);
            image->out->write(base64_str);
            g_free(base64_str);
            image->pos = 0;
        }
        image->buffer[image->pos] = data[i];
    }

    return CAIRO_STATUS_SUCCESS;
}

void XmlImageNode::writeOut(OutputStream* out) {
    out->write("<");
    out->write(tag);
    writeAttributes(out);

    out->write(">");

    if (!this->imageData.empty()) {
        writeBase64(out, this->imageData);
    } else if (this->img != nullptr) {
        this->out = out;
        this->pos = 0;
        cairo_surface_write_to_png_stream(this->img, reinterpret_cast<cairo_write_func_t>(&pngWriteFunction), this);
        gchar* base64_str = g_base64_encode(this->buffer, this->pos);
        out->write(base64_str);
        g_free(base64_str);

        this->out = nullptr;
    } else {
        g_error("XmlImageNode::writeOut(); neither an image surface nor image data was set");
    }

    out->write("</");
    out->write(tag);
    out->write(">\n");
}
