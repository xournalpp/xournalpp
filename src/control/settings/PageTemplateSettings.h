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

#include "control/Tool.h"
#include "model/BackgroundType.h"

#include <XournalType.h>

#include <string>

using std::string;

class PageType;

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

	double getPageWidth();
	void setPageWidth(double pageWidth);

	double getPageHeight();
	void setPageHeight(double pageHeight);

	int getBackgroundColor();
	void setBackgroundColor(int backgroundColor);

	BackgroundType getBackgroundType();
	PageType getPageInsertType();
	void setBackgroundType(BackgroundType backgroundType);

private:
	XOJ_TYPE_ATTRIB;

	/**
	 * Copy the settings from the last page
	 */
	bool copyLastPageSettings;

	double pageWidth;
	double pageHeight;

	/**
	 * Background color in RGB
	 */
	int backgroundColor;

	/**
	 * Background type
	 */
	BackgroundType backgroundType;
};
