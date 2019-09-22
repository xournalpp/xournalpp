/*
 * Xournal++
 *
 * XML Writer helper class
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "Attribute.h"

class TextAttribute : public XMLAttribute
{
public:
	TextAttribute(string name, string value);
	virtual ~TextAttribute();

public:
	virtual void writeOut(OutputStream* out);

private:
	string value;
};
