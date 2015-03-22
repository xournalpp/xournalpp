#include "CustomizeableColorList.h"
#include <config.h>
#include <glib/gi18n-lib.h>

CustomizeableColorList::CustomizeableColorList()
{
	XOJ_INIT_TYPE(CustomizeableColorList);

	this->colors = NULL;

	this->addPredefinedColor(0x000000, _("Black"));
	this->addPredefinedColor(0x008000, _("Green"));
	this->addPredefinedColor(0x00c0ff, _("Light Blue"));
	this->addPredefinedColor(0x00ff00, _("Light Green"));
	this->addPredefinedColor(0x3333cc, _("Blue"));
	this->addPredefinedColor(0x808080, _("Gray"));
	this->addPredefinedColor(0xff0000, _("Red"));
	this->addPredefinedColor(0xff00ff, _("Mangenta"));
	this->addPredefinedColor(0xff8000, _("Orange"));
	this->addPredefinedColor(0xffff00, _("Yellow"));
	this->addPredefinedColor(0xffffff, _("White"));
}

CustomizeableColorList::~CustomizeableColorList()
{
	XOJ_CHECK_TYPE(CustomizeableColorList);

	for (GList* l = this->colors; l != NULL; l = l->next)
	{
		delete (XojColor*) l->data;
	}
	g_list_free(this->colors);
	this->colors = NULL;

	XOJ_RELEASE_TYPE(CustomizeableColorList);
}

ListIterator<XojColor*> CustomizeableColorList::getPredefinedColors()
{
	XOJ_CHECK_TYPE(CustomizeableColorList);

	return ListIterator<XojColor*>(this->colors);
}

void CustomizeableColorList::addPredefinedColor(int color, string name)
{
	this->colors = g_list_append(this->colors, new XojColor(color, name));
}
