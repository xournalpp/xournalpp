/*
 * Xournal++
 *
 * Part of the customizable toolbars
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */
#pragma once

#include <string>

#include "ComboToolButton.h"

class IconNameHelper;
class ActionDatabase;

class ToolSelectCombocontrol: public ComboToolButton {
public:
    ToolSelectCombocontrol(std::string id, IconNameHelper& icons, const ActionDatabase& db, bool hideAudio);
    ~ToolSelectCombocontrol() override = default;
};
