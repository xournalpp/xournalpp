/*
 * Xournal++
 *
 * Paper background type
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <XournalType.h>

class PageType
{
public:
	PageType();
	PageType(string format);
	PageType(const PageType& other);
	~PageType();

private:
	XOJ_TYPE_ATTRIB;

public:
	/**
	 * Compare Operator
	 */
	bool operator ==(const PageType& other) const;

	/**
	 * PDF background
	 */
	bool isPdfPage();

	/**
	 * Image Background
	 */
	bool isImagePage();

	/**
	 * Special background
	 */
	bool isSpecial();

public:
	/**
	 * Base format
	 */
	string format;

	/**
	 * Arguments for the format
	 */
	string config;
};
