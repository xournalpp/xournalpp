/*
 * Xournal++
 *
 * XML Writer helper class
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __INTATTRIBUTE_H__
#define __INTATTRIBUTE_H__

#include "Attribute.h"

class IntAttribute : public Attribute
{
public:
	IntAttribute(const char* name, int value);
	virtual ~IntAttribute();

public:
	virtual void writeOut(OutputStream* out);

private:
	XOJ_TYPE_ATTRIB;

	int value;
};

#endif /* __INTATTRIBUTE_H__ */
