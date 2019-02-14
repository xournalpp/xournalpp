// ** THIS FILE IS GENERATED **
// ** use generateConvert.php to update this file **



#include "../ActionGroup.enum.h"

#include <string>
using std::string;
#include <glib.h>


// ** This needs to be copied to the header
ActionGroup ActionGroup_fromString(string value);
string ActionGroup_toString(ActionGroup value);


ActionGroup ActionGroup_fromString(string value)
{
	if (value == "GROUP_NOGROUP")
	{
		return GROUP_NOGROUP;
	}

	if (value == "GROUP_TOOL")
	{
		return GROUP_TOOL;
	}

	if (value == "GROUP_COLOR")
	{
		return GROUP_COLOR;
	}

	if (value == "GROUP_SIZE")
	{
		return GROUP_SIZE;
	}

	if (value == "GROUP_ERASER_MODE")
	{
		return GROUP_ERASER_MODE;
	}

	if (value == "GROUP_ERASER_SIZE")
	{
		return GROUP_ERASER_SIZE;
	}

	if (value == "GROUP_PEN_SIZE")
	{
		return GROUP_PEN_SIZE;
	}

	if (value == "GROUP_PEN_FILL")
	{
		return GROUP_PEN_FILL;
	}

	if (value == "GROUP_HILIGHTER_SIZE")
	{
		return GROUP_HILIGHTER_SIZE;
	}

	if (value == "GROUP_HILIGHTER_FILL")
	{
		return GROUP_HILIGHTER_FILL;
	}

	if (value == "GROUP_TOGGLE_GROUP")
	{
		return GROUP_TOGGLE_GROUP;
	}

	if (value == "GROUP_PAIRED_PAGES")
	{
		return GROUP_PAIRED_PAGES;
	}

	if (value == "GROUP_PRESENTATION_MODE")
	{
		return GROUP_PRESENTATION_MODE;
	}

	if (value == "GROUP_FULLSCREEN")
	{
		return GROUP_FULLSCREEN;
	}

	if (value == "GROUP_RULER")
	{
		return GROUP_RULER;
	}

	if (value == "GROUP_LINE_STYLE")
	{
		return GROUP_LINE_STYLE;
	}

	if (value == "GROUP_REC")
	{
		return GROUP_REC;
	}

	if (value == "GROUP_SNAPPING")
	{
		return GROUP_SNAPPING;
	}

	if (value == "GROUP_GRID_SNAPPING")
	{
		return GROUP_GRID_SNAPPING;
	}

	if (value == "GROUP_FILL")
	{
		return GROUP_FILL;
	}

	if (value == "GROUP_COLUMNS")
	{
		return GROUP_COLUMNS;
	}

	if (value == "GROUP_ROWS")
	{
		return GROUP_ROWS;
	}

	if (value == "GROUP_LAYOUT_HORIZONTAL")
	{
		return GROUP_LAYOUT_HORIZONTAL;
	}

	if (value == "GROUP_LAYOUT_LR")
	{
		return GROUP_LAYOUT_LR;
	}

	if (value == "GROUP_LAYOUT_TB")
	{
		return GROUP_LAYOUT_TB;
	}

	g_error("Invalid enum value for ActionGroup: «%s»", value.c_str());
	return GROUP_NOGROUP;
}



string ActionGroup_toString(ActionGroup value)
{
	if (value == GROUP_NOGROUP)
	{
		return "GROUP_NOGROUP";
	}

	if (value == GROUP_TOOL)
	{
		return "GROUP_TOOL";
	}

	if (value == GROUP_COLOR)
	{
		return "GROUP_COLOR";
	}

	if (value == GROUP_SIZE)
	{
		return "GROUP_SIZE";
	}

	if (value == GROUP_ERASER_MODE)
	{
		return "GROUP_ERASER_MODE";
	}

	if (value == GROUP_ERASER_SIZE)
	{
		return "GROUP_ERASER_SIZE";
	}

	if (value == GROUP_PEN_SIZE)
	{
		return "GROUP_PEN_SIZE";
	}

	if (value == GROUP_PEN_FILL)
	{
		return "GROUP_PEN_FILL";
	}

	if (value == GROUP_HILIGHTER_SIZE)
	{
		return "GROUP_HILIGHTER_SIZE";
	}

	if (value == GROUP_HILIGHTER_FILL)
	{
		return "GROUP_HILIGHTER_FILL";
	}

	if (value == GROUP_TOGGLE_GROUP)
	{
		return "GROUP_TOGGLE_GROUP";
	}

	if (value == GROUP_PAIRED_PAGES)
	{
		return "GROUP_PAIRED_PAGES";
	}

	if (value == GROUP_PRESENTATION_MODE)
	{
		return "GROUP_PRESENTATION_MODE";
	}

	if (value == GROUP_FULLSCREEN)
	{
		return "GROUP_FULLSCREEN";
	}

	if (value == GROUP_RULER)
	{
		return "GROUP_RULER";
	}

	if (value == GROUP_LINE_STYLE)
	{
		return "GROUP_LINE_STYLE";
	}

	if (value == GROUP_REC)
	{
		return "GROUP_REC";
	}

	if (value == GROUP_SNAPPING)
	{
		return "GROUP_SNAPPING";
	}

	if (value == GROUP_GRID_SNAPPING)
	{
		return "GROUP_GRID_SNAPPING";
	}

	if (value == GROUP_FILL)
	{
		return "GROUP_FILL";
	}

	if (value == GROUP_COLUMNS)
	{
		return "GROUP_COLUMNS";
	}

	if (value == GROUP_ROWS)
	{
		return "GROUP_ROWS";
	}

	if (value == GROUP_LAYOUT_HORIZONTAL)
	{
		return "GROUP_LAYOUT_HORIZONTAL";
	}

	if (value == GROUP_LAYOUT_LR)
	{
		return "GROUP_LAYOUT_LR";
	}

	if (value == GROUP_LAYOUT_TB)
	{
		return "GROUP_LAYOUT_TB";
	}

	g_error("Invalid enum value for ActionGroup: %i", value);
	return "";
}
