#include "Image.h"

#include <algorithm>  // for min
#include <array>      // for array
#include <memory>
#include <utility>    // for move, pair

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

    img->x = this->x;
    img->y = this->y;
    img->setColor(this->getColor());
    img->width = this->width;
    img->height = this->height;
    img->data = this->data;

    img->image = cairo_surface_reference(this->image);
    img->snappedBounds = this->snappedBounds;
    img->sizeCalculated = this->sizeCalculated;

    return img;
}

void Image::setWidth(double width) {
    this->width = width;
    this->calcSize();
}

void Image::setHeight(double height) {
    this->height = height;
    this->calcSize();
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
    if (this->image) {
        cairo_surface_destroy(this->image);
        this->image = nullptr;
    }
    this->imageSize = {gdk_pixbuf_get_width(img), gdk_pixbuf_get_height(img)};

    this->image = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, this->imageSize.first, this->imageSize.second);
    xoj_assert(this->image != nullptr);

    // Paint the pixbuf on to the surface
    cairo_t* cr = cairo_create(this->image);
    gdk_cairo_set_source_pixbuf(cr, img, 0, 0);
    cairo_paint(cr);
    cairo_destroy(cr);

    struct {
        std::string buffer;
    } closure_;
    const cairo_write_func_t writeFunc = [](void* closurePtr, const unsigned char* data,
                                            unsigned int length) -> cairo_status_t {
        auto& closure = *reinterpret_cast<decltype(&closure_)>(closurePtr);
        closure.buffer.append(reinterpret_cast<const char*>(data), length);
        return CAIRO_STATUS_SUCCESS;
    };
    cairo_surface_write_to_png_stream(image, writeFunc, &closure_);

    data = std::move(closure_.buffer);
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

void Image::scale(double x0, double y0, double fx, double fy, double rotation,
                  bool) {  // line width scaling option is not used
    this->x -= x0;
    this->x *= fx;
    this->x += x0;
    this->y -= y0;
    this->y *= fy;
    this->y += y0;

    this->width *= fx;
    this->height *= fy;
    this->calcSize();
}

void Image::rotate(double x0, double y0, double th) {}

void Image::serialize(ObjectOutputStream& out) const {
    out.writeObject("Image");

    this->Element::serialize(out);

    out.writeDouble(this->width);
    out.writeDouble(this->height);

    out.writeImage(this->data);

    out.endObject();
}

void Image::readSerialized(ObjectInputStream& in) {
    in.readObject("Image");

    this->Element::readSerialized(in);

    this->width = in.readDouble();
    this->height = in.readDouble();

    if (this->image) {
        cairo_surface_destroy(this->image);
        this->image = nullptr;
    }

    this->data = in.readImage();

    in.endObject();
    this->calcSize();
}

void Image::calcSize() const {
    this->snappedBounds = Rectangle<double>(this->x, this->y, this->width, this->height);
    this->sizeCalculated = true;
}

bool Image::hasData() const { return !this->data.empty(); }

const unsigned char* Image::getRawData() const { return reinterpret_cast<const unsigned char*>(this->data.data()); }

size_t Image::getRawDataLength() const { return this->data.size(); }

std::pair<int, int> Image::getImageSize() const { return this->imageSize; }

GdkPixbufFormat* Image::getImageFormat() const { return this->format; }
