#include "ToolbarEntry.h"

ToolbarEntry::ToolbarEntry()
{
}

ToolbarEntry::ToolbarEntry(const ToolbarEntry& e)
{
	*this = e;
}

void ToolbarEntry::operator=(const ToolbarEntry& e)
{
	this->name = e.name;
	clearList();

	for (ToolbarItem* item : e.entries)
	{
		entries.push_back(new ToolbarItem(*item));
	}
}

ToolbarEntry::~ToolbarEntry()
{
	clearList();
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
	return this->name;
}

void ToolbarEntry::setName(string name)
{
	this->name = name;
}

int ToolbarEntry::addItem(string item)
{
	ToolbarItem* it = new ToolbarItem(item);
	entries.push_back(it);

	return it->getId();
}

bool ToolbarEntry::removeItemById(int id)
{
	for (unsigned int i = 0; i < this->entries.size(); i++)
	{
		if (this->entries[i]->getId() == id)
		{
			delete this->entries[i];
			entries[i] = nullptr;
			entries.erase(entries.begin() + i);
			return true;
		}
	}
	return false;
}

int ToolbarEntry::insertItem(string item, int position)
{
	ToolbarItem* it = new ToolbarItem(item);
	if (position >= (int)entries.size())
	{
		entries.push_back(it);
		return it->getId();
	}

	entries.insert(entries.begin() + position, it);
	return it->getId();
}

const ToolbarItemVector& ToolbarEntry::getItems() const
{
	return entries;
}
