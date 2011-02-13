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

class DoubleAttribute: public Attribute {
public:
	DoubleAttribute(const char * name, double value);
	virtual ~DoubleAttribute();

public:
	virtual void writeOut(OutputStream * out);

private:
	double value;
};


#endif /* __DOUBLEATTRIBUTE_H__ */
