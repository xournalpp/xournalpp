/*
 * Xournal++
 *
 * A page range for PDF export etc. (e.g. 1-2,5,7)
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */
// TODO: AA: type check

#ifndef __PAGERANGE_H__
#define __PAGERANGE_H__

#include <glib.h>

class PageRangeEntry {
public:
	PageRangeEntry(int first, int last);

	int first;
	int last;
};

class PageRange {
public:
	PageRange();
	virtual ~PageRange();

public:
	static bool isSeparator(char c);
	static GList * parse(const char * str);
};

#endif /* __PAGERANGE_H__ */
