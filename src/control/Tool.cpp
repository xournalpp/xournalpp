#include "Tool.h"

Tool::Tool(string name, ToolType type, int color, int capabilities, double* thickness)
{
	this->name = name;
	this->type = type;
	this->thickness = thickness;

	this->capabilities = capabilities;

	setColor(color);
}

Tool::~Tool()
{
	delete[] this->thickness;
	this->thickness = nullptr;
}

string Tool::getName()
{
	return this->name;
}

void Tool::setCapability(int capability, bool enabled)
{
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
	return (this->capabilities & cap) != 0;
}

double Tool::getThickness(ToolSize size)
{
	return this->thickness[size - TOOL_SIZE_VERY_FINE];
}

