#include "ToolbarEntry.h"

ToolbarEntry::ToolbarEntry()
{
	XOJ_INIT_TYPE(ToolbarEntry);
}

ToolbarEntry::ToolbarEntry(const ToolbarEntry& e)
{
	XOJ_INIT_TYPE(ToolbarEntry);

	*this = e;
}

void ToolbarEntry::operator=(const ToolbarEntry& e)
{
	XOJ_CHECK_TYPE(ToolbarEntry);

	this->name = e.name;
	clearList();

	for (ToolbarItem* item : e.entries)
	{
		entries.push_back(new ToolbarItem(*item));
	}
}

ToolbarEntry::~ToolbarEntry()
{
	XOJ_CHECK_TYPE(ToolbarEntry);

	clearList();

	XOJ_RELEASE_TYPE(ToolbarEntry);
}

void ToolbarEntry::clearList()
{
	for (ToolbarItem* item : entries)
	{
		delete item;
	}
	entries.clear();
}

string ToolbarEntry::getName()
{
	XOJ_CHECK_TYPE(ToolbarEntry);

	return this->name;
}

void ToolbarEntry::setName(string name)
{
	XOJ_CHECK_TYPE(ToolbarEntry);

	this->name = name;
}

int ToolbarEntry::addItem(string item)
{
	XOJ_CHECK_TYPE(ToolbarEntry);

	ToolbarItem* it = new ToolbarItem(item);
	entries.push_back(it);

	return it->getId();
}

bool ToolbarEntry::removeItemById(int id)
{
	XOJ_CHECK_TYPE(ToolbarEntry);

	for (unsigned int i = 0; i < this->entries.size(); i++)
	{
		if (this->entries[i]->getId() == id)
		{
			delete this->entries[i];
			entries[i] = NULL;
			entries.erase(entries.begin() + i);
			return true;
		}
	}
	return false;
}

int ToolbarEntry::insertItem(string item, int position)
{
	XOJ_CHECK_TYPE(ToolbarEntry);

	ToolbarItem* it = new ToolbarItem(item);

	if (position >= entries.size()) {
		entries.push_back(it);
		return it->getId();
	}

	entries.insert(entries.begin() + position, it);
	return it->getId();
}

const ToolbarItemVector& ToolbarEntry::getItems() const
{
	XOJ_CHECK_TYPE(ToolbarEntry);

	return entries;
}
