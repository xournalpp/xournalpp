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

#include "model/PageRef.h"

#include <XournalType.h>

#include <gtk/gtk.h>

#include <map>
#include <string>
using std::map;
using std::string;

class BaseBackgroundPainter
{
public:
	BaseBackgroundPainter();
	virtual ~BaseBackgroundPainter();

public:
	virtual void paint(cairo_t* cr, PageRef page, map<string, string>& config);
	virtual void paint();

protected:
	void paintBackgroundColor();

private:
	XOJ_TYPE_ATTRIB;

protected:
	map<string, string> config;
	PageRef page;
	cairo_t* cr;

	double width;
	double height;
};
