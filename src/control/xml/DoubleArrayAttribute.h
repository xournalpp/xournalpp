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

#ifndef __DOUBLEARRAYATTRIBUTE_H__
#define __DOUBLEARRAYATTRIBUTE_H__

#include "Attribute.h"
#include "../../util/XournalType.h"

class DoubleArrayAttribute: public Attribute {
public:
	DoubleArrayAttribute(const char * name, double * values, int count);
	~DoubleArrayAttribute();

public:
	virtual void writeOut(OutputStream * out);

private:
	XOJ_TYPE_ATTRIB;


	double * values;
	int count;
};


#endif /* __DOUBLEARRAYATTRIBUTE_H__ */
