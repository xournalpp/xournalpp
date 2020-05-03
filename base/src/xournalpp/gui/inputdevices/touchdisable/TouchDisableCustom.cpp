#include "TouchDisableCustom.h"

#include <utility>

#include "Util.h"


TouchDisableCustom::TouchDisableCustom(string enableCommand, string disableCommand):
        enableCommand(std::move(enableCommand)), disableCommand(std::move(disableCommand)) {}

TouchDisableCustom::~TouchDisableCustom() = default;

void TouchDisableCustom::enableTouch() { Util::systemWithMessage(enableCommand.c_str()); }

void TouchDisableCustom::disableTouch() { Util::systemWithMessage(disableCommand.c_str()); }
