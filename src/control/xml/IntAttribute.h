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

class IntAttribute : public XMLAttribute
{
public:
	IntAttribute(const char* name, int value);
	virtual ~IntAttribute();

public:
	virtual void writeOut(OutputStream* out);

private:
	int value;
};
