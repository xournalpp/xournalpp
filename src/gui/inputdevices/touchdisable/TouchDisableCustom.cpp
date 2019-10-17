#include "TouchDisableCustom.h"

TouchDisableCustom::TouchDisableCustom(string enableCommand, string disableCommand)
 : enableCommand(enableCommand),
   disableCommand(disableCommand)
{
}

TouchDisableCustom::~TouchDisableCustom()
{
}

void TouchDisableCustom::enableTouch()
{
	system(enableCommand.c_str());
}

void TouchDisableCustom::disableTouch()
{
	system(disableCommand.c_str());
}
