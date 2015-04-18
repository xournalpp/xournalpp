/*
 * Xournal++
 *
 * Serializeable interface
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv3
 */

#pragma once

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
