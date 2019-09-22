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

#include <XournalType.h>

class LineBackgroundPainter : public BaseBackgroundPainter
{
public:
	LineBackgroundPainter(bool verticalLine);
	virtual ~LineBackgroundPainter();

public:
	virtual void paint();

	/**
	 * Reset all used configuration values
	 */
	virtual void resetConfig();


	void paintBackgroundRuled();
	void paintBackgroundVerticalLine();

private:
	bool verticalLine;
};
