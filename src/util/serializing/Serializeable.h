/*
 * Xournal++
 *
 * Serializeable interface
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __SERIALIZEABLE_H__
#define __SERIALIZEABLE_H__

#include "InputStreamException.h"

class ObjectOutputStream;
class ObjectInputStream;

extern const char* XML_VERSION_STR;

class Serializeable
{
public:
	virtual void serialize(ObjectOutputStream& out) = 0;
	virtual void readSerialized(ObjectInputStream& in) throw (
			InputStreamException) = 0;
};

#endif /* __SERIALIZEABLE_H__ */
