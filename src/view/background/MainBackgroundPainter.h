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
#include <string>
using std::map;
using std::string;

class BaseBackgroundPainter;

class MainBackgroundPainter
{
public:
	MainBackgroundPainter();
	virtual ~MainBackgroundPainter();

public:
	virtual void paint(PageType pt, cairo_t* cr, PageRef page);

private:
	XOJ_TYPE_ATTRIB;

	map<string, BaseBackgroundPainter*> painter;
	BaseBackgroundPainter* defaultPainter;
};
