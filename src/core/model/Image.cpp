#include "Image.h"

#include <algorithm>  // for min
#include <array>      // for array
#include <utility>    // for move, pair

#include <cairo.h>    // for cairo_surface_destroy
#include <gdk/gdk.h>  // for gdk_cairo_set_sourc...
#include <glib.h>     // for g_assert, guchar

#include "model/Element.h"                        // for Element, ELEMENT_IMAGE
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

    if (this->partialImage.mod_img) {
        cairo_surface_destroy(this->partialImage.mod_img);
        this->partialImage.mod_img = nullptr;
    }

    if (this->format) {
        gdk_pixbuf_format_free(this->format);
        this->format = nullptr;
    }
}

auto Image::clone() const -> Element* {
    auto* img = new Image();

    img->x = this->x;
    img->y = this->y;
    img->setColor(this->getColor());
    img->width = this->width;
    img->height = this->height;
    img->data = this->data;

    img->image = cairo_surface_reference(this->image);

    img->partialImage.mod_img = cairo_surface_reference(this->partialImage.mod_img);
    img->partialImage.alphaForIgnore = this->partialImage.alphaForIgnore;
    img->partialImage.xIgnoreP = this->partialImage.xIgnoreP;
    img->partialImage.xDrawP = this->partialImage.xDrawP;
    img->partialImage.yIgnoreP = this->partialImage.yIgnoreP;
    img->partialImage.yDrawP = this->partialImage.yDrawP;

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
    g_assert(this->format != nullptr && "could not parse the image format!");

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
}

auto Image::getImage() const -> cairo_surface_t* {
    g_assert(data.length() > 0 && "image has no data, cannot render it!");
    if (this->image == nullptr) {
        xoj::util::GObjectSPtr<GdkPixbufLoader> loader(gdk_pixbuf_loader_new(), xoj::util::adopt);
        gdk_pixbuf_loader_write(loader.get(), reinterpret_cast<const guchar*>(this->data.c_str()), this->data.length(),
                                nullptr);
        bool success = gdk_pixbuf_loader_close(loader.get(), nullptr);
        g_assert(success && "errors in loading image data!");

        GdkPixbuf* tmp = gdk_pixbuf_loader_get_pixbuf(loader.get());
        g_assert(tmp != nullptr);
        GdkPixbuf* pixbuf = gdk_pixbuf_apply_embedded_orientation(tmp);

        this->imageSize = {gdk_pixbuf_get_width(pixbuf), gdk_pixbuf_get_height(pixbuf)};

        // TODO: pass in window once this code is refactored into ImageView
        this->image = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, gdk_pixbuf_get_width(pixbuf),
                                                 gdk_pixbuf_get_height(pixbuf));
        g_assert(this->image != nullptr);

        // Paint the pixbuf on to the surface
        // NOTE: we do this manually instead of using gdk_cairo_surface_create_from_pixbuf
        // since this does not work in CLI mode.
        cairo_t* cr = cairo_create(this->image);
        gdk_cairo_set_source_pixbuf(cr, pixbuf, 0, 0);
        cairo_paint(cr);
        cairo_destroy(cr);
    }

    return this->image;
}

auto Image::getPartialImage(double xIgnoreP, double yIgnoreP, double xDrawP, double yDrawP, double alphaForIgnore) const
        -> cairo_surface_t* {
    if (this->partialImage.mod_img != nullptr && partialImage.alphaForIgnore == alphaForIgnore &&
        partialImage.xIgnoreP == xIgnoreP && partialImage.xDrawP == xDrawP && partialImage.yIgnoreP == yIgnoreP &&
        partialImage.yDrawP == yDrawP) {
        return partialImage.mod_img;
    }
    // only create the partial view surface if necessary

    if (this->partialImage.mod_img) {
        cairo_surface_destroy(this->partialImage.mod_img);
        this->partialImage.mod_img = nullptr;
    }

    this->partialImage.alphaForIgnore = alphaForIgnore;
    this->partialImage.xIgnoreP = xIgnoreP;
    this->partialImage.yIgnoreP = yIgnoreP;
    this->partialImage.xDrawP = xDrawP;
    this->partialImage.yDrawP = yDrawP;


    cairo_surface_t* img = getImage();
    int const img_width = cairo_image_surface_get_width(img);
    int const img_height = cairo_image_surface_get_height(img);

    unsigned char* raw_img_data = cairo_image_surface_get_data(img);

    this->partialImage.mod_img = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, img_width, img_height);
    cairo_t* del_cr = cairo_create(this->partialImage.mod_img);

    unsigned char* mod_img_data = cairo_image_surface_get_data(partialImage.mod_img);

    cairo_paint(del_cr);
    cairo_destroy(del_cr);

    int const cutoff_ignore_x = static_cast<int>(xIgnoreP * img_width);
    int const cutoff_draw_x = static_cast<int>(xDrawP * img_width);
    int const cutoff_ignore_y = static_cast<int>(yIgnoreP * img_height);
    int const cutoff_draw_y = static_cast<int>(yDrawP * img_height);

    for (int row = 0; row < img_height; row++) {
        for (int col = 0; col < img_width; col++) {
            const int chars_in_previous_rows = row * img_width * 4;
            const int cur_pos = chars_in_previous_rows + (col * 4);  // col * 4 is the chars in this row

            const bool apply_alpha =
                    (row < cutoff_ignore_y || row > cutoff_draw_y) || (col < cutoff_ignore_x || col > cutoff_draw_x);

            if (apply_alpha) {

                // alpha
                mod_img_data[cur_pos] = static_cast<unsigned char>(alphaForIgnore);

                // red green blue : premultiplied!
                mod_img_data[cur_pos + 1] = static_cast<unsigned char>(raw_img_data[cur_pos + 1] * alphaForIgnore);
                mod_img_data[cur_pos + 2] = static_cast<unsigned char>(raw_img_data[cur_pos + 2] * alphaForIgnore);
                mod_img_data[cur_pos + 3] = static_cast<unsigned char>(raw_img_data[cur_pos + 3] * alphaForIgnore);
            } else {

                // alpha
                mod_img_data[cur_pos] = raw_img_data[cur_pos];

                // red green blue
                mod_img_data[cur_pos + 1] = raw_img_data[cur_pos + 1];
                mod_img_data[cur_pos + 2] = raw_img_data[cur_pos + 2];
                mod_img_data[cur_pos + 3] = raw_img_data[cur_pos + 3];
            }
        }
    }

    return this->partialImage.mod_img;
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

    if (this->partialImage.mod_img) {
        cairo_surface_destroy(this->partialImage.mod_img);
        this->partialImage.mod_img = nullptr;
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
