#include "XojColor.h"

XojColor::XojColor(int color, string name)
 : color(color),
   name(name)
{
}

XojColor::~XojColor()
{
}

int XojColor::getColor()
{
	return this->color;
}

string XojColor::getName()
{
	return this->name;
}
