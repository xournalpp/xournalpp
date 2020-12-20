/*
 * Xournal++
 *
 * Serializeable interface
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "InputStreamException.h"

class ObjectInputStream;
class ObjectOutputStream;

extern const char* XML_VERSION_STR;

class Serializeable {
public:
    virtual void serialize(ObjectOutputStream& out) = 0;
    virtual void readSerialized(ObjectInputStream& in) = 0;

    virtual ~Serializeable() = default;
};
