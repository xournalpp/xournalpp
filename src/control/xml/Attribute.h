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

#ifndef __ATTRIBUTE_H__
#define __ATTRIBUTE_H__

#include "../../util/MemoryCheck.h"
#include "../../util/OutputStream.h"

class Attribute: public MemoryCheckObject {
public:
	Attribute(const char * name);
	virtual ~Attribute();

public:
	virtual void writeOut(OutputStream * out) = 0;

	const char * getName();

private:
	char * name;
};

#endif /* __ATTRIBUTE_H__ */
