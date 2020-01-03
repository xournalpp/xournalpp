/*
 * Xournal++
 *
 * A list with parts
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>
#include <vector>

#include <gtk/gtk.h>

#include "XournalType.h"

class EraseableStrokePart;

class PartList {
public:
    PartList();
    virtual ~PartList();

private:
    PartList(const PartList& list);
    void operator=(const PartList& list);

public:
    void add(EraseableStrokePart* part);
    PartList* clone();

private:
    GList* data = nullptr;

    friend class EraseableStroke;
};
