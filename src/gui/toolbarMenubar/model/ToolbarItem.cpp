#include "ToolbarItem.h"

int ToolbarItem::sid = 0;

ToolbarItem::ToolbarItem(string name)
{
	XOJ_INIT_TYPE(ToolbarItem);

	this->name = name;
	this->id = ToolbarItem::sid++;

	if (ToolbarItem::sid < 0)
	{
		ToolbarItem::sid = 0;
	}
}

ToolbarItem::ToolbarItem(const ToolbarItem& item)
{
	XOJ_INIT_TYPE(ToolbarItem);

	this->id = item.id;
	this->name = item.name;
	this->sid = item.sid;
}

ToolbarItem::ToolbarItem()
{
	XOJ_INIT_TYPE(ToolbarItem);

	this->name = "";
	this->id = -100;
}

ToolbarItem::~ToolbarItem()
{
	XOJ_RELEASE_TYPE(ToolbarItem);
}

string ToolbarItem::getName()
{
	XOJ_CHECK_TYPE(ToolbarItem);

	return this->name;
}

bool ToolbarItem::operator==(ToolbarItem& other)
{
	XOJ_CHECK_TYPE(ToolbarItem);

	return this->name == other.name;
}

int ToolbarItem::getId()
{
	XOJ_CHECK_TYPE(ToolbarItem);

	return this->id;
}
