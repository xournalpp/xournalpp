#include "TouchDisableCustom.h"

#include <utility>

TouchDisableCustom::TouchDisableCustom(string enableCommand, string disableCommand):
        enableCommand(std::move(enableCommand)), disableCommand(std::move(disableCommand)) {}

TouchDisableCustom::~TouchDisableCustom() = default;

void TouchDisableCustom::enableTouch() { system(enableCommand.c_str()); }

void TouchDisableCustom::disableTouch() { system(disableCommand.c_str()); }
