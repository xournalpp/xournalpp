#include "TexImage.h"

#include <pixbuf-utils.h>
#include <serializing/ObjectInputStream.h>
#include <serializing/ObjectOutputStream.h>

// TODO Serialize PDF

TexImage::TexImage()
 : Element(ELEMENT_TEXIMAGE)
{
	XOJ_INIT_TYPE(TexImage);

	this->sizeCalculated = true;
}

TexImage::~TexImage()
{
	XOJ_CHECK_TYPE(TexImage);

	if (this->image)
	{
		cairo_surface_destroy(this->image);
		this->image = NULL;
	}

	if (this->pdf)
	{
		g_object_unref(this->pdf);
		this->pdf = NULL;
	}

	XOJ_RELEASE_TYPE(TexImage);
}

Element* TexImage::clone()
{
	XOJ_CHECK_TYPE(TexImage);

	TexImage* img = new TexImage();

	img->x = this->x;
	img->y = this->y;
	img->setColor(this->getColor());
	img->width = this->width;
	img->height = this->height;
	img->text = this->text;
	img->data = this->data;

	if (this->pdf)
	{
		img->pdf = this->pdf;
		g_object_ref(img->pdf);
	}

	img->image = cairo_surface_reference(this->image);

	return img;
}

void TexImage::setWidth(double width)
{
	XOJ_CHECK_TYPE(TexImage);

	this->width = width;
}

void TexImage::setHeight(double height)
{
	XOJ_CHECK_TYPE(TexImage);

	this->height = height;
}

cairo_status_t TexImage::cairoReadFunction(TexImage* image, unsigned char* data, unsigned int length)
{
	XOJ_CHECK_TYPE_OBJ(image, TexImage);

	for (unsigned int i = 0; i < length; i++, image->read++)
	{
		if (image->read >= image->data.length())
		{
			return CAIRO_STATUS_READ_ERROR;
		}
		data[i] = image->data[image->read];
	}

	return CAIRO_STATUS_SUCCESS;
}

void TexImage::setImage(string data)
{
	XOJ_CHECK_TYPE(TexImage);

	if (this->image)
	{
		cairo_surface_destroy(this->image);
		this->image = NULL;
	}
	this->data = data;
}

void TexImage::setImage(GdkPixbuf* img)
{
	setImage(f_pixbuf_to_cairo_surface(img));
}

void TexImage::setImage(cairo_surface_t* image)
{
	XOJ_CHECK_TYPE(TexImage);

	if (this->image)
	{
		cairo_surface_destroy(this->image);
		this->image = NULL;
	}

	this->image = image;
}

void TexImage::setText(string text)
{
	this->text = text;
}

string TexImage::getText()
{
	return this->text;
}

cairo_surface_t* TexImage::getImage()
{
	XOJ_CHECK_TYPE(TexImage);

	if (this->image == NULL && this->data.length() != 0)
	{
		this->read = 0;
		this->image = cairo_image_surface_create_from_png_stream((cairo_read_func_t) &cairoReadFunction, this);
	}

	return this->image;
}

/**
 * @return The PDF Document, if rendered as .pdf
 *
 * The document needs to be referenced, if it will be hold somewhere
 */
PopplerDocument* TexImage::getPdf()
{
	XOJ_CHECK_TYPE(TexImage);

	return this->pdf;
}

/**
 * @param pdf The PDF Document, if rendered as .pdf
 *
 * The PDF will be referenced
 */
void TexImage::setPdf(PopplerDocument* pdf)
{
	XOJ_CHECK_TYPE(TexImage);

	if (this->pdf != nullptr)
	{
		g_object_unref(this->pdf);
	}

	this->pdf = pdf;

	if (this->pdf != nullptr)
	{
		g_object_ref(this->pdf);
	}
}

void TexImage::scale(double x0, double y0, double fx, double fy)
{
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

void TexImage::rotate(double x0, double y0, double xo, double yo, double th)
{
	XOJ_CHECK_TYPE(TexImage);

	// Rotation for TexImages not yet implemented
}

void TexImage::serialize(ObjectOutputStream& out)
{
	XOJ_CHECK_TYPE(TexImage);

	out.writeObject("TexImage");

	serializeElement(out);

	out.writeDouble(this->width);
	out.writeDouble(this->height);
	out.writeString(this->text);

	out.writeImage(this->image);

	out.endObject();
}

void TexImage::readSerialized(ObjectInputStream& in)
{
	XOJ_CHECK_TYPE(TexImage);

	in.readObject("TexImage");

	readSerializedElement(in);

	this->width = in.readDouble();
	this->height = in.readDouble();
	this->text = in.readString();

	if (this->image)
	{
		cairo_surface_destroy(this->image);
		this->image = NULL;
	}

	this->image = in.readImage();

	in.endObject();
}

void TexImage::calcSize()
{
	XOJ_CHECK_TYPE(TexImage);
}
