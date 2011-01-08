#include "Image.h"

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

void Image::calcSize() {
}

