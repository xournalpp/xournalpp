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
#include "String.h"
#include "XFont.h"
#include <gtk/gtk.h>

class Text: public Element {
public:
	Text();
	virtual ~Text();

public:
	void setFont(PangoFontDescription * font);
	XFont & getFont();

	String getText();
	void setText(String text);

	virtual bool isInSelection(ShapeContainer * container);

	void setWidth(double width);
	void setHeight(double height);

	void setInEditing(bool inEditing);
	bool isInEditing();
protected:
	virtual void calcSize();
private:
	XFont font;

	String text;

	bool inEditing;
};

#endif /* TEXT_H_ */
