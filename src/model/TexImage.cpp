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

	freeImageAndPdf();

	XOJ_RELEASE_TYPE(TexImage);
}

/**
 * Free image and PDF
 */
void TexImage::freeImageAndPdf()
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
	img->binaryData = this->binaryData;

	if (this->pdf)
	{
		img->pdf = this->pdf;
		g_object_ref(img->pdf);
	}

	if (this->image)
	{
		img->image = cairo_surface_reference(this->image);
	}

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
		if (image->read >= image->binaryData.length())
		{
			return CAIRO_STATUS_READ_ERROR;
		}
		data[i] = image->binaryData[image->read];
	}

	return CAIRO_STATUS_SUCCESS;
}

/**
 * Sets the binary data, a .PNG image or a .PDF
 */
void TexImage::setBinaryData(string binaryData)
{
	XOJ_CHECK_TYPE(TexImage);

	this->binaryData = binaryData;
}

/**
 * Gets the binary data, a .PNG image or a .PDF
 */
string& TexImage::getBinaryData()
{
	XOJ_CHECK_TYPE(TexImage);

	return this->binaryData;
}

void TexImage::setText(string text)
{
	XOJ_CHECK_TYPE(TexImage);

	this->text = text;
}

string TexImage::getText()
{
	XOJ_CHECK_TYPE(TexImage);

	return this->text;
}

cairo_surface_t* TexImage::getImage()
{
	XOJ_CHECK_TYPE(TexImage);

	if (this->image == NULL && this->parsedBinaryData == false)
	{
		loadBinaryData();
	}

	return this->image;
}

/**
 * Load the binary data, either .PNG or .PDF
 */
void TexImage::loadBinaryData()
{
	XOJ_CHECK_TYPE(TexImage);

	freeImageAndPdf();

	if (this->binaryData.length() < 4)
	{
		this->parsedBinaryData = true;
		return;
	}

	string type = this->binaryData.substr(0, 4);

	if (type[1] == 'P' && type[2] == 'N' && type[3] == 'G')
	{
		this->read = 0;
		this->image = cairo_image_surface_create_from_png_stream((cairo_read_func_t) &cairoReadFunction, this);
	}
	else if (type[1] == 'P' && type[2] == 'D' && type[3] == 'F')
	{
		this->pdf = poppler_document_new_from_data((char*)this->binaryData.c_str(), this->binaryData.length(), NULL, NULL);
	}
	else
	{
		g_warning("Unknown Latex image type: «%s»", type.c_str());
	}

	this->parsedBinaryData = true;
}

/**
 * @return The PDF Document, if rendered as .pdf
 *
 * The document needs to be referenced, if it will be hold somewhere
 */
PopplerDocument* TexImage::getPdf()
{
	XOJ_CHECK_TYPE(TexImage);

	if (this->pdf == NULL && this->parsedBinaryData == false)
	{
		loadBinaryData();
	}

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

	if (this->pdf != NULL)
	{
		g_object_unref(this->pdf);
	}

	this->pdf = pdf;

	if (this->pdf != NULL)
	{
		g_object_ref(this->pdf);
		this->parsedBinaryData = true;
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
