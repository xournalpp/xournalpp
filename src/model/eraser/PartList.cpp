#include "PartList.h"

#include "EraseableStrokePart.h"

PartList::PartList()
{
	XOJ_INIT_TYPE(PartList);
}

PartList::~PartList()
{
	XOJ_CHECK_TYPE(PartList);

	for (GList* l = this->data; l != NULL; l = l->next)
	{
		EraseableStrokePart* p = (EraseableStrokePart*) l->data;
		delete p;
	}
	g_list_free(this->data);
	this->data = NULL;

	XOJ_RELEASE_TYPE(PartList);
}

void PartList::add(EraseableStrokePart* part)
{
	XOJ_CHECK_TYPE(PartList);

	this->data = g_list_append(this->data, part);
}

PartList* PartList::clone()
{
	XOJ_CHECK_TYPE(PartList);

	PartList* list = new PartList();
	for (GList* l = this->data; l != NULL; l = l->next)
	{
		EraseableStrokePart* p = (EraseableStrokePart*) l->data;
		list->data = g_list_append(list->data, p->clone());
	}

	return list;
}
