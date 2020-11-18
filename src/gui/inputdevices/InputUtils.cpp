#include "InputUtils.h"

#include "control/settings/ButtonConfig.h"


void InputUtils::applyButton(ToolHandler* toolHandler, Settings* settings, Buttons button, bool& pressed,
                             bool& toolChanged, bool& configChanged) {
    pressed = true;
    if (toolHandler->pointCurrentToolToButtonTool(button)) {
        toolChanged = true;
        ButtonConfig* cfg = settings->getButtonConfig(button);
        configChanged = cfg->acceptActions(toolHandler, button);
    }
}
