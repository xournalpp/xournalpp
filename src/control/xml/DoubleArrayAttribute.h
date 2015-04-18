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

class DoubleArrayAttribute : public Attribute
{
public:
	DoubleArrayAttribute(const char* name, double* values, int count);
	virtual ~DoubleArrayAttribute();

public:
	virtual void writeOut(OutputStream* out);

private:
	XOJ_TYPE_ATTRIB;


	double* values;
	int count;
};
