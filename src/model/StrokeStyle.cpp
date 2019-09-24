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
		LineStyle style; \
		style.setDashes(def, sizeof(def) / sizeof(def[0])); \
		return style; \
	}

LineStyle StrokeStyle::parseStyle(const char* style)
{
	PARSE_STYLE("dash", dashLinePattern);
	PARSE_STYLE("dashdot", dashDotLinePattern);
	PARSE_STYLE("dot", dotLinePattern);


	if (strncmp("cust: ", style, 6) != 0)
	{
		return LineStyle();
	}

	vector<double> dash;

	const char* widths = style + 6;
	while (*widths != 0)
	{
		char* tmpptr = nullptr;
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
		return LineStyle();
	}

	double* dashesArr = new double[dash.size()];
	for (int i = 0; i < (int)dash.size(); i++)
	{
		dashesArr[i] = dash[i];
	}

	LineStyle ls;
	ls.setDashes(dashesArr, (int)dash.size());
	delete[] dashesArr;

	return ls;
}

#define FORMAT_STYLE(name, def) \
	if (count == (sizeof(def) / sizeof(def[0])) && memcmp(dashes, def, count * sizeof(def[0])) == 0) \
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

string StrokeStyle::formatStyle(const LineStyle& style)
{
	const double* dashes = nullptr;
	int dashCount = 0;
	if (style.getDashes(dashes, dashCount))
	{
		return StrokeStyle::formatStyle(dashes, dashCount);
	}

	// Should not be returned, in this case the attribute is not written
	return "plain";
}

