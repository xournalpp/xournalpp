/*
 * Xournal++
 *
 * An element on the Document
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __ELEMENT_H__
#define __ELEMENT_H__

#include <gtk/gtk.h>
#include "../util/Util.h"
#include "../util/Serializeable.h"

enum ElementType {
	ELEMENT_STROKE = 1, ELEMENT_IMAGE, ELEMENT_TEXT
};

class ShapeContainer {
public:
	virtual bool contains(double x, double y) = 0;
};

class Element: public MemoryCheckObject, public Serializeable {
protected:
	Element(ElementType type);

public:
	virtual ~Element();

	ElementType getType() const;

	void setX(double x);
	void setY(double y);
	double getX();
	double getY();

	virtual void move(double dx, double dy);
	virtual void finalizeMove();

	void setColor(int color);
	int getColor() const;

	double getElementWidth();
	double getElementHeight();

	virtual bool intersectsArea(const GdkRectangle * src);

	virtual bool isInSelection(ShapeContainer * container);
protected:
	// The position on the screen
	double x;
	double y;

	virtual void calcSize() = 0;

	double width;
	double height;

	void serializeElement(ObjectOutputStream & out);
	void readSerializedElement(ObjectInputStream & in) throw (InputStreamException);

protected:
	// If the size has been calculated
	bool sizeCalculated;

private:
	ElementType type;

	// The color in RGB format
	int color;
};

#endif /* __ELEMENT_H__ */
