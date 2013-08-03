#include "TexImage.h"
#include <serializing/ObjectOutputStream.h>
#include <serializing/ObjectInputStream.h>
#include <pixbuf-utils.h>

TexImage::TexImage() :
	Element(ELEMENT_TEXIMAGE) {

	XOJ_INIT_TYPE(TexImage);

	this->sizeCalculated = true;
	this->image = NULL;
	this->data = NULL;
	this->dLen = 0;
	this->read = false;

	//text tags
	this->text = NULL;
	this->textlen = 0;
}

TexImage::~TexImage() {
	XOJ_CHECK_TYPE(TexImage);

	if (this->image) {
		cairo_surface_destroy(this->image);
		this->image = NULL;
	}

	XOJ_RELEASE_TYPE(TexImage);
}

void TexImage::setWidth(double width) {
	XOJ_CHECK_TYPE(TexImage);

	this->width = width;
}

void TexImage::setHeight(double height) {
	XOJ_CHECK_TYPE(TexImage);

	this->height = height;
}

cairo_status_t TexImage::cairoReadFunction(TexImage * image, unsigned char * data, unsigned int length) {
	XOJ_CHECK_TYPE_OBJ(image, TexImage);

	for (int i = 0; i < length; i++, image->read++) {
		if (image->read >= image->dLen) {
			return CAIRO_STATUS_READ_ERROR;
		}
		data[i] = image->data[image->read];
	}

	return CAIRO_STATUS_SUCCESS;
}

void TexImage::setImage(unsigned char * data, int len) {
	XOJ_CHECK_TYPE(TexImage);

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

void TexImage::setImage(GdkPixbuf * img) {
	setImage(f_pixbuf_to_cairo_surface(img));
}

void TexImage::setImage(cairo_surface_t * image) {
	XOJ_CHECK_TYPE(TexImage);

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

void TexImage::setText(unsigned char * text, int textlen)
{
	this->text = text;
	this->textlen = textlen;
}

unsigned char * TexImage::getText()
{
	return this->text;
}

cairo_surface_t * TexImage::getImage() {
	XOJ_CHECK_TYPE(TexImage);

	if (this->image == NULL && this->dLen != 0) {
		this->read = 0;
		this->image = cairo_image_surface_create_from_png_stream((cairo_read_func_t) &cairoReadFunction, this);
		g_free(this->data);
		this->data = NULL;
		this->dLen = 0;
	}

	return this->image;
}

void TexImage::scale(double x0, double y0, double fx, double fy) {
	XOJ_CHECK_TYPE(TexImage);

	this->x -= x0;
	this->x *= fx;
	this->x += x0;
	this->y -= y0;
	this->y *= fy;
	this->y += y0;

	this->width *= fx;
	this->height *= fy;
}

void TexImage::serialize(ObjectOutputStream & out) {
	XOJ_CHECK_TYPE(TexImage);

	out.writeObject("TexImage");

	serializeElement(out);

	out.writeDouble(this->width);
	out.writeDouble(this->height);

	out.writeImage(this->image);

	out.endObject();
}

void TexImage::readSerialized(ObjectInputStream & in) throw (InputStreamException) {
	XOJ_CHECK_TYPE(TexImage);

	in.readObject("TexImage");

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

void TexImage::calcSize() {
	XOJ_CHECK_TYPE(TexImage);
}

