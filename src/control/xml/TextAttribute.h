/*
 * Xournal++
 *
 * XML Writer helper class
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv3
 */

#pragma once

#include "Attribute.h"

class TextAttribute : public Attribute
{
public:
	TextAttribute(const char* name, const char* value);
	virtual ~TextAttribute();

public:
	virtual void writeOut(OutputStream* out);

private:
	XOJ_TYPE_ATTRIB;

	char* value;
};
