/*
 * Xournal++
 *
 * Draws dotted background
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "BaseBackgroundPainter.h"

#include <XournalType.h>

class DottedBackgroundPainter : public BaseBackgroundPainter
{
public:
	DottedBackgroundPainter();
	virtual ~DottedBackgroundPainter();

public:
	virtual void paint();
	void paintBackgroundDotted();

	/**
	 * Reset all used configuration values
	 */
	virtual void resetConfig();

private:
	};
