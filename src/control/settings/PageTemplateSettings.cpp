#include "PageTemplateSettings.h"

#include "control/pagetype/PageType.h"
#include "control/xojfile/SaveHandler.h"

#include <sstream>

using std::stringstream;

PageTemplateSettings::PageTemplateSettings()
 : copyLastPageSettings(true),
   pageWidth(595.275591),
   pageHeight(841.889764),
   backgroundColor(0xffffff),
   backgroundType(BACKGROUND_TYPE_LINED)
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


double PageTemplateSettings::getPageWidth()
{
	XOJ_CHECK_TYPE(PageTemplateSettings);

	return this->pageWidth;
}

void PageTemplateSettings::setPageWidth(double pageWidth)
{
	XOJ_CHECK_TYPE(PageTemplateSettings);

	this->pageWidth = pageWidth;
}

double PageTemplateSettings::getPageHeight()
{
	XOJ_CHECK_TYPE(PageTemplateSettings);

	return this->pageHeight;
}

void PageTemplateSettings::setPageHeight(double pageHeight)
{
	XOJ_CHECK_TYPE(PageTemplateSettings);

	this->pageHeight = pageHeight;
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

BackgroundType PageTemplateSettings::getBackgroundType()
{
	XOJ_CHECK_TYPE(PageTemplateSettings);

	return backgroundType;
}

// TODO !!!!!!!
PageType PageTemplateSettings::getPageInsertType()
{
	XOJ_CHECK_TYPE(PageTemplateSettings);
//
//	if (copyLastPageSettings)
//	{
//		return PAGE_INSERT_TYPE_COPY;
//	}
//
//	switch (backgroundType)
//	{
//	case BACKGROUND_TYPE_NONE:
//		return PAGE_INSERT_TYPE_PLAIN;
//	case BACKGROUND_TYPE_PDF:
//		return PAGE_INSERT_TYPE_PDF_BACKGROUND;
//	case BACKGROUND_TYPE_LINED:
//		return PAGE_INSERT_TYPE_LINED;
//	case BACKGROUND_TYPE_RULED:
//		return PAGE_INSERT_TYPE_RULED;
//	case BACKGROUND_TYPE_GRAPH:
//		return PAGE_INSERT_TYPE_GRAPH;
//	default:
//		return PAGE_INSERT_TYPE_PLAIN;
//	}

	PageType pt;
	return pt;
}

void PageTemplateSettings::setBackgroundType(BackgroundType backgroundType)
{
	XOJ_CHECK_TYPE(PageTemplateSettings);

	this->backgroundType = backgroundType;
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
			if (value == "plain")
			{
				backgroundType = BACKGROUND_TYPE_NONE;
			}
			else if (value == "lined")
			{
				backgroundType = BACKGROUND_TYPE_LINED;
			}
			else if (value == "ruled")
			{
				backgroundType = BACKGROUND_TYPE_RULED;
			}
			else if (value == "graph")
			{
				backgroundType = BACKGROUND_TYPE_GRAPH;
			}
		}
	}

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
	str += string("backgroundType=") + SaveHandler::getSolidBgStr(backgroundType) + "\n";

	char buffer[64];
	sprintf(buffer, "#%06x", this->backgroundColor);
	str += string("backgroundColor=") + buffer + "\n";

	return str;
}
