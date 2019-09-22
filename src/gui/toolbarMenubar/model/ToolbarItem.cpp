#include "ToolbarItem.h"

int ToolbarItem::sid = 0;

ToolbarItem::ToolbarItem(string name)
{
	this->name = name;
	this->id = ToolbarItem::sid++;

	if (ToolbarItem::sid < 0)
	{
		ToolbarItem::sid = 0;
	}
}

ToolbarItem::ToolbarItem(const ToolbarItem& item)
{
	this->id = item.id;
	this->name = item.name;
}

ToolbarItem::ToolbarItem()
{
	this->name = "";
	this->id = -100;
}

ToolbarItem::~ToolbarItem()
{
}

string ToolbarItem::getName()
{
	return this->name;
}

bool ToolbarItem::operator==(ToolbarItem& other)
{
	return this->name == other.name;
}

int ToolbarItem::getId()
{
	return this->id;
}
