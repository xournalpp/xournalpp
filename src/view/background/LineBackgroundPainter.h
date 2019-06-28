/*
 * Xournal++
 *
 * Draws lined backgrounds of all sorts
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "BaseBackgroundPainter.h"

#include "LineBackgroundType.h"
#include <XournalType.h>

class LineBackgroundPainter : public BaseBackgroundPainter
{
public:
	LineBackgroundPainter(LineBackgroundType lineType, bool verticalLine);
	virtual ~LineBackgroundPainter();

public:
	virtual void paint();

	/**
	 * Reset all used configuration values
	 */
	virtual void resetConfig();


	void paintBackgroundLined();
	void paintBackgroundStaves();
	void paintBackgroundVerticalLine();

private:
	XOJ_TYPE_ATTRIB;

	LineBackgroundType lineType;
	bool verticalLine;
};
