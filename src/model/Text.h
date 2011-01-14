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
protected:
	virtual void calcSize();
private:
	XojFont font;

	String text;

	bool inEditing;
};

#endif /* TEXT_H_ */
