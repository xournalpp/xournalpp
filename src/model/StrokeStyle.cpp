#include "StrokeStyle.h"

#include "model/Stroke.h"

StrokeStyle::StrokeStyle()
{
}

StrokeStyle::~StrokeStyle()
{
}

const double dashLinePattern[] = { 6, 3 };
const double dashDotLinePattern[] = { 6, 3, 0.5, 3 };
const double dotLinePattern[] = { 0.5, 3 };

#define PARSE_STYLE(name, def) \
	if (strcmp(style, name) == 0) \
	{ \
		dashes = def; \
		count = sizeof(def) / sizeof(def[0]); \
		return true; \
	}


void StrokeStyle::parseStyle(Stroke* stroke, const char* style)
{
	const double* dashes = NULL;
	int dashCount = 0;
	if (StrokeStyle::parseStyle(style, dashes, dashCount))
	{
		stroke->setDashes(dashes, dashCount);
		return;
	}

	if (strncmp("cust: ", style, 6) != 0)
	{
		return;
	}

	vector<double> dash;

	const char* widths = style + 6;
	while (*widths != 0)
	{
		char* tmpptr = NULL;
		double val = g_ascii_strtod(widths, &tmpptr);
		if (tmpptr == widths)
		{
			break;
		}
		widths = tmpptr;
		dash.push_back(val);
	}

	if (dash.size() == 0)
	{
		return;
	}

	double* dashesArr = new double[dash.size()];
	for (int i = 0; i < (int)dash.size(); i++)
	{
		dashesArr[i] = dash[i];
	}
	stroke->setDashes(dashesArr, (int)dash.size());
	delete[] dashes;
}

bool StrokeStyle::parseStyle(const char* style, const double*& dashes, int& count)
{
	PARSE_STYLE("dash", dashLinePattern);
	PARSE_STYLE("dashdot", dashDotLinePattern);
	PARSE_STYLE("dot", dotLinePattern);

	return false;
}

#define FORMAT_STYLE(name, def) \
	if (count == (sizeof(def) / sizeof(def[0])) && memcmp(dashes, def, count) == 0) \
	{ \
		return name; \
	}

string StrokeStyle::formatStyle(const double* dashes, int count)
{
	FORMAT_STYLE("dash", dashLinePattern);
	FORMAT_STYLE("dashdot", dashDotLinePattern);
	FORMAT_STYLE("dot", dotLinePattern);

	string custom = "cust:";

	for (int i = 0; i < count; i++)
	{
		custom += " ";
		char* str = g_strdup_printf("%0.2lf", dashes[i]);
		custom += str;
		g_free(str);
	}

	return custom;
}

