/*
 * Xournal++
 *
 * A stroke which is temporary used if you erase a part
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "model/Point.h"
#include <XournalType.h>

#include <gtk/gtk.h>

class EraseableStrokePart;
class PartList;
class Range;
class Stroke;

class EraseableStroke
{
public:
	EraseableStroke(Stroke* stroke);
	virtual ~EraseableStroke();

public:
	/**
	 * Returns a repaint rectangle or NULL, the rectangle is own by the caller
	 */
	Range* erase(double x, double y, double halfEraserSize, Range* range = NULL);

	GList* getStroke(Stroke* original);

	void draw(cairo_t* cr);

private:
	void erase(double x, double y, double halfEraserSize, EraseableStrokePart* part, PartList* list);
	bool erasePart(double x, double y, double halfEraserSize, EraseableStrokePart* part,
				   PartList* list, bool* deleteStrokeAfter);

	void addRepaintRect(double x, double y, double width, double height);

private:
	GMutex partLock;
	PartList * parts = NULL;

	Range* repaintRect = NULL;

	Stroke* stroke = NULL;
};
