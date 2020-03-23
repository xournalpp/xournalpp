#include "TouchDisableCustom.h"

#include "Util.h"

TouchDisableCustom::TouchDisableCustom(string enableCommand, string disableCommand)
 : enableCommand(enableCommand),
   disableCommand(disableCommand)
{
	XOJ_INIT_TYPE(TouchDisableCustom);
}

TouchDisableCustom::~TouchDisableCustom()
{
	XOJ_CHECK_TYPE(TouchDisableCustom);
	XOJ_RELEASE_TYPE(TouchDisableCustom);
}

void TouchDisableCustom::enableTouch()
{
	XOJ_CHECK_TYPE(TouchDisableCustom);

	Util::systemWithMessage(enableCommand.c_str());
}

void TouchDisableCustom::disableTouch()
{
	XOJ_CHECK_TYPE(TouchDisableCustom);

	Util::systemWithMessage(disableCommand.c_str());
}
