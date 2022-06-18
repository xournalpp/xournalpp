#include "XmlImageNode.h"

#include <glib.h>  // for g_base64_encode, g_free, gchar, g_e...

#include "control/xml/XmlNode.h"  // for XmlNode
#include "util/OutputStream.h"    // for OutputStream

XmlImageNode::XmlImageNode(const char* tag): XmlNode(tag) {
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
}

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

    if (this->img == nullptr) {
        g_error("XmlImageNode::writeOut(); this->img == nullptr");
    } else {
        this->out = out;
        this->pos = 0;
        cairo_surface_write_to_png_stream(this->img, reinterpret_cast<cairo_write_func_t>(&pngWriteFunction), this);
        gchar* base64_str = g_base64_encode(this->buffer, this->pos);
        out->write(base64_str);
        g_free(base64_str);

        this->out = nullptr;
    }

    out->write("</");
    out->write(tag);
    out->write(">\n");
}
