/*
 * Xournal++
 *
 * A TexImage on the document
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "Element.h"
#include <XournalType.h>

#include <poppler.h>


class TexImage: public Element
{
public:
	TexImage();
	virtual ~TexImage();

public:
	void setWidth(double width);
	void setHeight(double height);

	/**
	 * Sets the binary data, a .PNG image or a .PDF
	 */
	void setBinaryData(string binaryData);

	/**
	 * Gets the binary data, a .PNG image or a .PDF
	 */
	string& getBinaryData();

	/**
	 * Get the Image, if rendered as image
	 */
	cairo_surface_t* getImage();

	/**
	 * @return The PDF Document, if rendered as .pdf
	 *
	 * The document needs to be referenced, if it will be hold somewhere
	 */
	PopplerDocument* getPdf();

	/**
	 * @param pdf The PDF Document, if rendered as .pdf
	 *
	 * The PDF will be referenced
	 */
	void setPdf(PopplerDocument* pdf);

	virtual void scale(double x0, double y0, double fx, double fy);
	virtual void rotate(double x0, double y0, double xo, double yo, double th);

	// text tag to alow latex
	void setText(string text);
	string getText();

	virtual Element* clone();

public:
	// Serialize interface
	void serialize(ObjectOutputStream& out);
	void readSerialized(ObjectInputStream& in);

private:
	virtual void calcSize();

	static cairo_status_t cairoReadFunction(TexImage* image, unsigned char* data, unsigned int length);

	/**
	 * Free image and PDF
	 */
	void freeImageAndPdf();

	/**
	 * Load the binary data, either .PNG or .PDF
	 */
	void loadBinaryData();

private:
	/**
	 * Tex PDF Document, if rendered as PDF
	 */
	PopplerDocument* pdf = nullptr;

	/**
	 * Tex image, if rendered as image
	 */
	cairo_surface_t* image = nullptr;

	/**
	 * PNG Image / PDF Document
	 */
	string binaryData;

	/**
	 * Flag if the binary data is already parsed
	 */
	bool parsedBinaryData = false;

	/**
	 * Read position in binaryData
	 */
	string::size_type read = 0;

	/**
	 * Tex String
	 */
	string text;
};
