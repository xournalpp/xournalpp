#include "PageRange.h"

#include <cctype>
#include <cstdlib>

PageRangeEntry::PageRangeEntry(int first, int last)
{
	this->first = first;
	this->last = last;
}

PageRangeEntry::~PageRangeEntry() = default;

auto PageRangeEntry::getLast() -> int
{
	return this->last;
}

auto PageRangeEntry::getFirst() -> int
{
	return this->first;
}

auto PageRange::isSeparator(char c) -> bool
{
	return (c == ',' || c == ';' || c == ':');
}

auto PageRange::parse(const char* str) -> PageRangeVector
{
	PageRangeVector data;

	if (*str == 0)
	{
		return data;
	}

	int start, end;
	char* next = nullptr;
	const char* p = str;
	while (*p)
	{
		while (isspace(*p))
		{
			p++;
		}

		if (*p == '-')
		{
			// a half-open range like -2
			start = 1;
		}
		else
		{
			start = (int) strtol(p, &next, 10);
			if (start < 1)
			{
				start = 1;
			}
			p = next;
		}

		end = start;

		while (isspace(*p))
		{
			p++;
		}

		if (*p == '-')
		{
			p++;
			end = (int) strtol(p, &next, 10);
			if (next == p) // a half-open range like 2-
			{
				end = 0;
			}
			else if (end < start)
			{
				end = start;
			}
		}

		data.push_back(new PageRangeEntry(start - 1, end - 1));

		// Skip until end or separator
		while (*p && !isSeparator(*p))
		{
			p++;
		}

		// if not at end, skip separator
		if (*p)
		{
			p++;
		}
	}

	return data;
}
