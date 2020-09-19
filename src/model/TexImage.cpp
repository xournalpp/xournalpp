#include "TexImage.h"

#include <utility>

#include "serializing/ObjectInputStream.h"
#include "serializing/ObjectOutputStream.h"

#include "pixbuf-utils.h"

TexImage::TexImage(): Element(ELEMENT_TEXIMAGE) { this->sizeCalculated = true; }

TexImage::~TexImage() { freeImageAndPdf(); }

void TexImage::freeImageAndPdf() {
    if (this->image) {
        cairo_surface_destroy(this->image);
        this->image = nullptr;
    }

    if (this->pdf) {
        g_object_unref(this->pdf);
        this->pdf = nullptr;
    }
}

auto TexImage::clone() -> Element* {
    auto* img = new TexImage();
    img->loadData(std::string(this->binaryData), nullptr);
    img->x = this->x;
    img->y = this->y;
    img->setColor(this->getColor());
    img->width = this->width;
    img->height = this->height;
    img->text = this->text;
    return img;
}

void TexImage::setWidth(double width) {
    this->width = width;
    this->calcSize();
}

void TexImage::setHeight(double height) {
    this->height = height;
    this->calcSize();
}

auto TexImage::cairoReadFunction(TexImage* image, unsigned char* data, unsigned int length) -> cairo_status_t {
    for (unsigned int i = 0; i < length; i++, image->read++) {
        if (image->read >= image->binaryData.length()) {
            return CAIRO_STATUS_READ_ERROR;
        }
        data[i] = image->binaryData[image->read];
    }

    return CAIRO_STATUS_SUCCESS;
}

/**
 * Gets the binary data, a .PNG image or a .PDF
 */
auto TexImage::getBinaryData() const -> std::string const& { return this->binaryData; }

void TexImage::setText(string text) { this->text = std::move(text); }

auto TexImage::getText() -> string { return this->text; }

auto TexImage::loadData(std::string&& bytes, GError** err) -> bool {
    this->freeImageAndPdf();
    this->binaryData = bytes;
    if (this->binaryData.length() < 4) {
        return false;
    }

    const std::string type = binaryData.substr(1, 3);
    if (type == "PDF") {
        // Note: binaryData must not be modified while pdf is live.
        this->pdf = poppler_document_new_from_data(this->binaryData.data(), this->binaryData.size(), nullptr, err);
        if (!pdf || poppler_document_get_n_pages(this->pdf) < 1) {
            return false;
        }
        if (!this->width && !this->height) {
            PopplerPage* page = poppler_document_get_page(this->pdf, 0);
            poppler_page_get_size(page, &this->width, &this->height);
        }
    } else if (type == "PNG") {
        this->image = cairo_image_surface_create_from_png_stream(
                reinterpret_cast<cairo_read_func_t>(&cairoReadFunction), this);
    } else {
        g_warning("Unknown Latex image type: «%s»", type.c_str());
    }

    return true;
}

auto TexImage::getImage() -> cairo_surface_t* { return this->image; }

auto TexImage::getPdf() -> PopplerDocument* { return this->pdf; }

void TexImage::scale(double x0, double y0, double fx, double fy, double rotation,
                     bool) {  // line width scaling option is not used

    this->x = (this->x - x0) * fx + x0;
    this->y = (this->y - y0) * fy + y0;

    this->width *= fx;
    this->height *= fy;
    this->calcSize();
}

void TexImage::rotate(double x0, double y0, double th) {
    // Rotation for TexImages not yet implemented
}

void TexImage::serialize(ObjectOutputStream& out) {
    out.writeObject("TexImage");

    serializeElement(out);

    out.writeDouble(this->width);
    out.writeDouble(this->height);
    out.writeString(this->text);

    out.writeData(this->binaryData.c_str(), this->binaryData.length(), 1);

    out.endObject();
}

void TexImage::readSerialized(ObjectInputStream& in) {
    in.readObject("TexImage");

    readSerializedElement(in);

    this->width = in.readDouble();
    this->height = in.readDouble();
    this->text = in.readString();

    freeImageAndPdf();

    char* data = nullptr;
    int len = 0;
    in.readData(reinterpret_cast<void**>(&data), &len);

    this->loadData(std::string(data, len), nullptr);

    in.endObject();
    this->calcSize();
}

void TexImage::calcSize() const {
    this->snappedBounds = Rectangle<double>(this->x, this->y, this->width, this->height);
    this->sizeCalculated = true;
}
