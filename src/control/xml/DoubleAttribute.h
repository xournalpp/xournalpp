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
#include <XournalType.h>

class DoubleAttribute : public Attribute
{
public:
	DoubleAttribute(const char* name, double value);
	virtual ~DoubleAttribute();

public:
	virtual void writeOut(OutputStream* out);

private:
	XOJ_TYPE_ATTRIB;

	double value;
};
