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
#include <XournalType.h>

class DoubleAttribute : public XMLAttribute
{
public:
	DoubleAttribute(const char* name, double value);
	virtual ~DoubleAttribute();

public:
	virtual void writeOut(OutputStream* out);

private:
	double value;
};
