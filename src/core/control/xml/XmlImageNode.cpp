#include "XmlImageNode.h"

#include <utility>  // for move

#include "control/xml/XmlBase64Writer.h"  // for XmlBase64Writer
#include "control/xml/XmlNode.h"          // for XmlNode
#include "util/OutputStream.h"            // for OutputStream
#include "util/StringUtils.h"             // for StaticStringView

XmlImageNode::XmlImageNode(StringUtils::StaticStringView tag): XmlNode(tag) {}

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

void XmlImageNode::setImageData(std::string data) { this->imageData = std::move(data); }

void XmlImageNode::writeOut(OutputStream* out) {
    out->write("<");
    out->write(tag);
    writeAttributes(out);

    out->write(">");

    XmlBase64Writer b64(out);
    if (!this->imageData.empty()) {
        b64.write(reinterpret_cast<const unsigned char*>(this->imageData.data()), this->imageData.size());
    } else if (this->img != nullptr) {
        cairo_surface_write_to_png_stream(
                this->img,
                [](void* closure, const unsigned char* data, unsigned int length) -> cairo_status_t {
                    static_cast<XmlBase64Writer*>(closure)->write(data, length);
                    return CAIRO_STATUS_SUCCESS;
                },
                &b64);
    } else {
        g_error("XmlImageNode::writeOut(); neither an image surface nor image data was set");
    }
    b64.finish();

    out->write("</");
    out->write(tag);
    out->write(">\n");
}
