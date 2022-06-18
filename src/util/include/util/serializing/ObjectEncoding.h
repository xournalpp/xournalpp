/*
 * Xournal++
 *
 * Encoding for serialized streams
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <glib.h>  // for GString


class ObjectEncoding {
public:
    ObjectEncoding();
    virtual ~ObjectEncoding();

public:
    void addStr(const char* str) const;
    virtual void addData(const void* data, int len) = 0;

    GString* getData();

public:
    GString* data;
};
