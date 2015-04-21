/*
 * Xournal++
 *
 * A text element
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "Element.h"
#include <StringUtils.h>
#include "Font.h"
#include <gtk/gtk.h>

class Text : public Element
{
public:
	Text();
	virtual ~Text();

public:
	void setFont(XojFont& font);
	XojFont& getFont();

	string getText();
	void setText(string text);

	void setWidth(double width);
	void setHeight(double height);

	void setInEditing(bool inEditing);
	bool isInEditing();

	virtual void scale(double x0, double y0, double fx, double fy);

	virtual bool rescaleOnlyAspectRatio();

	/**
	 * @overwrite
	 */
	virtual Element* clone();

public:
	// Serialize interface
	void serialize(ObjectOutputStream& out);
	void readSerialized(ObjectInputStream& in) throw (InputStreamException);

protected:
	virtual void calcSize();

private:
	XOJ_TYPE_ATTRIB;

	XojFont font;

	string text;

	bool inEditing;
};
