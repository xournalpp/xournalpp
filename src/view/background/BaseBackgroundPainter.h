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
	 * b1=XXXXXX
	 * e.g. FF0000 for Red
	 */
	int backgroundColor1;

	/**
	 * b2=XXXXXX
	 * e.g. FF0000 for Red
	 */
	int backgroundColor2;

};
