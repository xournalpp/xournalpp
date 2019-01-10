/*
 * Xournal++
 *
 * Definition for StrokeStyles
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <XournalType.h>

class StrokeStyle
{
private:
	StrokeStyle();
	virtual ~StrokeStyle();

public:
	static bool parseStyle(const char* style, const double*& dashes, int& count);

public:
};

