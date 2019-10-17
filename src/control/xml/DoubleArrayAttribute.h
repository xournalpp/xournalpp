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

class DoubleArrayAttribute : public XMLAttribute
{
public:
	DoubleArrayAttribute(const char* name, double* values, int count);
	virtual ~DoubleArrayAttribute();

public:
	virtual void writeOut(OutputStream* out);

private:
	double* values;
	int count;
};
