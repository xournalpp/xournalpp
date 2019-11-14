#include "LastSelectedTool.h"

#include "Tool.h"

LastSelectedTool::LastSelectedTool(Tool* tool)
 : tool(tool)
{
	this->applyFrom(tool);
}

LastSelectedTool::~LastSelectedTool() = default;

/**
 * Restore the tool config and return it
 */
auto LastSelectedTool::restoreAndGet() -> Tool*
{
	tool->applyFrom(this);

	return tool;
}
