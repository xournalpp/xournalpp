#include "InputUtils.h"

#include <glib.h>  // for g_message

#include "control/ToolEnums.h"               // for TOOL_NONE
#include "control/ToolHandler.h"             // for ToolHandler
#include "control/settings/ButtonConfig.h"   // for ButtonConfig
#include "control/settings/Settings.h"       // for Settings
#include "control/settings/SettingsEnums.h"  // for Button, BUTTON_TOUCH


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

bool InputUtils::touchDrawingDisallowed(ToolHandler* toolHandler, Settings* settings) {
    static bool warningAlreadyShown{false};
    ButtonConfig* cfg = settings->getButtonConfig(Button::BUTTON_TOUCH);
    if (cfg->getDisableDrawing() && cfg->getAction() == TOOL_NONE && toolHandler->isDrawingTool()) {
        if (!warningAlreadyShown)
            g_message("Ignoring touchscreen for drawing:\n"
                      " Please check the settings for Touchscreen.\n"
                      " The current combination of \"Disable Drawing for this device\"\n"
                      " together with \"Tool - don't change\"\n"
                      " prevents any drawing with the selected tool using the TouchScreen.");
        warningAlreadyShown = true;
        return true;
    }
    // reset warningAlreadyShown this allows the message to be sent multiple times
    // in case the user switches the settings in between multiple times
    warningAlreadyShown = false;
    return false;
}
