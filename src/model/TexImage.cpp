#include "TexImage.h"

#include <pixbuf-utils.h>
#include <serializing/ObjectInputStream.h>
#include <serializing/ObjectOutputStream.h>

TexImage::TexImage()
 : Element(ELEMENT_TEXIMAGE)
{
	this->sizeCalculated = true;
}

TexImage::~TexImage()
{
	freeImageAndPdf();
}

/**
 * Free image and PDF
 */
void TexImage::freeImageAndPdf()
{
	if (this->image)
	{
		cairo_surface_destroy(this->image);
		this->image = nullptr;
	}

	if (this->pdf)
	{
		g_object_unref(this->pdf);
		this->pdf = nullptr;
	}

	this->parsedBinaryData = false;
}

Element* TexImage::clone()
{
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
	this->width = width;
}

void TexImage::setHeight(double height)
{
	this->height = height;
}

cairo_status_t TexImage::cairoReadFunction(TexImage* image, unsigned char* data, unsigned int length)
{
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
	this->binaryData = binaryData;
}

/**
 * Gets the binary data, a .PNG image or a .PDF
 */
string& TexImage::getBinaryData()
{
	return this->binaryData;
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
	if (this->image == nullptr && this->parsedBinaryData == false)
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
		this->pdf = poppler_document_new_from_data((char*)this->binaryData.c_str(), this->binaryData.length(), nullptr, nullptr);
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
	if (this->pdf == nullptr && this->parsedBinaryData == false)
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
	if (this->pdf != nullptr)
	{
		g_object_unref(this->pdf);
	}

	this->pdf = pdf;

	if (this->pdf != nullptr)
	{
		g_object_ref(this->pdf);
		this->parsedBinaryData = true;
	}
}

void TexImage::scale(double x0, double y0, double fx, double fy)
{
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
	// Rotation for TexImages not yet implemented
}

void TexImage::serialize(ObjectOutputStream& out)
{
	out.writeObject("TexImage");

	serializeElement(out);

	out.writeDouble(this->width);
	out.writeDouble(this->height);
	out.writeString(this->text);

	out.writeData(this->binaryData.c_str(), this->binaryData.length(), 1);

	out.endObject();
}

void TexImage::readSerialized(ObjectInputStream& in)
{
	in.readObject("TexImage");

	readSerializedElement(in);

	this->width = in.readDouble();
	this->height = in.readDouble();
	this->text = in.readString();

	freeImageAndPdf();

	char* data = nullptr;
	int len = 0;
	in.readData((void**)&data, &len);

	this->binaryData = string(data, len);

	in.endObject();
}

void TexImage::calcSize()
{
}
