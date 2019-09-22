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
#include "Font.h"
#include "AudioElement.h"

#include <gtk/gtk.h>

class Text : public AudioElement
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
	virtual void rotate(double x0, double y0, double xo, double yo, double th);

	virtual bool rescaleOnlyAspectRatio();

	/**
	 * @overwrite
	 */
	virtual Element* clone();

	bool intersects(double x, double y, double halfSize) override;
	bool intersects(double x, double y, double halfSize, double* gap) override;

public:
	// Serialize interface
	void serialize(ObjectOutputStream& out);
	void readSerialized(ObjectInputStream& in);

protected:
	virtual void calcSize();

private:
	XojFont font;

	string text;

	bool inEditing = false;
};
