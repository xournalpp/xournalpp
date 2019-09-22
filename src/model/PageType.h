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

enum class PageTypeFormat
{
	Plain,
	Ruled,
	Lined,
	Staves,
	Graph,
	Dotted,
	Pdf,
	Image,
	Copy
};

class PageType
{
public:
	PageType();
	explicit PageType(PageTypeFormat format);
	PageType(const PageType& other);
	~PageType();

private:
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
	PageTypeFormat format;

	/**
	 * Arguments for the format
	 */
	string config;
};
