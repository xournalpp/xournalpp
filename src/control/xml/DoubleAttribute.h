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

#ifndef __DOUBLEATTRIBUTE_H__
#define __DOUBLEATTRIBUTE_H__

#include "Attribute.h"
#include <XournalType.h>

class DoubleAttribute: public Attribute {
public:
	DoubleAttribute(const char * name, double value);
	virtual ~DoubleAttribute();

public:
	virtual void writeOut(OutputStream * out);

private:
	XOJ_TYPE_ATTRIB;

	double value;
};


#endif /* __DOUBLEATTRIBUTE_H__ */
