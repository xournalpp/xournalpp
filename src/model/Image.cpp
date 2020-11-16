#include "Image.h"

#include <utility>

#include "serializing/ObjectInputStream.h"
#include "serializing/ObjectOutputStream.h"

#include "pixbuf-utils.h"

Image::Image(): Element(ELEMENT_IMAGE) {}

Image::~Image() {
    if (this->image) {
        cairo_surface_destroy(this->image);
        this->image = nullptr;
    }
}

auto Image::clone() -> Element* {
    auto* img = new Image();

    img->x = this->x;
    img->y = this->y;
    img->setColor(this->getColor());
    img->width = this->width;
    img->height = this->height;
    img->data = this->data;

    img->image = cairo_surface_reference(this->image);
    img->calcSize();

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

auto Image::cairoReadFunction(Image* image, unsigned char* data, unsigned int length) -> cairo_status_t {
    for (unsigned int i = 0; i < length; i++, image->read++) {
        if (image->read >= image->data.length()) {
            return CAIRO_STATUS_READ_ERROR;
        }

        data[i] = image->data[image->read];
    }

    return CAIRO_STATUS_SUCCESS;
}

void Image::setImage(string data) {
    if (this->image) {
        cairo_surface_destroy(this->image);
        this->image = nullptr;
    }
    this->data = std::move(data);
}

void Image::setImage(GdkPixbuf* img) { setImage(f_pixbuf_to_cairo_surface(img)); }

void Image::setImage(cairo_surface_t* image) {
    if (this->image) {
        cairo_surface_destroy(this->image);
        this->image = nullptr;
    }

    this->image = image;
}

auto Image::getImage() -> cairo_surface_t* {
    if (this->image == nullptr && this->data.length()) {
        this->read = 0;
        this->image = cairo_image_surface_create_from_png_stream(
                reinterpret_cast<cairo_read_func_t>(&cairoReadFunction), this);
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

void Image::serialize(ObjectOutputStream& out) {
    out.writeObject("Image");

    serializeElement(out);

    out.writeDouble(this->width);
    out.writeDouble(this->height);

    out.writeImage(this->image);

    out.endObject();
}

void Image::readSerialized(ObjectInputStream& in) {
    in.readObject("Image");

    readSerializedElement(in);

    this->width = in.readDouble();
    this->height = in.readDouble();

    if (this->image) {
        cairo_surface_destroy(this->image);
        this->image = nullptr;
    }

    this->image = in.readImage();

    in.endObject();
    this->calcSize();
}

void Image::calcSize() const {
    this->snappedBounds = Rectangle<double>(this->x, this->y, this->width, this->height);
    this->sizeCalculated = true;
}
