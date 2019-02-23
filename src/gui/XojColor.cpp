#include "XojColor.h"

XojColor::XojColor(int color, string name)
 : color(color),
   name(name)
{
	XOJ_INIT_TYPE(XojColor);
}

XojColor::~XojColor()
{
	XOJ_RELEASE_TYPE(XojColor);
}

int XojColor::getColor()
{
	XOJ_CHECK_TYPE(XojColor);

	return this->color;
}

string XojColor::getName()
{
	XOJ_CHECK_TYPE(XojColor);

	return this->name;
}
