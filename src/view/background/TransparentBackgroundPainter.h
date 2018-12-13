/*
 * Xournal++
 *
 * Draws transparent background
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "BaseBackgroundPainter.h"

#include <XournalType.h>

class TransparentBackgroundPainter : public BaseBackgroundPainter
{
public:
	TransparentBackgroundPainter();
	virtual ~TransparentBackgroundPainter();

public:
	virtual void paint();
	void paintBackgroundTransparent();

	/**
	 * Reset all used configuration values
	 */
	virtual void resetConfig();

private:
	XOJ_TYPE_ATTRIB;
};
