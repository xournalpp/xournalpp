#include "PageTemplateSettings.h"

#include "control/xojfile/SaveHandler.h"
#include "control/pagetype/PageTypeHandler.h"

#include <sstream>

using std::stringstream;

PageTemplateSettings::PageTemplateSettings()
 : copyLastPageSettings(true),
   copyLastPageSize(false),
   pageWidth(595.275591),
   pageHeight(841.889764),
   backgroundColor(0xffffff)
{
	backgroundType.format = PageTypeFormat::Lined;
}

PageTemplateSettings::~PageTemplateSettings()
{
}

bool PageTemplateSettings::isCopyLastPageSettings()
{
	return this->copyLastPageSettings;
}

void PageTemplateSettings::setCopyLastPageSettings(bool copyLastPageSettings)
{
	this->copyLastPageSettings = copyLastPageSettings;
}

bool PageTemplateSettings::isCopyLastPageSize()
{
	return this->copyLastPageSize;
}

void PageTemplateSettings::setCopyLastPageSize(bool copyLastPageSize)
{
	this->copyLastPageSize = copyLastPageSize;
}

double PageTemplateSettings::getPageWidth()
{
	return this->pageWidth;
}

void PageTemplateSettings::setPageWidth(double pageWidth)
{
	this->pageWidth = pageWidth;
}

double PageTemplateSettings::getPageHeight()
{
	return this->pageHeight;
}

void PageTemplateSettings::setPageHeight(double pageHeight)
{
	this->pageHeight = pageHeight;
}

int PageTemplateSettings::getBackgroundColor()
{
	return this->backgroundColor;
}

void PageTemplateSettings::setBackgroundColor(int backgroundColor)
{
	this->backgroundColor = backgroundColor;
}

PageType PageTemplateSettings::getBackgroundType()
{
	return backgroundType;
}

PageType PageTemplateSettings::getPageInsertType()
{
	if (copyLastPageSettings)
	{
		return PageType(PageTypeFormat::Copy);
	}

	return backgroundType;
}

void PageTemplateSettings::setBackgroundType(PageType backgroundType)
{
	this->backgroundType = backgroundType;
}

/**
 * Parse a template string
 *
 * @return true if valid
 */
bool PageTemplateSettings::parse(string tpl)
{
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
		size_t pos = line.find("=");
		if (pos == string::npos)
		{
			continue;
		}

		string key = line.substr(0, pos);
		string value = line.substr(pos + 1);

		if (key == "copyLastPageSettings")
		{
			copyLastPageSettings = value == "true";
		}
		else if (key == "copyLastPageSize")
		{
			copyLastPageSize = value == "true";
		}
		else if (key == "size")
		{
			pos = value.find("x");
			pageWidth = std::stod(value.substr(0, pos));
			pageHeight = std::stod(value.substr(pos + 1));
		}
		else if (key == "backgroundColor")
		{
			backgroundColor = std::stoul(value.substr(1), nullptr, 16);
		}
		else if (key == "backgroundType")
		{
			this->backgroundType.format = PageTypeHandler::getPageTypeFormatForString(value);
		}
		else if (key == "backgroundTypeConfig")
		{
			this->backgroundType.config = value;
		}
	}

	return true;
}

/**
 * Convert to a parsable string
 */
string PageTemplateSettings::toString()
{
	string str = "xoj/template\n";

	str += string("copyLastPageSize=") + (copyLastPageSize ? "true" : "false") + "\n";
	str += string("copyLastPageSettings=") + (copyLastPageSettings ? "true" : "false") + "\n";
	str += string("size=") + std::to_string(pageWidth) + "x" + std::to_string(pageHeight) + "\n";
	str += string("backgroundType=") + PageTypeHandler::getStringForPageTypeFormat(backgroundType.format) + "\n";

	if (!backgroundType.config.empty())
	{
		str += string("backgroundTypeConfig=") + backgroundType.config + "\n";
	}

	char buffer[64];
	sprintf(buffer, "#%06x", this->backgroundColor);
	str += string("backgroundColor=") + buffer + "\n";

	return str;
}
