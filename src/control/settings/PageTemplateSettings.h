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
#include "model/PageType.h"

#include <XournalType.h>

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

	bool isCopyLastPageSize();
	void setCopyLastPageSize(bool copyLastPageSize);

	double getPageWidth();
	void setPageWidth(double pageWidth);

	double getPageHeight();
	void setPageHeight(double pageHeight);

	int getBackgroundColor();
	void setBackgroundColor(int backgroundColor);

	PageType getBackgroundType();
	PageType getPageInsertType();
	void setBackgroundType(PageType backgroundType);

private:
	/**
	 * Copy the settings from the last page
	 */
	bool copyLastPageSettings;

	/**
	 * Copy the last page size
	 */
	bool copyLastPageSize;

	double pageWidth;
	double pageHeight;

	/**
	 * Background color in RGB
	 */
	int backgroundColor;

	/**
	 * Background type
	 */
	PageType backgroundType;
};
