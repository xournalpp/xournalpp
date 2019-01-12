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

class Stroke;

class StrokeStyle
{
private:
	StrokeStyle();
	virtual ~StrokeStyle();

public:
	static void parseStyle(Stroke* stroke, const char* style);
	static bool parseStyle(const char* style, const double*& dashes, int& count);
	static string formatStyle(const double* dashes, int count);

public:
};

