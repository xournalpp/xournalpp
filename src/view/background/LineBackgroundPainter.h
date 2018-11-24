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
	LineBackgroundPainter(bool ruled);
	virtual ~LineBackgroundPainter();

public:
	virtual void paint();
	void paintBackgroundRuled();
	void paintBackgroundLined();

private:
	XOJ_TYPE_ATTRIB;

	bool ruled;
};
