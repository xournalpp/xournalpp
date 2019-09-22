/*
 * Xournal++
 *
 * Selects and configures the right Background Painter Class
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "model/PageRef.h"

#include <XournalType.h>

#include <gtk/gtk.h>

#include <map>
using std::map;

class BaseBackgroundPainter;

class MainBackgroundPainter
{
public:
	MainBackgroundPainter();
	virtual ~MainBackgroundPainter();

public:
	virtual void paint(PageType pt, cairo_t* cr, PageRef page);

	/**
	 * Set a factor to draw the lines bolder, for previews
	 */
	void setLineWidthFactor(double factor);

private:
	map<PageTypeFormat, BaseBackgroundPainter*> painter;
	BaseBackgroundPainter* defaultPainter;
};
