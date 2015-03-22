#include "ToolbarEntry.h"

ToolbarEntry::ToolbarEntry()
{
	XOJ_INIT_TYPE(ToolbarEntry);
	this->entries = NULL;
}

ToolbarEntry::ToolbarEntry(const ToolbarEntry& e)
{
	XOJ_INIT_TYPE(ToolbarEntry);

	this->entries = NULL;

	*this = e;
}

void ToolbarEntry::operator=(const ToolbarEntry& e)
{
	XOJ_CHECK_TYPE(ToolbarEntry);

	this->name = e.name;
	clearList();

	for (GList* l = e.entries; l != NULL; l = l->next)
	{
		ToolbarItem* item = (ToolbarItem*) l->data;
		this->entries = g_list_append(this->entries, new ToolbarItem(*item));
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
	for (GList* l = this->entries; l != NULL; l = l->next)
	{
		ToolbarItem* item = (ToolbarItem*) l->data;
		delete item;
	}

	g_list_free(this->entries);
	this->entries = NULL;
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
	this->entries = g_list_append(this->entries, it);

	return it->getId();
}

bool ToolbarEntry::removeItemById(int id)
{
	XOJ_CHECK_TYPE(ToolbarEntry);

	for (GList* l = this->entries; l != NULL; l = l->next)
	{
		ToolbarItem* item = (ToolbarItem*) l->data;
		if (item->getId() == id)
		{
			this->entries = g_list_delete_link(this->entries, l);
			delete item;
			return true;
		}
	}
	return false;
}

int ToolbarEntry::insertItem(string item, int position)
{
	XOJ_CHECK_TYPE(ToolbarEntry);

	ToolbarItem* it = new ToolbarItem(item);
	this->entries = g_list_insert(this->entries, it, position);

	return it->getId();
}

ListIterator<ToolbarItem*> ToolbarEntry::iterator()
{
	XOJ_CHECK_TYPE(ToolbarEntry);

	return ListIterator<ToolbarItem*> (this->entries);
}
