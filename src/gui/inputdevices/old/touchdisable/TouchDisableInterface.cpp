#include "TouchDisableInterface.h"

TouchDisableInterface::TouchDisableInterface()
{
	XOJ_INIT_TYPE(TouchDisableInterface);
}

TouchDisableInterface::~TouchDisableInterface()
{
	XOJ_CHECK_TYPE(TouchDisableInterface);
	XOJ_RELEASE_TYPE(TouchDisableInterface);
}

void TouchDisableInterface::init()
{
	XOJ_CHECK_TYPE(TouchDisableInterface);
}
