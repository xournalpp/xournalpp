#include "LastSelectedTool.h"


LastSelectedTool::LastSelectedTool(Tool* tool)
 : tool(tool)
{
	XOJ_INIT_TYPE(LastSelectedTool);

	this->color = tool->getColor();
	this->size = tool->getSize();
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

	tool->setColor(this->color);
	tool->setSize(this->size);

	return tool;
}
