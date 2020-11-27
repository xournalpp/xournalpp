/*
 * Xournal++
 *
 * A page range for PDF export etc. (e.g. 1-2,5,7)
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>
#include <vector>

#include "XournalType.h"

class PageRangeEntry {
public:
    PageRangeEntry(int first, int last);
    virtual ~PageRangeEntry();

public:
    int getLast() const;
    int getFirst() const;

private:
    int first;
    int last;
};

typedef vector<PageRangeEntry*> PageRangeVector;

class PageRange {
private:
    PageRange();
    virtual ~PageRange();

public:
    static bool isSeparator(char c);
    static PageRangeVector parse(const char* str, int pageCount);
};
