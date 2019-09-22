#include "CustomizeableColorList.h"

#include <config.h>
#include <i18n.h>

CustomizeableColorList::CustomizeableColorList()
{
	this->addPredefinedColor(0x000000, _("Black"));
	this->addPredefinedColor(0x008000, _("Green"));
	this->addPredefinedColor(0x00c0ff, _("Light Blue"));
	this->addPredefinedColor(0x00ff00, _("Light Green"));
	this->addPredefinedColor(0x3333cc, _("Blue"));
	this->addPredefinedColor(0x808080, _("Gray"));
	this->addPredefinedColor(0xff0000, _("Red"));
	this->addPredefinedColor(0xff00ff, _("Magenta"));
	this->addPredefinedColor(0xff8000, _("Orange"));
	this->addPredefinedColor(0xffff00, _("Yellow"));
	this->addPredefinedColor(0xffffff, _("White"));
}

CustomizeableColorList::~CustomizeableColorList()
{
	for (XojColor* c : this->colors)
	{
		delete c;
	}
	this->colors.clear();
}

vector<XojColor*>* CustomizeableColorList::getPredefinedColors()
{
	return &this->colors;
}

void CustomizeableColorList::addPredefinedColor(int color, string name)
{
	this->colors.push_back(new XojColor(color, name));
}
