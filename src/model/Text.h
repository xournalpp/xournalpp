/*
 * Xournal++
 *
 * A text element
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __TEXT_H__
#define __TEXT_H__

#include "Element.h"
#include <String.h>
#include "Font.h"
#include <gtk/gtk.h>

class Text: public Element {
public:
	Text();
	virtual ~Text();

public:
	void setFont(XojFont & font);
	XojFont & getFont();

	String getText();
	void setText(String text);

	void setWidth(double width);
	void setHeight(double height);

	void setInEditing(bool inEditing);
	bool isInEditing();

	virtual void scale(double x0, double y0, double fx, double fy);

	virtual bool rescaleOnlyAspectRatio();

	/**
	 * @overwrite
	 */
	virtual Element * clone();

public:
	// Serialize interface
	void serialize(ObjectOutputStream & out);
	void readSerialized(ObjectInputStream & in) throw (InputStreamException);

protected:
	virtual void calcSize();

private:
	XOJ_TYPE_ATTRIB;

	XojFont font;

	String text;

	bool inEditing;
};

#endif /* TEXT_H_ */
