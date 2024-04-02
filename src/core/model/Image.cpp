#include "Image.h"

#include <algorithm>  // for min
#include <array>      // for array
#include <memory>
#include <tuple>
#include <utility>  // for move, pair

#include <cairo.h>    // for cairo_surface_destroy
#include <gdk/gdk.h>  // for gdk_cairo_set_sourc...
#include <glib.h>     // for guchar

#include "model/Element.h"                        // for Element, ELEMENT_IMAGE
#include "util/Assert.h"                          // for xoj_assert
#include "util/Rectangle.h"                       // for Rectangle
#include "util/raii/GObjectSPtr.h"                // for GObjectSPtr
#include "util/serializing/ObjectInputStream.h"   // for ObjectInputStream
#include "util/serializing/ObjectOutputStream.h"  // for ObjectOutputStream

using xoj::util::Rectangle;

Image::Image(): Element(ELEMENT_IMAGE) {}

Image::~Image() {
    if (this->image) {
        cairo_surface_destroy(this->image);
        this->image = nullptr;
    }

    if (this->format) {
        gdk_pixbuf_format_free(this->format);
        this->format = nullptr;
    }
}

auto Image::clone() const -> ElementPtr {
    auto img = std::make_unique<Image>();

    img->setColor(this->getColor());
    img->data = this->data;

    img->image = cairo_surface_reference(this->image);
    img->imageSize = this->imageSize;
    return img;
}

void Image::setImage(std::string_view data) { setImage(std::string(data)); }

void Image::setImage(std::string&& data) {
    if (this->image) {
        cairo_surface_destroy(this->image);
        this->image = nullptr;
    }
    this->data = std::move(data);

    if (this->format) {
        gdk_pixbuf_format_free(this->format);
        this->format = nullptr;
    }

    // FIXME: awful hack to try to parse the format
    std::array<char*, 4096> buffer{};
    xoj::util::GObjectSPtr<GdkPixbufLoader> loader(gdk_pixbuf_loader_new(), xoj::util::adopt);
    size_t remaining = this->data.size();
    while (remaining > 0) {
        size_t readLen = std::min(remaining, buffer.size());
        if (!gdk_pixbuf_loader_write(loader.get(), reinterpret_cast<const guchar*>(this->data.c_str()), readLen,
                                     nullptr))
            break;
        remaining -= readLen;

        // Try to determine the format early, if possible
        this->format = gdk_pixbuf_loader_get_format(loader.get());
        if (this->format) {
            break;
        }
    }
    gdk_pixbuf_loader_close(loader.get(), nullptr);
    // if the format was not determined early, it can probably be determined now
    if (!this->format) {
        this->format = gdk_pixbuf_loader_get_format(loader.get());
    }
    xoj_assert_message(this->format != nullptr, "could not parse the image format!");

    // the format is owned by the pixbuf, so create a copy
    this->format = gdk_pixbuf_format_copy(this->format);
}

void Image::setImage(GdkPixbuf* img) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    setImage(gdk_cairo_surface_create_from_pixbuf(img, 0, nullptr));
#pragma GCC diagnostic pop
}

void Image::setImage(cairo_surface_t* image) {
    if (this->image) {
        cairo_surface_destroy(this->image);
        this->image = nullptr;
    }

    struct {
        std::string buffer;
        std::string readbuf;
    } closure_;
    const cairo_write_func_t writeFunc = [](void* closurePtr, const unsigned char* data,
                                            unsigned int length) -> cairo_status_t {
        auto& closure = *reinterpret_cast<decltype(&closure_)>(closurePtr);
        closure.buffer.append(reinterpret_cast<const char*>(data), length);
        return CAIRO_STATUS_SUCCESS;
    };
    cairo_surface_write_to_png_stream(image, writeFunc, &closure_);

    data = std::move(closure_.buffer);
    this->imageSize = {cairo_image_surface_get_width(image), cairo_image_surface_get_height(image)};
}

auto Image::getImage() const -> cairo_surface_t* {
    xoj_assert_message(data.length() > 0, "image has no data, cannot render it!");
    if (this->image == nullptr) {
        xoj::util::GObjectSPtr<GdkPixbufLoader> loader(gdk_pixbuf_loader_new(), xoj::util::adopt);
        gdk_pixbuf_loader_write(loader.get(), reinterpret_cast<const guchar*>(this->data.c_str()), this->data.length(),
                                nullptr);
        [[maybe_unused]] bool success = gdk_pixbuf_loader_close(loader.get(), nullptr);
        xoj_assert_message(success, "errors in loading image data!");

        GdkPixbuf* tmp = gdk_pixbuf_loader_get_pixbuf(loader.get());
        xoj_assert(tmp != nullptr);
        xoj::util::GObjectSPtr<GdkPixbuf> pixbuf(gdk_pixbuf_apply_embedded_orientation(tmp), xoj::util::adopt);

        this->imageSize = {gdk_pixbuf_get_width(pixbuf.get()), gdk_pixbuf_get_height(pixbuf.get())};

        // TODO: pass in window once this code is refactored into ImageView
        this->image = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, this->imageSize.first, this->imageSize.second);
        xoj_assert(this->image != nullptr);

        // Paint the pixbuf on to the surface
        // NOTE: we do this manually instead of using gdk_cairo_surface_create_from_pixbuf
        // since this does not work in CLI mode.
        cairo_t* cr = cairo_create(this->image);
        gdk_cairo_set_source_pixbuf(cr, pixbuf.get(), 0, 0);
        cairo_paint(cr);
        cairo_destroy(cr);
    }

    return this->image;
}

void Image::serialize(ObjectOutputStream& out) const {
    out.writeObject("Image");

    this->Element::serialize(out);
    auto bounds = this->boundingRect();

    out.writeDouble(bounds.width);   // Todo:
    out.writeDouble(bounds.height);  // Todo:

    out.writeImage(this->data);

    out.endObject();
}

void Image::readSerialized(ObjectInputStream& in) {
    in.readObject("Image");

    this->Element::readSerialized(in);


    imageSize.first = in.readDouble();   // Todo:
    imageSize.second = in.readDouble();  // Todo:

    if (this->image) {
        cairo_surface_destroy(this->image);
        this->image = nullptr;
    }

    this->data = in.readImage();

    in.endObject();
    this->internalUpdateBounds();
}

auto Image::internalUpdateBounds() const -> std::pair<xoj::util::Rectangle<double>, xoj::util::Rectangle<double>> {
    auto sizes = getImageSize();
    auto rect = Rectangle<double>(0, 0, sizes.first, sizes.second);
    return {rect, rect};
}

auto Image::hasData() const -> bool { return !this->data.empty(); }

auto Image::getRawData() const -> const unsigned char* {
    return reinterpret_cast<const unsigned char*>(this->data.data());
}

auto Image::getRawDataLength() const -> size_t { return this->data.size(); }

auto Image::getImageSize() const -> std::pair<int, int> { return this->imageSize; }

auto Image::getImageFormat() const -> GdkPixbufFormat* { return this->format; }
