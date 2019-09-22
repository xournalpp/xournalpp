#include "LastSelectedTool.h"

#include "Tool.h"

LastSelectedTool::LastSelectedTool(Tool* tool)
 : tool(tool)
{
	this->applyFrom(tool);
}

LastSelectedTool::~LastSelectedTool()
{
}

/**
 * Restore the tool config and return it
 */
Tool* LastSelectedTool::restoreAndGet()
{
	tool->applyFrom(this);

	return tool;
}
