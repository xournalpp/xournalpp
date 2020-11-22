#include "InputUtils.h"

#include "control/ToolHandler.h"
#include "control/settings/ButtonConfig.h"
#include "control/settings/Settings.h"
#include "control/settings/SettingsEnums.h"


bool InputUtils::applyButton(ToolHandler* toolHandler, Settings* settings, Button button) {
    bool toolChanged = false;
    // if active tool already points to the correct tool nothing needs to be done
    if (toolHandler->pointActiveToolToButtonTool(button)) {
        toolChanged = true;
        ButtonConfig* cfg = settings->getButtonConfig(button);

        // if ToolChange is not activated for this button stay with ToolBartool
        if (cfg->getAction() == TOOL_NONE)
            toolChanged = toolHandler->pointActiveToolToToolbarTool();
        else
            cfg->applyNoChangeSettings(toolHandler, button);
    }
    return toolChanged;
}

void InputUtils::warnIfQuestionableTouchDrawingSettings(ToolHandler* toolHandler, Settings* settings) {
    ButtonConfig* cfg = settings->getButtonConfig(Button::BUTTON_TOUCH);
    if (cfg->getDisableDrawing() && cfg->getAction() == TOOL_NONE && toolHandler->hasCapability(TOOL_CAP_SIZE)) {
        g_message("Ignoring touchscreen for drawing.\n"
                  " Please check the settings for Touchscreen.\n"
                  " The current combination of \"Disable Drawing for this device\" and \"Tool - don't change\"\n"
                  " results in drawing with the currently selected tool (in the toolbar).\n"
                  " This mightnot be the desired behaviour.");
    }
}
