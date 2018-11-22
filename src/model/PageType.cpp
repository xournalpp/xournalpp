#include "PageType.h"

PageType::PageType()
{
	XOJ_INIT_TYPE(PageType);
}

PageType::PageType(const PageType& other)
{
	XOJ_INIT_TYPE(PageType);
	this->format = other.format;
	this->config = other.config;
}

PageType::~PageType()
{
	XOJ_RELEASE_TYPE(PageType);
}

/**
 * PDF background
 */
bool PageType::isPdfPage()
{
	XOJ_CHECK_TYPE(PageType);

	return this->format == ":pdf";
}

/**
 * Image Background
 */
bool PageType::isImagePage()
{
	XOJ_CHECK_TYPE(PageType);

	return this->format == ":image";
}

/**
 * Special background
 */
bool PageType::isSpecial()
{
	XOJ_CHECK_TYPE(PageType);

	return this->format.at(0) == ':';
}

