#include "StrokeStyle.h"

StrokeStyle::StrokeStyle()
{
}

StrokeStyle::~StrokeStyle()
{
}

const double dashLinePattern[] = { 6, 3 };
const double dashDotLinePattern[] = { 6, 3, 0.5, 3 };
const double dotLinePattern[] = { 0.5, 3 };


bool StrokeStyle::parseStyle(const char* style, const double*& dashes, int& count)
{
	if (strcmp(style, "dash") == 0)
	{
		dashes = dashLinePattern;
		count = sizeof(dashLinePattern) / sizeof(dashLinePattern[0]);
		return true;
	}
	else if (strcmp(style, "dashdot") == 0)
	{
		dashes = dashDotLinePattern;
		count = sizeof(dashDotLinePattern) / sizeof(dashDotLinePattern[0]);
		return true;
	}
	else if (strcmp(style, "dot") == 0)
	{
		dashes = dotLinePattern;
		count = sizeof(dotLinePattern) / sizeof(dotLinePattern[0]);
		return true;
	}

	return false;
}
