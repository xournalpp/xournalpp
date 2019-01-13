#include "Tool.h"

Tool::Tool(string name, ToolType type, int color, int capabilities, double* thickness)
{
	XOJ_INIT_TYPE(Tool);

	this->name = name;
	this->type = type;
	this->thickness = thickness;

	this->capabilities = capabilities;

	setColor(color);
}

Tool::~Tool()
{
	XOJ_CHECK_TYPE(Tool);

	delete[] this->thickness;
	this->thickness = NULL;

	XOJ_RELEASE_TYPE(Tool);
}

string Tool::getName()
{
	XOJ_CHECK_TYPE(Tool);

	return this->name;
}

void Tool::setCapability(int capability, bool enabled)
{
	XOJ_CHECK_TYPE(Tool);

	if (enabled)
	{
		this->capabilities |= capability;
	}
	else
	{
		this->capabilities &= ~capability;
	}
}

bool Tool::hasCapability(ToolCapabilities cap)
{
	XOJ_CHECK_TYPE(Tool);

	return (this->capabilities & cap) != 0;
}

double Tool::getThickness(ToolSize size)
{
	return this->thickness[size - TOOL_SIZE_VERY_FINE];
}

