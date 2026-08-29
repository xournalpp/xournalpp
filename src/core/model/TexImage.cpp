#include "TexImage.h"

#include <memory>
#include <utility>  // for move

#include <poppler-document.h>  // for poppler_document_ge...
#include <poppler-page.h>      // for poppler_page_get_size

#include "model/Element.h"                        // for Element, ELEMENT_TE...
#include "util/Rectangle.h"                       // for Rectangle
#include "util/raii/GObjectSPtr.h"                // for GObjectSPtr
#include "util/serializing/ObjectInputStream.h"   // for ObjectInputStream
#include "util/serializing/ObjectOutputStream.h"  // for ObjectOutputStream

TexImage::TexImage(): RectangularElement(ELEMENT_TEXIMAGE) { this->sizeCalculated = true; }

TexImage::~TexImage() { freeImageAndPdf(); }

void TexImage::freeImageAndPdf() {
    this->image.reset();
    this->pdf.reset();
}

auto TexImage::cloneTexImage() const -> std::unique_ptr<TexImage> {
    auto img = std::make_unique<TexImage>();
    static_cast<RectangularElement&>(*img) = *this;

    img->text = this->text;

    // Clone has a copy of our PDF.
    img->pdf = this->pdf;

    // Load a copy of our data (must be called after
    // giving the clone a copy of our PDF -- it may change
    // the PDF we've given it).
    img->loadData(std::string(this->binaryData), nullptr);

    return img;
}

auto TexImage::clone() const -> ElementPtr { return cloneTexImage(); }

void TexImage::setWidth(double width) {
    this->snappedBounds.width = width;
    this->calcSize();
}

void TexImage::setHeight(double height) {
    this->snappedBounds.height = height;
    this->calcSize();
}

auto TexImage::cairoReadFunction(TexImage* image, unsigned char* data, unsigned int length) -> cairo_status_t {
    for (unsigned int i = 0; i < length; i++, image->read++) {
        if (image->read >= image->binaryData.length()) {
            return CAIRO_STATUS_READ_ERROR;
        }
        data[i] = static_cast<unsigned char>(image->binaryData[image->read]);
    }

    return CAIRO_STATUS_SUCCESS;
}

/**
 * Gets the binary data, a .PNG image or a .PDF
 */
auto TexImage::getBinaryData() const -> std::string const& { return this->binaryData; }

void TexImage::setText(std::string text) { this->text = std::move(text); }

auto TexImage::getText() const -> std::string { return this->text; }

auto TexImage::loadData(std::string&& bytes, GError** err) -> bool {
    this->freeImageAndPdf();
    this->binaryData = bytes;
    if (this->binaryData.length() < 4) {
        return false;
    }

    const std::string type = binaryData.substr(1, 3);
    if (type == "PDF") {
        // Note: binaryData must not be modified while pdf is live.
        auto* bytes = g_bytes_new_with_free_func(this->binaryData.data(), this->binaryData.size(), nullptr, nullptr);
        this->pdf.reset(poppler_document_new_from_bytes(bytes, nullptr, err), xoj::util::adopt);
        g_bytes_unref(bytes);

        if (!pdf.get() || poppler_document_get_n_pages(this->pdf.get()) < 1) {
            return false;
        }
        if (std::abs(this->snappedBounds.area()) <= std::numeric_limits<double>::epsilon()) {
            xoj::util::GObjectSPtr<PopplerPage> page(poppler_document_get_page(this->pdf.get(), 0), xoj::util::adopt);
            poppler_page_get_size(page.get(), &this->snappedBounds.width, &this->snappedBounds.height);
        }
    } else if (type == "PNG") {
        this->image.reset(cairo_image_surface_create_from_png_stream(
                                  reinterpret_cast<cairo_read_func_t>(&cairoReadFunction), this),
                          xoj::util::adopt);
    } else {
        g_warning("Unknown Latex image type: \"%s\"", type.c_str());
    }

    return true;
}

auto TexImage::getImage() const -> cairo_surface_t* { return this->image.get(); }

auto TexImage::getPdf() const -> PopplerDocument* { return this->pdf.get(); }

void TexImage::serialize(ObjectOutputStream& out) const {
    out.writeObject("TexImage");

    this->RectangularElement::serialize(out);

    out.writeDouble(this->snappedBounds.width);
    out.writeDouble(this->snappedBounds.height);
    out.writeString(this->text);

    out.writeString(this->binaryData);

    out.endObject();
}

void TexImage::readSerialized(ObjectInputStream& in) {
    in.readObject("TexImage");

    this->RectangularElement::readSerialized(in);

    this->snappedBounds.width = in.readDouble();
    this->snappedBounds.height = in.readDouble();
    this->text = in.readString();

    std::string data = in.readString();
    this->loadData(std::move(data), nullptr);

    in.endObject();
    this->calcSize();
}

void TexImage::calcSize() const {
    this->boundingBox = this->snappedBounds;
    this->sizeCalculated = true;
}
