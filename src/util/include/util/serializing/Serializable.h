/*
 * Xournal++
 *
 * Serializable interface
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

const static char* const XML_VERSION_STR = "XojStrm1:";

class Serializable {
public:
    virtual void serialize(ObjectOutputStream& out) const = 0;
    virtual void readSerialized(ObjectInputStream& in) = 0;

    virtual ~Serializable() = default;
};
