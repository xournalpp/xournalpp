/*
 * Xournal++
 *
 * A page range for PDF export etc. (e.g. 1-2,5,7)
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv3
 */

#ifndef __PAGERANGE_H__
#define __PAGERANGE_H__

#include <XournalType.h>
#include <vector>
using std::vector;

class PageRangeEntry
{
public:
	PageRangeEntry(int first, int last);
	virtual ~PageRangeEntry();

public:
	int getLast();
	int getFirst();

private:
	XOJ_TYPE_ATTRIB;

	int first;
	int last;
};

typedef vector<PageRangeEntry*> PageRangeVector;

class PageRange
{
private:
	PageRange();
	virtual ~PageRange();

public:
	static bool isSeparator(char c);
	static PageRangeVector parse(const char* str);
};

#endif /* __PAGERANGE_H__ */
