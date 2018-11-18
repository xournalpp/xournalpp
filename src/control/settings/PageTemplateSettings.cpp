#include "PageTemplateSettings.h"

#include <sstream>

using std::stringstream;

PageTemplateSettings::PageTemplateSettings()
 : copyLastPageSettings(true),
   pageWidth(100),
   pageHeight(100),
   sizeUnit("cm"),
   backgroundColor(0xffffff)
{
	XOJ_INIT_TYPE(PageTemplateSettings);
}

PageTemplateSettings::~PageTemplateSettings()
{
	XOJ_RELEASE_TYPE(PageTemplateSettings);
}

bool PageTemplateSettings::isCopyLastPageSettings()
{
	XOJ_CHECK_TYPE(PageTemplateSettings);

	return this->copyLastPageSettings;
}

void PageTemplateSettings::setCopyLastPageSettings(bool copyLastPageSettings)
{
	XOJ_CHECK_TYPE(PageTemplateSettings);

	this->copyLastPageSettings = copyLastPageSettings;
}


int PageTemplateSettings::getPageWidth()
{
	XOJ_CHECK_TYPE(PageTemplateSettings);

	return this->pageWidth;
}

void PageTemplateSettings::setPageWidth(int pageWidth)
{
	XOJ_CHECK_TYPE(PageTemplateSettings);

	this->pageWidth = pageWidth;
}

int PageTemplateSettings::getPageHeight()
{
	XOJ_CHECK_TYPE(PageTemplateSettings);

	return this->pageHeight;
}

void PageTemplateSettings::setPageHeight(int pageHeight)
{
	XOJ_CHECK_TYPE(PageTemplateSettings);

	this->pageHeight = pageHeight;
}

string PageTemplateSettings::getSizeUnit()
{
	XOJ_CHECK_TYPE(PageTemplateSettings);

	return sizeUnit;
}

void PageTemplateSettings::setSizeUnit(string sizeUnit)
{
	XOJ_CHECK_TYPE(PageTemplateSettings);

	this->sizeUnit = sizeUnit;
}

int PageTemplateSettings::getBackgroundColor()
{
	XOJ_CHECK_TYPE(PageTemplateSettings);

	return this->backgroundColor;
}

void PageTemplateSettings::setBackgroundColor(int backgroundColor)
{
	XOJ_CHECK_TYPE(PageTemplateSettings);

	this->backgroundColor = backgroundColor;
}

/**
 * Parse a template string
 *
 * @return true if valid
 */
bool PageTemplateSettings::parse(string tpl)
{
	XOJ_CHECK_TYPE(PageTemplateSettings);

	stringstream ss(tpl.c_str());
	string line;

	if (!std::getline(ss, line, '\n'))
	{
		return false;
	}

	if (line != "xoj/template")
	{
		return false;
	}

	while (std::getline(ss, line, '\n'))
	{
		printf("=>%s\n",line.c_str());
	}

/*
	copyLastPageSettings(true),
	   pageWidth(100),
	   pageHeight(100),
	   sizeUnit("cm"),
	   backgroundColor(0xffffff)
	*/

	return true;
}

/**
 * Convert to a parsable string
 */
string PageTemplateSettings::toString()
{
	XOJ_CHECK_TYPE(PageTemplateSettings);

	string str = "xoj/template\n";

	str += string("copyLastPageSettings=") + (copyLastPageSettings ? "true" : "false") + "\n";
	str += string("size=") + std::to_string(pageWidth) + "x" + std::to_string(pageHeight) + "\n";
	str += string("sizeUnite=") + sizeUnit + "\n";

	char buffer[64];
	sprintf(buffer, "#%06x", this->backgroundColor);
	str += string("backgroundColor=") + buffer + "\n";

	return str;
}
