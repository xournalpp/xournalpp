#include "PageType.h"

PageType::PageType()
 : format(PageTypeFormat::Lined)
{
}

PageType::PageType(PageTypeFormat format)
 : format(format)
{
}

PageType::PageType(const PageType& other)
{
	this->format = other.format;
	this->config = other.config;
}

PageType::~PageType()
{
}

/**
 * Compare Operator
 */
bool PageType::operator ==(const PageType& other) const
{
	return this->config == other.config && this->format == other.format;
}

/**
 * PDF background
 */
bool PageType::isPdfPage()
{
	return this->format == PageTypeFormat::Pdf;
}

/**
 * Image Background
 */
bool PageType::isImagePage()
{
	return this->format == PageTypeFormat::Image;
}

/**
 * Special background
 */
bool PageType::isSpecial()
{
	return this->format == PageTypeFormat::Pdf || this->format == PageTypeFormat::Image ||
	       this->format == PageTypeFormat::Copy;
}

