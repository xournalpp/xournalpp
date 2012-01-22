#include "Image.h"
#include <serializing/ObjectOutputStream.h>
#include <serializing/ObjectInputStream.h>
#include <pixbuf-utils.h>

#include <string.h> // memcpy

Image::Image() :
	Element(ELEMENT_IMAGE) {

	XOJ_INIT_TYPE(Image);

	this->sizeCalculated = true;
	this->image = NULL;
	this->data = NULL;
	this->dLen = 0;
	this->read = false;
}

Image::~Image() {
	XOJ_CHECK_TYPE(Image);

	if (this->image) {
		cairo_surface_destroy(this->image);
		this->image = NULL;
	}

	XOJ_RELEASE_TYPE(Image);
}

Element * Image::clone() {
	XOJ_CHECK_TYPE(Image);

	Image * img = new Image();

	img->x = this->x;
	img->y = this->y;
	img->setColor(this->getColor());
	img->width = this->width;
	img->height = this->height;

	img->data = (unsigned char *)g_malloc(this->dLen);
	img->dLen = this->dLen;
	memcpy(img->data, this->data, this->dLen);

	img->image = cairo_surface_reference(this->image);

	return img;
}


void Image::setWidth(double width) {
	XOJ_CHECK_TYPE(Image);

	this->width = width;
}

void Image::setHeight(double height) {
	XOJ_CHECK_TYPE(Image);

	this->height = height;
}

cairo_status_t Image::cairoReadFunction(Image * image, unsigned char * data, unsigned int length) {
	XOJ_CHECK_TYPE_OBJ(image, Image);

	for (int i = 0; i < length; i++, image->read++) {
		if (image->read >= image->dLen) {
			return CAIRO_STATUS_READ_ERROR;
		}
		data[i] = image->data[image->read];
	}

	return CAIRO_STATUS_SUCCESS;
}

void Image::setImage(unsigned char * data, int len) {
	XOJ_CHECK_TYPE(Image);

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

void Image::setImage(GdkPixbuf * img) {
	setImage(f_pixbuf_to_cairo_surface(img));
}

void Image::setImage(cairo_surface_t * image) {
	XOJ_CHECK_TYPE(Image);

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
	XOJ_CHECK_TYPE(Image);

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
	XOJ_CHECK_TYPE(Image);

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
	XOJ_CHECK_TYPE(Image);

	out.writeObject("Image");

	serializeElement(out);

	out.writeDouble(this->width);
	out.writeDouble(this->height);

	out.writeImage(this->image);

	out.endObject();
}

void Image::readSerialized(ObjectInputStream & in) throw (InputStreamException) {
	XOJ_CHECK_TYPE(Image);

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
	XOJ_CHECK_TYPE(Image);
}

