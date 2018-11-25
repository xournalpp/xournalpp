/*
 * Xournal++
 *
 * Base class for Background paints (This class fills the background)
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "BackgroundConfig.h"

#include "model/PageRef.h"

#include <XournalType.h>

#include <gtk/gtk.h>

#include <string>
using std::string;

class BaseBackgroundPainter
{
public:
	BaseBackgroundPainter();
	virtual ~BaseBackgroundPainter();

public:
	virtual void paint(cairo_t* cr, PageRef page, BackgroundConfig* config);
	virtual void paint();

	/**
	 * Reset all used configuration values
	 */
	virtual void resetConfig();

	/**
	 * Set a factor to draw the lines bolder, for previews
	 */
	void setLineWidthFactor(double factor);

protected:
	void paintBackgroundColor();

private:
	XOJ_TYPE_ATTRIB;

protected:
	BackgroundConfig* config;
	PageRef page;
	cairo_t* cr;

	double width;
	double height;

	// Drawing attributes
	// ParserKey=Value
protected:
	/**
	 * c1=XXXXXX
	 * e.g. FF0000 for Red
	 */
	int foregroundColor1;

	/**
	 * c2=XXXXXX
	 * e.g. FF0000 for Red
	 */
	int foregroundColor2;

	/**
	 * lw=1.23
	 */
	double lineWidth;

	/**
	 * r1=1.23
	 */
	double drawRaster1;

	/**
	 * m1=40
	 */
	double margin1;

	/**
	 * rm=1
	 * Round margin = 0 => No rounding
	 * Round margin = 1 => Round to next grid etc.
	 */
	int roundMargin;

	/**
	 * Line width factor, to use to draw Previews
	 */
	double lineWidthFactor;
};
