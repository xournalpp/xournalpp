/*
 * Xournal++
 *
 * A stroke which is temporary used if you erase a part
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef ERASEABLESTROKE_H_
#define ERASEABLESTROKE_H_

#include "../Point.h"
#include <XournalType.h>
#include <gtk/gtk.h>

class Stroke;
class EraseableStrokePart;
class PartList;
class Range;

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

	void draw(cairo_t* cr, double x, double y, double width, double height);

private:
	void erase(double x, double y, double halfEraserSize, EraseableStrokePart* part,
			PartList* list);
	bool erasePart(double x, double y, double halfEraserSize,
				EraseableStrokePart* part, PartList* list, bool* deleteStrokeAfter);

	void addRepaintRect(double x, double y, double width, double height);

private:
	XOJ_TYPE_ATTRIB;


	GMutex partLock;
	PartList * parts;

	Range* repaintRect;

	Stroke* stroke;
};

#endif /* ERASEABLESTROKE_H_ */
