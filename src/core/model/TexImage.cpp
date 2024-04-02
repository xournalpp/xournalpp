#include "TexImage.h"

#include <memory>
#include <tuple>
#include <utility>  // for move

#include <poppler-document.h>  // for poppler_document_ge...
#include <poppler-page.h>      // for poppler_page_get_size

#include "model/Element.h"  // for Element, ELEMENT_TE...
#include "util/Assert.h"
#include "util/Rectangle.h"                       // for Rectangle
#include "util/raii/GObjectSPtr.h"                // for GObjectSPtr
#include "util/serializing/ObjectInputStream.h"   // for ObjectInputStream
#include "util/serializing/ObjectOutputStream.h"  // for ObjectOutputStream

using xoj::util::Rectangle;

[[nodiscard]] static auto retrievePdfBounds(PopplerDocument* pdf) -> xoj::util::Rectangle<double> {
    double width = 0;
    double height = 0;
    auto page = xoj::util::GObjectSPtr<PopplerPage>(poppler_document_get_page(pdf, 0), xoj::util::adopt);
    poppler_page_get_size(page.get(), &width, &height);
    return {0, 0, width, height};
}

TexImage::TexImage(): Element(ELEMENT_TEXIMAGE){};
TexImage::~TexImage() = default;

void TexImage::freeImageAndPdf() { this->pdf.reset(); }

auto TexImage::cloneTexImage() const -> std::unique_ptr<TexImage> {

    auto img = std::make_unique<TexImage>();
    img->setColor(this->getColor());
    img->text = this->text;
    img->bounds = this->bounds;

    // Load a copy of our data (must be called after
    // giving the clone a copy of our PDF -- it may change
    // the PDF we've given it).
    img->loadData(std::string(this->binaryData), nullptr);

    return img;
}

auto TexImage::clone() const -> ElementPtr { return cloneTexImage(); }

/**
 * Gets the binary data, a .PNG image or a .PDF
 */
auto TexImage::getBinaryData() const -> std::string const& { return this->binaryData; }

void TexImage::setText(std::string text) { this->text = std::move(text); }

auto TexImage::getText() const -> std::string { return this->text; }

auto TexImage::loadData(std::string&& bytes, GError** err) -> bool {
    this->freeImageAndPdf();
    this->binaryData = std::move(bytes);
    if (this->binaryData.length() < 4) {
        return false;
    }
    const std::string type = binaryData.substr(1, 3);
    if (type != "PDF") {
        g_warning("Unsupported Latex image type: \"%s\"", type.c_str());
        return false;
    }

    // Note: binaryData must not be modified while pdf is live.
    this->pdf.reset(poppler_document_new_from_data(this->binaryData.data(), this->binaryData.size(), nullptr, err),
                    xoj::util::adopt);
    if (!pdf.get() || poppler_document_get_n_pages(this->pdf.get()) < 1) {
        pdf.reset();

        g_warning("Generated PDF is empty:");
        return false;
    }

    this->bounds = retrievePdfBounds(this->pdf.get());
    return true;
}

auto TexImage::getPdf() const -> PopplerDocument* { return this->pdf.get(); }

void TexImage::serialize(ObjectOutputStream& out) const {
    out.writeObject("TexImage");

    this->Element::serialize(out);
    auto br = boundingRect();
    out.writeDouble(br.width);
    out.writeDouble(br.height);
    out.writeString(this->text);

    out.writeString(this->binaryData);

    out.endObject();
}

void TexImage::readSerialized(ObjectInputStream& in) {
    in.readObject("TexImage");

    this->Element::readSerialized(in);

    std::ignore = in.readDouble();
    std::ignore = in.readDouble();
    this->text = in.readString();

    freeImageAndPdf();

    std::string data = in.readString();
    this->loadData(std::move(data), nullptr);

    in.endObject();
    this->bounds = retrievePdfBounds(this->pdf.get());
}

auto TexImage::internalUpdateBounds() const -> std::pair<xoj::util::Rectangle<double>, xoj::util::Rectangle<double>> {
    return {bounds, bounds};
}
