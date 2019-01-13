#include "LastSelectedTool.h"

#include "Tool.h"

LastSelectedTool::LastSelectedTool(Tool* tool)
 : tool(tool)
{
	XOJ_INIT_TYPE(LastSelectedTool);

	this->applyFrom(tool);
}

LastSelectedTool::~LastSelectedTool()
{
	XOJ_CHECK_TYPE(LastSelectedTool);

	XOJ_RELEASE_TYPE(LastSelectedTool);
}

/**
 * Restore the tool config and return it
 */
Tool* LastSelectedTool::restoreAndGet()
{
	XOJ_CHECK_TYPE(LastSelectedTool);

	tool->applyFrom(this);

	return tool;
}
