#include "ToolbarEntry.h"

ToolbarEntry::ToolbarEntry() {
	XOJ_INIT_TYPE(ToolbarEntry);
	this->entries = NULL;
}

ToolbarEntry::ToolbarEntry(const ToolbarEntry & e) {
	XOJ_INIT_TYPE(ToolbarEntry);

	*this = e;
}

void ToolbarEntry::operator =(const ToolbarEntry & e) {
	XOJ_CHECK_TYPE(ToolbarEntry);

	this->name = e.name;
	this->entries = g_list_copy(e.entries);
}

ToolbarEntry::~ToolbarEntry() {
	XOJ_CHECK_TYPE(ToolbarEntry);

	for (GList * l = this->entries; l != NULL; l = l->next) {
		ToolbarItem * item = (ToolbarItem *) item;
		delete item;
	}

	g_list_free(this->entries);
	this->entries = NULL;

	XOJ_RELEASE_TYPE(ToolbarEntry);
}

String ToolbarEntry::getName() {
	XOJ_CHECK_TYPE(ToolbarEntry);

	return this->name;
}

void ToolbarEntry::setName(String name) {
	XOJ_CHECK_TYPE(ToolbarEntry);

	this->name = name;
}

void ToolbarEntry::addItem(String item) {
	XOJ_CHECK_TYPE(ToolbarEntry);

	ToolbarItem * it = new ToolbarItem(item);
	this->entries = g_list_append(this->entries, it);
}

bool ToolbarEntry::removeItemById(int id) {
	XOJ_CHECK_TYPE(ToolbarEntry);

	for (GList * l = this->entries; l != NULL; l = l->next) {
		ToolbarItem * item = (ToolbarItem *) item;
		if (item->getId() == id) {
			this->entries = g_list_delete_link(this->entries, l);
			delete item;
			return true;
		}
	}
	return false;
}

void ToolbarEntry::insertItem(String item, int position) {
	XOJ_CHECK_TYPE(ToolbarEntry);

	ToolbarItem * it = new ToolbarItem(item);
	this->entries = g_list_insert(this->entries, it, position);
}

ListIterator<ToolbarItem *> ToolbarEntry::iterator() {
	XOJ_CHECK_TYPE(ToolbarEntry);

	return ListIterator<ToolbarItem *> (this->entries);
}
