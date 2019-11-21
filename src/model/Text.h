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
	~Text() override;

public:
	void setFont(XojFont& font);
	XojFont& getFont();

	string getText();
	void setText(string text);

	void setWidth(double width);
	void setHeight(double height);

	void setInEditing(bool inEditing);
	bool isInEditing() const;

	void scale(double x0, double y0, double fx, double fy) override;
	void rotate(double x0, double y0, double xo, double yo, double th) override;

	bool rescaleOnlyAspectRatio() override;

	/**
	 * @overwrite
	 */
	Element* clone() override;

	bool intersects(double x, double y, double halfEraserSize) override;
	bool intersects(double x, double y, double halfEraserSize, double* gap) override;

public:
	// Serialize interface
	void serialize(ObjectOutputStream& out) override;
	void readSerialized(ObjectInputStream& in) override;

protected:
	void calcSize() override;

private:
	XojFont font;

	string text;

	bool inEditing = false;
};
