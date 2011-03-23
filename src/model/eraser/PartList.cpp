#include "PartList.h"
#include "EraseableStrokePart.h"
// TODO: AA: type check

PartList::PartList() {
	this->data = NULL;
}

PartList::~PartList() {
	for (GList * l = this->data; l != NULL; l = l->next) {
		EraseableStrokePart * p = (EraseableStrokePart *) l->data;
		delete p;
	}
	this->data = NULL;
}

void PartList::add(EraseableStrokePart * part) {
	this->data = g_list_append(this->data, part);
}

PartList * PartList::clone() {
	PartList * list = new PartList();
	for (GList * l = this->data; l != NULL; l = l->next) {
		EraseableStrokePart * p = (EraseableStrokePart *) l->data;
		list->data = g_list_append(list->data, p->clone());
	}

	return list;
}
