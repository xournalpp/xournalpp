#include "PageRange.h"
#include <ctype.h>
#include <stdlib.h>

PageRangeEntry::PageRangeEntry(int first, int last) {
	this->first = first;
	this->last = last;
}

PageRange::PageRange() {
}

PageRange::~PageRange() {
}

bool PageRange::isSeparator(char c) {
	return (c == ',' || c == ';' || c == ':');
}

GList * PageRange::parse(const char * str) {
	GList * data = NULL;

	if (*str == 0) {
		return NULL;
	}

	int start, end;
	char * next = NULL;
	const char * p = str;
	while (*p) {
		while (isspace(*p)) {
			p++;
		}

		if (*p == '-') {
			// a half-open range like -2
			start = 1;
		} else {
			start = (int) strtol(p, &next, 10);
			if (start < 1) {
				start = 1;
			}
			p = next;
		}

		end = start;

		while (isspace(*p)) {
			p++;
		}

		if (*p == '-') {
			p++;
			end = (int) strtol(p, &next, 10);
			if (next == p) { // a half-open range like 2-
				end = 0;
			} else if (end < start) {
				end = start;
			}
		}

		data = g_list_append(data, new PageRangeEntry(start - 1, end - 1));

		// Skip until end or separator
		while (*p && !isSeparator(*p)) {
			p++;
		}

		// if not at end, skip separator
		if (*p) {
			p++;
		}
	}

	return data;
}
