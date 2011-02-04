#include "Image.h"
#include "../util/ObjectStream.h"

Image::Image() :
	Element(ELEMENT_IMAGE) {
	sizeCalculated = true;
	this->image = NULL;
	this->data = NULL;
	this->dLen = 0;
	this->read = false;
}

Image::~Image() {
	if (this->image) {
		cairo_surface_destroy(this->image);
	}
}

void Image::setWidth(double width) {
	this->width = width;
}

void Image::setHeight(double height) {
	this->height = height;
}

cairo_status_t Image::cairoReadFunction(Image * image, unsigned char *data, unsigned int length) {
	for (int i = 0; i < length; i++, image->read++) {
		if (image->read >= image->dLen) {
			return CAIRO_STATUS_READ_ERROR;
		}
		data[i] = image->data[image->read];
	}

	return CAIRO_STATUS_SUCCESS;
}

void Image::setImage(unsigned char * data, int len) {
	if (this->image) {
		cairo_surface_destroy(this->image);
		this->image = NULL;
	}
	if (this->data) {
		g_free(this->data);
	}
	this->data = data;
	this->dLen = len;
}

void Image::setImage(cairo_surface_t * image) {
	if (this->image) {
		cairo_surface_destroy(this->image);
		this->image = NULL;
	}
	if (this->data) {
		g_free(this->data);
	}
	this->data = NULL;
	this->dLen = 0;

	this->image = image;
}

cairo_surface_t * Image::getImage() {
	if (this->image == NULL && this->dLen != 0) {
		this->read = 0;
		this->image = cairo_image_surface_create_from_png_stream((cairo_read_func_t) &cairoReadFunction, this);
		g_free(this->data);
		this->data = NULL;
		this->dLen = 0;
	}

	return this->image;
}

void Image::scale(double x0, double y0, double fx, double fy) {
	this->x -= x0;
	this->x *= fx;
	this->x += x0;
	this->y -= y0;
	this->y *= fy;
	this->y += y0;

	this->width *= fx;
	this->height *= fy;
}

void Image::serialize(ObjectOutputStream & out) {
	out.writeObject("Image");

	serializeElement(out);

	out.writeDouble(this->width);
	out.writeDouble(this->height);

	out.writeImage(this->image);

	out.endObject();
}

void Image::readSerialized(ObjectInputStream & in) throw (InputStreamException) {
	in.readObject("Image");

	readSerializedElement(in);

	this->width = in.readDouble();
	this->height = in.readDouble();

	if (this->image) {
		cairo_surface_destroy(this->image);
		this->image = NULL;
	}

	this->image = in.readImage();

	in.endObject();
}

void Image::calcSize() {
}

