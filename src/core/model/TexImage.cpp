#include "TexImage.h"

#include <utility>  // for move

#include <poppler-document.h>  // for poppler_document_ge...
#include <poppler-page.h>      // for poppler_page_get_size

#include "model/Element.h"                        // for Element, ELEMENT_TE...
#include "util/Rectangle.h"                       // for Rectangle
#include "util/raii/GObjectSPtr.h"                // for GObjectSPtr
#include "util/serializing/ObjectInputStream.h"   // for ObjectInputStream
#include "util/serializing/ObjectOutputStream.h"  // for ObjectOutputStream

using xoj::util::Rectangle;

TexImage::TexImage(): Element(ELEMENT_TEXIMAGE) { this->sizeCalculated = true; }

TexImage::~TexImage() { freeImageAndPdf(); }

void TexImage::freeImageAndPdf() {
    if (this->image) {
        cairo_surface_destroy(this->image);
        this->image = nullptr;
    }

    this->pdf.reset();
}

auto TexImage::clone() const -> Element* {
    auto* img = new TexImage();
    img->x = this->x;
    img->y = this->y;
    img->setColor(this->getColor());
    img->width = this->width;
    img->height = this->height;
    img->text = this->text;
    img->snappedBounds = this->snappedBounds;
    img->sizeCalculated = this->sizeCalculated;

    // Clone has a copy of our PDF.
    img->pdf = this->pdf;

    // Load a copy of our data (must be called after
    // giving the clone a copy of our PDF -- it may change
    // the PDF we've given it).
    std::vector<std::byte> data = this->binaryData;
    img->loadData(std::move(data), nullptr);

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
        if (image->read >= image->binaryData.size()) {
            return CAIRO_STATUS_READ_ERROR;
        }
        data[i] = static_cast<unsigned char>(image->binaryData[image->read]);
    }

    return CAIRO_STATUS_SUCCESS;
}

/**
 * Gets the binary data, a .PNG image or a .PDF
 */
auto TexImage::getBinaryData() const -> std::vector<std::byte> const& { return this->binaryData; }

void TexImage::setText(std::string text) { this->text = std::move(text); }

auto TexImage::getText() const -> std::string { return this->text; }

auto TexImage::loadData(std::vector<std::byte>&& bytes, GError** err) -> bool {
    this->freeImageAndPdf();
    this->binaryData = std::move(bytes);
    if (this->binaryData.size() < 4) {
        return false;
    }

    const std::string type = std::string(reinterpret_cast<unsigned char*>(this->binaryData.data()) + 1,
                                         reinterpret_cast<unsigned char*>(this->binaryData.data()) + 4);
    if (type == "PDF") {
        // Note: binaryData must not be modified while pdf is live.
        this->pdf.reset(poppler_document_new_from_data(reinterpret_cast<char*>(this->binaryData.data()),
                                                       this->binaryData.size(), nullptr, err),
                        xoj::util::adopt);
        if (!pdf || poppler_document_get_n_pages(this->pdf.get()) < 1) {
            return false;
        }
        if (!this->width && !this->height) {
            xoj::util::GObjectSPtr<PopplerPage> page(poppler_document_get_page(this->pdf.get(), 0), xoj::util::adopt);
            poppler_page_get_size(page.get(), &this->width, &this->height);
        }
    } else if (type == "PNG") {
        this->image = cairo_image_surface_create_from_png_stream(
                reinterpret_cast<cairo_read_func_t>(&cairoReadFunction), this);
    } else {
        g_warning("Unknown Latex image type: \"%s\"", type.c_str());
    }

    return true;
}

auto TexImage::getImage() const -> cairo_surface_t* { return this->image; }

auto TexImage::getPdf() const -> PopplerDocument* { return this->pdf.get(); }

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

void TexImage::serialize(ObjectOutputStream& out) const {
    out.writeObject("TexImage");

    this->Element::serialize(out);

    out.writeDouble(this->width);
    out.writeDouble(this->height);
    out.writeString(this->text);

    out.writeData(this->binaryData);

    out.endObject();
}

void TexImage::readSerialized(ObjectInputStream& in) {
    in.readObject("TexImage");

    this->Element::readSerialized(in);

    this->width = in.readDouble();
    this->height = in.readDouble();
    this->text = in.readString();

    freeImageAndPdf();

    std::vector<std::byte> data;
    in.readData(data);
    this->loadData(std::move(data), nullptr);

    in.endObject();
    this->calcSize();
}

void TexImage::calcSize() const {
    this->snappedBounds = Rectangle<double>(this->x, this->y, this->width, this->height);
    this->sizeCalculated = true;
}
