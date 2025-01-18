#pragma once

#include <string>

#include "ComboToolButton.h"

class IconNameHelper;
class ActionDatabase;

class ToolPdfCombocontrol: public ComboToolButton {
public:
    ToolPdfCombocontrol(std::string id, IconNameHelper& icons, const ActionDatabase& db);
    ~ToolPdfCombocontrol() override = default;
};
