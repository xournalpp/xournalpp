/*
 * Xournal++
 *
 * Page template settings handler
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <XournalType.h>

#include <string>

using std::string;

class PageTemplateSettings
{
public:
	PageTemplateSettings();
	virtual ~PageTemplateSettings();

public:
	/**
	 * Parse a template string
	 *
	 * @return true if valid
	 */
	bool parse(string tpl);

	/**
	 * Convert to a parsable string
	 */
	string toString();

	bool isCopyLastPageSettings();
	void setCopyLastPageSettings(bool copyLastPageSettings);

	int getPageWidth();
	void setPageWidth(int pageWidth);

	int getPageHeight();
	void setPageHeight(int pageHeight);

	string getSizeUnit();
	void setSizeUnit(string sizeUnit);

	int getBackgroundColor();
	void setBackgroundColor(int backgroundColor);

private:
	XOJ_TYPE_ATTRIB;

	/**
	 * Copy the settings from the last page
	 */
	bool copyLastPageSettings;

	int pageWidth;
	int pageHeight;

	/**
	 * Background color in RGB
	 */
	int backgroundColor;

	/**
	 * Unit, see XOJ_UNITS
	 */
	string sizeUnit;
};
